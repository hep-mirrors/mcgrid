//
//  sherpaFill.cpp
//  MCgrid 03/10/2013.
//

// System
#include "config.h"

#include "mcgrid.hh"
#include "grid.hh"
#include "sherpaFillInfo.hh"

#include <sstream>

// Rivet includes
#include "Rivet/Rivet.hh"
#include "Rivet/Event.hh"

// HEPMC
#include "HepMC/GenEvent.h"
#include "HepMC/PdfInfo.h"

using namespace MCgrid;

using Rivet::cerr;
using Rivet::cout;
using Rivet::endl;

// ************ Projection functions for collinear counterterms ***********
// These functions multiply each flavour contribution for the counterterms.
// Variables proj and a should have LHA flavour index (-top), i.e -5 to 5

// Identity projection
static inline void pi( const int a, double* proj)
{
  for (int i=-5; i<=5; i++ )
    proj[i] = ( (i==a) ? 1:0 );
  return;
}

// Corresponds to building f_a^1 = f_a(x_a) (a=quark), \sum_q f_q(x_a) (a=gluon)
static inline void p1( const int a, double* proj )
{
  if ( a == 0 )
  {
    // Do not accidentally introduce initial state quarks that are inactive
    for (int i=-numberOfActiveFlavors; i<=numberOfActiveFlavors; i++)
      proj[i] = ( (i==0) ? 0:1 );
  } else
  {
    for (int i=-5; i<=5; i++ )
      proj[i] = ( (i==a) ? 1:0 );
  }
  return;
}

// Corresponds to building f_a^3 = f_g(x_a),
static inline void p2( const int a, double* proj )
{
  for (int i=-5; i<=5; i++ )
    proj[i] = ( (i==0) ? 1:0 );
  return;
}

 // ************************ SHERPA Fill Method ****************************

/*
 *  grid::sherpaFill
 *  Fills the APPLgrid/fastNLO table from a SHERPA-generated HepMC event.
 *  Takes into account the CS counterterm structure as implemented
 *  in SHERPA.
 */

void _grid::sherpaFill( double coord, sherpaFillInfo const & info)
{
  const double norm = pdf->EventRatio(info.fl1, info.fl2);

  const ReweightType type(info.reweight_type);

  if (type == ReweightTypeLO) {
    // LO(PS)
    fillInfo subInfo(info);
    subInfo.wgt = info.usr_wgt["Reweight_B"];
    sherpaBLikeFill(coord, norm, subInfo, 0);

  } else {
    // NLO(PS)

    // NLO(PS) Born
    if (type & ReweightTypeB) {
      fillInfo subInfo(info);
      subInfo.wgt = info.usr_wgt["Reweight_B"];
      sherpaBLikeFill(coord, norm, subInfo, 0);
    }

    // NLO(PS) VI
    if (type & ReweightTypeVI) {
      fillInfo subInfo(info);
      subInfo.wgt = info.usr_wgt["Reweight_VI"];
      sherpaBLikeFill(coord, norm, subInfo, 1);
    }

    // NLO(PS) KP
    if (type & ReweightTypeKP) {
      sherpaKPFill(coord, norm, info);
    }

    // NLOPS DADS terms
    if (type & ReweightTypeDADS) {
      for (size_t i(0); i < info.DADS_fill_infos.size(); i++) {
        sherpaBLikeFill(coord, norm, info.DADS_fill_infos[i], 1);
      }
    }

    // NLOPS H
    if (type & ReweightTypeH) {
      fillInfo subInfo(info);
      subInfo.wgt = info.usr_wgt["Reweight_B"];
      sherpaBLikeFill(coord, norm, subInfo, 1);
    }

    // NLO RS
    if (type & ReweightTypeRS) {
      // For RS (sub)events in Sherpa, the pdfinfo object is filled with the
      // original values. Namely the scale and the (scale-dependent) PDF values
      // are therefore incorrect. However, the latter are not used, we only
      // have to fix the scale.
      fillInfo subInfo(info);
      subInfo.wgt =   info.usr_wgt["Reweight_RS"];
      subInfo.pdfQ2 = info.usr_wgt["MuR2"];
      sherpaBLikeFill(coord, norm, subInfo, 1);
    }

  }
}

void _grid::sherpaBLikeFill(double coord, double norm, fillInfo const& info, int ptord)
{
  assert(ptord == 0 || ptord == 1); // Only NLO is supported

  const double asfac = pow(info.alphas / (2*M_PI), leadingOrder + ptord);
  const double meweight = norm * info.wgt / asfac;

  zeroWeights();
  fillWeight(info.fl1, info.fl2, meweight, false);
  fillUnderlyingGrid(info.x1, info.x2, info.pdfQ2, coord, ptord);
}

void _grid::sherpaKPFill(double coord, double norm, sherpaFillInfo const& info)
{
  zeroWeights();
  const double asfac = pow(info.alphas / (2*M_PI), leadingOrder + 1);

  // Read x-prime values
  const double x1p = info.usr_wgt["Reweight_KP_x1p"];
  const double x2p = info.usr_wgt["Reweight_KP_x2p"];

  // Prepare weights
  double w[8];
  for (int i=0; i<8; i++) {
    std::ostringstream key;
    key << "Reweight_KP_wfac_" << i;
    w[i] = norm * info.usr_wgt[key.str()] / asfac;
  }
  
  // Factors of xprime
  w[1]/=x1p;
  w[3]/=x1p;
  w[5]/=x2p;
  w[7]/=x2p;
  
  // First fill - untransformed x values
  // f_a^1 w_1 F_b(x_b) + f_a(x_a) w_5 F_b^1
  projectWeights(info.fl1, info.fl2, w[0], p1, pi, false);
  projectWeights(info.fl1, info.fl2, w[4], pi, p1, false);

  // f_a^3 w_3 F_b(x_b) + f_a(x_a)w_7 F_b^3
  projectWeights(info.fl1, info.fl2, w[2], p2, pi, false);
  projectWeights(info.fl1, info.fl2, w[6], pi, p2, false);
  fillUnderlyingGrid(info.x1, info.x2, info.pdfQ2, coord, 1);
  
  // Prepare for x1p fill
  zeroWeights();
  
  // f_a^2 w_2 F_b(x_b) + f_a^4 w_4 F_b(x_b)
  projectWeights(info.fl1, info.fl2, w[1], p1, pi, false);
  projectWeights(info.fl1, info.fl2, w[3], p2, pi, false);
  fillUnderlyingGrid(info.x1/x1p, info.x2, info.pdfQ2, coord, 1);
  
  // Prepare for x2p fill
  zeroWeights();
  
  // f_a(x_a) w_6 F_b^2 + f_a(x_a) w_8 F_b^4
  projectWeights(info.fl1, info.fl2, w[5], pi, p1, false);
  projectWeights(info.fl1, info.fl2, w[7], pi, p2, false);
  fillUnderlyingGrid(info.x1, info.x2/x2p, info.pdfQ2, coord, 1);  
}
