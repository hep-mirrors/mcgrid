//
//  grid_appl.hh
//  MCgrid 03/05/2015.
//

#ifndef MCgrid_grid_appl_h
#define MCgrid_grid_appl_h

#include "grid.hh"

namespace appl{ class grid; }

namespace MCgrid {

class _grid_appl : public _grid {
public:
  // Create a new APPLgrid-backed grid based upon a YODA histogram
  _grid_appl(const Rivet::Histo1DPtr,
             const std::string _analysis,
             applGridConfig);

  ~_grid_appl();

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

  appl::grid *applgrid;
};

}

#endif
