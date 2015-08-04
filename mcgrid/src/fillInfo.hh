//
//  fillInfo.hh
//  MCgrid 30/01/2015.
//

#ifndef mcgrid_fill_info_hh
#define mcgrid_fill_info_hh

#include <string>

namespace Rivet{ class Event; }
namespace HepMC{ class WeightContainer; }

namespace MCgrid {

  class sherpaFillInfo;

  class fillInfo
  {
  public:
    fillInfo(Rivet::Event const&);

    // Initialise (DADS) fill info from Sherpa fill info user weights
    fillInfo(HepMC::WeightContainer const &, std::string const & usr_wgt_key_prefix, const double pdfQ2, const double alphas);

    double wgt;      //!< Event weight

    int    fl1, fl2; //!< Flavors of the incoming partons
    double x1,  x2;  //!< x values of the incoming partons
    double pdfQ2;          //!< The Q^2 value used for the PDFs
    double alphas;   //!< alpha_s value
  };

}

#endif
