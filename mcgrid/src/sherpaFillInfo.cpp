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
  DADS_fill_infos(DADSInfos(usr_wgt, pdfQ2, alphas)),
  RDA_fill_infos(RDAInfos(usr_wgt, fl1, fl2, x1, x2))
  { }

  std::vector<fillInfo> sherpaFillInfo::DADSInfos(HepMC::WeightContainer const & usr_wgt,
                                                 const double pdfQ2, const double alphas)
  {
    std::vector<fillInfo> subInfos;
    const std::string sub_info_tag("DADS");

    for (size_t i(0); i < numberOfTerms(usr_wgt, sub_info_tag); i++) {
      std::string key_prefix = keyPrefixForEntryWithIndex(i, sub_info_tag);
      if (hasNonZeroWeightEntryForKeyPrefix(usr_wgt, key_prefix)) {
        fillInfo info(usr_wgt, key_prefix, pdfQ2, alphas);
        subInfos.push_back(info);
      }
    }

    return subInfos;
  }

  std::vector<fillInfo> sherpaFillInfo::RDAInfos(HepMC::WeightContainer const & usr_wgt,
                                                 int fl1, int fl2, double x1, double x2)
  {
    std::vector<fillInfo> subInfos;
    const std::string sub_info_tag("RDA");

    for (size_t i(0); i < numberOfTerms(usr_wgt, sub_info_tag); i++) {
      std::string key_prefix = keyPrefixForEntryWithIndex(i, sub_info_tag);
      if (hasNonZeroWeightEntryForKeyPrefix(usr_wgt, key_prefix)) {
        fillInfo info(usr_wgt, key_prefix, fl1, fl2, x1, x2);
        subInfos.push_back(info);
      }
    }

    return subInfos;
  }

  size_t sherpaFillInfo::numberOfTerms(HepMC::WeightContainer const & usr_wgt, std::string const & sub_info_tag)
  {
    std::ostringstream key;
    key << "Reweight_" << sub_info_tag << "_N";
    if (!usr_wgt.has_key(key.str())) {
      return 0;
    }
    return (size_t)usr_wgt[key.str()];
  }

  std::string sherpaFillInfo::keyPrefixForEntryWithIndex(size_t index, std::string const & sub_info_tag)
  {
    std::ostringstream key;
    key << "Reweight_" << sub_info_tag << "_" << index << "_";
    return key.str();
  }

  bool sherpaFillInfo::hasNonZeroWeightEntryForKeyPrefix(HepMC::WeightContainer const & usr_wgt, std::string const & key_prefix)
  {
    std::string weightKey(key_prefix + "Weight");
    return (usr_wgt.has_key(weightKey) && usr_wgt[weightKey] != 0.0);
  }

}
