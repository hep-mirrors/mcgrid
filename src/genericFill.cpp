//
//  genericFill.cpp
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
#include "appl_grid/lumi_pdf.h"

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

void grid::genericFill(double coord, fillInfo const& info)
{
  // Remove the required pdf and alpha_s factors from the full weight
  const double norm = applpdf->EventRatio(info.fl1, info.fl2);
  const double meweight = norm*info.wgt/(info.fx1*info.fx2);
  const double meweight_noalpha = meweight/info.asfac;
  
  // Populate weight grid and fill the APPLgrid
  zeroWeights(); fillWeight(info.fl1, info.fl2, meweight_noalpha);
  applgrid->fill_grid(info.x1, info.x2, info.pdfQ2, coord, weights, info.ptord);
  
  return;
}
