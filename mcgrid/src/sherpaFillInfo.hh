//
//  sherpaFillInfo.hh
//  MCgrid 30/07/2015.
//

#ifndef mcgrid_sherpa_fill_info_hh
#define mcgrid_sherpa_fill_info_hh

#include "fillInfo.hh"

#include <vector>
#include <string>

namespace MCgrid {

  // This follows the Sherpa convention
  typedef enum {                        // Present in:
    ReweightTypeLO            = 0,      // LO(PS)
    ReweightTypeB             = 1 << 0, // NLO(PS), MEPS@NLO
    ReweightTypeVI            = 1 << 1, // NLO(PS), MEPS@NLO
    ReweightTypeKP            = 1 << 2, // NLO(PS), MEPS@NLO
    ReweightTypeDADS          = 1 << 3, // NLOPS,   MEPS@NLO
    ReweightTypeClusterSteps  = 1 << 4, // MEPS@(N)LO
    ReweightTypeH             = 1 << 5, // NLOPS, MEPS@NLO
    ReweightTypeRS            = 1 << 6  // NLO
  } ReweightType;

  class sherpaFillInfo : public fillInfo
  {
  public:
    sherpaFillInfo(Rivet::Event const&);

    const HepMC::WeightContainer & usr_wgt;
    const ReweightType reweight_type;
    const std::vector<fillInfo> DADS_fill_infos;
    const std::vector<fillInfo> RDA_fill_infos;

  private:
    // Helper methods to construc sub fill infos (DADS/RDA) from user weights
    static std::vector<fillInfo> DADSInfos(HepMC::WeightContainer const &,
                                           const double pdfQ2, const double alphas);
    static std::vector<fillInfo> RDAInfos(HepMC::WeightContainer const &,
                                          int fl1, int fl2, double x1, double x2);
    static size_t numberOfTerms(HepMC::WeightContainer const &, std::string const &);
    static std::string keyPrefixForEntryWithIndex(size_t, std::string const &);
    static bool hasNonZeroWeightEntryForKeyPrefix(HepMC::WeightContainer const &, std::string const &);
  };

}

#endif
