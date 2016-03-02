//
//  genericFill.cpp
//  MCgrid 03/10/2013.
//

// System
#include "config.h"

#include "grid.hh"
#include "fillInfo.hh"

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

/*
 *  grid::genericFill
 *  This method takes a generic HepMC event weight, and assumes
 *  that the full functional dependance of the weight upon the PDFs
 *  and alpha_S can be removed by dividing out fx1fx2
 *  from the full weight.
 */

void _grid::genericFill(double coord, fillInfo const& info)
{
  // NOTE: As we do not need it when treating Sherpa events, the PDF values
  // itself are currently not read out from the HepMC record. If it is
  // necessary, add pdf value fields to the fillInfo class (or a subclass)
  // and read out the value like this in `fillInfo(Rivet::Event const&)`:
  // `event.genEvent()->pdf_info()->pdf1()/x1`

  const double meweight = info.wgt;

  // NOTE: The correct order must be determined here, we assume LO-only events here
  const double aspowers = leadingOrder;

  // Remove the a_S factor from the weight
  const double asfac = pow(info.alphas * alphaSPrefactor, aspowers);
  const double meweight_without_asfac = meweight / asfac;
  
  // Populate weight grid and fill the APPLgrid
  zeroWeights();
  fillWeight(info.fl1, info.fl2, meweight_without_asfac, true);
  fillUnderlyingGrid(info.x1, info.x2, info.pdfQ2, coord, LO);
  
  return;
}
