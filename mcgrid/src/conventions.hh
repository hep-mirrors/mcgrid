//
//  conventions.hh
//  MCgrid 30/01/2015.
//

#ifndef mcgrid_conventions_hh
#define mcgrid_conventions_hh

namespace MCgrid {
  
  // Convert PDG codes to LHAPDF for partons
  static inline int pdgToLHA( int pdg )
  {
    if (pdg == 21 ) return 0;
    return pdg;
  }

}

#endif
