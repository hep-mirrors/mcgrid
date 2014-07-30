//
//  sherpaFill.cpp
//  MCgrid 03/10/2013.
//

#include "mcgrid/mcgrid.hh"

// Rivet includes
#include "Rivet/Rivet.hh"
#include "Rivet/Event.hh"

// HEPMC
#include "HepMC/GenEvent.h"
#include "HepMC/PdfInfo.h"

// APPLgrid includes
#include "appl_grid/appl_grid.h"

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
    for (int i=-5; i<=5; i++ )
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
 *  Fills the applgrid from a SHERPA-generated HepMC event.
 *  Takes into account the CS counterterm structure as implemented
 *  in SHERPA.
 */

void grid::sherpaFill( double coord, fillInfo const& info)
{
  // Determine normalisation ratio
  const double norm = applpdf->EventRatio(info.fl1, info.fl2);
  
  // Fill weight array with basic me weight
  zeroWeights();
  const double meweight = norm*info.usr_wgt[5];
  fillWeight(info.fl1, info.fl2, meweight/info.asfac);
  
  // Determine the total number of sub-weights
  // corresponding to collinear subtraction terms
  const int nWeights = info.usr_wgt[6];
  if ( nWeights != 18 )
  {
    applgrid->fill_grid(info.x1, info.x2, info.pdfQ2, coord, weights, info.ptord);
    return;
  }
  
  // Read x-prime values
  const double x1p = info.usr_wgt[7];
  const double x2p = info.usr_wgt[8];
  
  // Read weight array, removing factor of alpha_s
  // and normalising to ensure subprocess combination is
  // statistically sound
  double w[9];
  for (int i=0; i<9; i++)
    w[i] = norm*(info.usr_wgt[9+(i+1)])/info.asfac;
  
  // Factors of xprime
  w[2]/=x1p;
  w[4]/=x1p;
  w[6]/=x2p;
  w[8]/=x2p;
  
  // First fill - untransformed x values
  // f_a^1 w_1 F_b(x_b) + f_a(x_a) w_5 F_b^1
  projectWeights(info.fl1, info.fl2, w[1], p1, pi);
  projectWeights(info.fl1, info.fl2, w[5], pi, p1);

  // f_a^3 w_3 F_b(x_b) + f_a(x_a)w_7 F_b^3
  projectWeights(info.fl1, info.fl2, w[3], p2, pi);
  projectWeights(info.fl1, info.fl2, w[7], pi, p2);
  applgrid->fill_grid(info.x1, info.x2, info.pdfQ2, coord, weights, info.ptord);
  
  // Prepare for x1p fill
  zeroWeights();
  
  // f_a^2 w_2 F_b(x_b) + f_a^4 w_4 F_b(x_b)
  projectWeights(info.fl1, info.fl2, w[2], p1, pi);
  projectWeights(info.fl1, info.fl2, w[4], p2, pi);
  applgrid->fill_grid(info.x1/x1p, info.x2, info.pdfQ2, coord, weights, info.ptord);
  
  // Prepare for x2p fill
  zeroWeights();
  
  // f_a(x_a) w_6 F_b^2 + f_a(x_a) w_8 F_b^4
  projectWeights(info.fl1, info.fl2, w[6], pi, p1);
  projectWeights(info.fl1, info.fl2, w[8], pi, p2);
  applgrid->fill_grid(info.x1, info.x2/x2p, info.pdfQ2, coord, weights, info.ptord);
  
}
