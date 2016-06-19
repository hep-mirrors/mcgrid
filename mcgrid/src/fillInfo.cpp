//
//  fillInfo.cpp
//  MCgrid 30/01/2015.
//

#include <cmath>
#include <vector>

#include "fillInfo.hh"
#include "sherpaFillInfo.hh"
#include "conventions.hh"

#include "Rivet/Rivet.hh"

using Rivet::cerr;
using Rivet::cout;
using Rivet::endl;


namespace MCgrid {

  // Restricts the HepMC PDF scale to 8 decimal places
  // Improves comparison with scale bin widths
  double roundScale(double PDFscale)
  {
    const double prec = 1E8;
    const double rounded = round(PDFscale*prec)/prec;
    return rounded;
  }

  fillInfo::fillInfo(Rivet::Event const& event):
  // Full event weight
  wgt(event.weight()),
  // Flavours and subprocess classification
  fl1(pdgToLHA(event.genEvent()->pdf_info()->id1())),
  fl2(pdgToLHA(event.genEvent()->pdf_info()->id2())),
  // PDF Kinematics
  x1(event.genEvent()->pdf_info()->x1()),
  x2(event.genEvent()->pdf_info()->x2()),
  pdfQ2(roundScale(pow(event.genEvent()->pdf_info()->scalePDF(),2))),
  // Alpha_s
  alphas(event.genEvent()->alphaQCD())
  { }

  fillInfo::fillInfo(HepMC::WeightContainer const & usr_wgt, std::string const & usr_wgt_key_prefix, const double pdfQ2, const double alphas):
  wgt(usr_wgt[usr_wgt_key_prefix + "Weight"]),
  fl1(pdgToLHA(usr_wgt[usr_wgt_key_prefix + "fl1"])),
  fl2(pdgToLHA(usr_wgt[usr_wgt_key_prefix + "fl2"])),
  x1(usr_wgt[usr_wgt_key_prefix + "x1"]),
  x2(usr_wgt[usr_wgt_key_prefix + "x2"]),
  pdfQ2(pdfQ2),
  alphas(alphas)
  { }

  fillInfo::fillInfo(HepMC::WeightContainer const & usr_wgt, std::string const & usr_wgt_key_prefix, int fl1, int fl2, double x1, double x2):
  wgt(usr_wgt[usr_wgt_key_prefix + "Weight"]),
  fl1(fl1),
  fl2(fl2),
  x1(x1),
  x2(x2),
  pdfQ2(usr_wgt[usr_wgt_key_prefix + "MuF12"]),
  alphas(usr_wgt[usr_wgt_key_prefix + "AlphaS"])
  { }

}
