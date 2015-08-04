//
//  sherpaFillInfo.cpp
//  MCgrid 30/07/2015.
//

#include "sherpaFillInfo.hh"

#include "Rivet/Rivet.hh"
#include <HepMC/WeightContainer.h>

namespace MCgrid {

  sherpaFillInfo::sherpaFillInfo(Rivet::Event const& event):
  fillInfo(event),
  usr_wgt(event.genEvent()->weights()),
  reweight_type((ReweightType)usr_wgt["Reweight_Type"]),
  DADS_fill_infos(DADSInfos(usr_wgt, pdfQ2, alphas))
  { }

  std::vector<fillInfo> sherpaFillInfo::DADSInfos(HepMC::WeightContainer const & usr_wgt, const double pdfQ2, const double alphas)
  {
    std::vector<fillInfo> DADSInfos;

    for (size_t i(0); i < numberOfDADSTerms(usr_wgt); i++) {
      std::string key_prefix = keyPrefixForEntryWithIndex(i);
      if (hasNonZeroWeightEntryForKeyPrefix(usr_wgt, key_prefix)) {
        fillInfo info(usr_wgt, key_prefix, pdfQ2, alphas);
        DADSInfos.push_back(info);
      }
    }

    return DADSInfos;
  }

  size_t sherpaFillInfo::numberOfDADSTerms(HepMC::WeightContainer const & usr_wgt)
  {
    if (!usr_wgt.has_key("Reweight_DADS_N")) {
      return 0;
    }
    return (size_t)usr_wgt["Reweight_DADS_N"];
  }

  std::string sherpaFillInfo::keyPrefixForEntryWithIndex(size_t index)
  {
    std::ostringstream key;
    key << "Reweight_DADS_" << index << "_";
    return key.str();
  }

  bool sherpaFillInfo::hasNonZeroWeightEntryForKeyPrefix(HepMC::WeightContainer const & usr_wgt, std::string const & key_prefix)
  {
    std::string weightKey(key_prefix + "Weight");
    return (usr_wgt.has_key(weightKey) && usr_wgt[weightKey] != 0.0);
  }

}