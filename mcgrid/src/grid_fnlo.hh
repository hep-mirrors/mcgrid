//
//  grid_fnlo.hh
//  MCgrid 03/05/2015.
//

#ifndef MCgrid_grid_fnlo_h
#define MCgrid_grid_fnlo_h

#include "grid.hh"

class fastNLOCreate;

namespace MCgrid {

class _grid_fnlo : public _grid {
public:
  // Create a new fastNLO-backed grid based upon a YODA histogram
  _grid_fnlo(const Rivet::Histo1DPtr histPtr,
             const std::string analysis,
             fastnloConfig config);

  ~_grid_fnlo();

private:
  std::string gridInterfaceName() const;
  void fillReferenceHistogram(double coord, double wgt);
  bool isWarmup() const;
  void exportgrid();
  void scale(double const & scale);
  void fillUnderlyingGrid(const double x1,
                          const double x2,
                          const double pdfQ2,
                          const double coord,
                          const double ptord);
  std::string phasespaceFileExtension() const;
  std::string gridFileExtension() const;

  fastNLOCreate* ftableBase;  //!< Pointer to fastNLO grid used for warmup or LO
  fastNLOCreate* ftableNLO;   //!< Pointer to fastNLO grid used for NLO
};

}

#endif
