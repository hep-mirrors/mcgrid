//
//  grid_appl.cpp
//  MCgrid 03/05/2015.
//

#include "config.h"

#if APPLGRID_ENABLED

#include "appl_grid/appl_grid.h"
#include "appl_grid/lumi_pdf.h"

#include "grid_appl.hh"

using Rivet::cerr;
using Rivet::cout;
using Rivet::endl;

namespace MCgrid
{
  std::string _grid_appl::gridInterfaceName() const
  {
    return _grid::gridInterfaceName(applgridInterface);
  }
  _grid_appl::_grid_appl(const Rivet::Histo1DPtr histPtr,
                         const std::string _analysis,
                         applGridConfig config):
    _grid(histPtr, _analysis)
  {
    // Inform the user what we're up to
    cout << "MCgrid: Use APPLgrid as underlying grid implementation" << endl;

    // For APPLgrid-based grids, the pdf is backed by an appl::lumi_pdf
    // instance, which is needed to setup a grid. So we first create the pdf,
    // and only then the grid

    leadingOrder = config.lo;

    mcgrid_base_pdf_params *pdf_params;
    pdf_params = new mcgrid_appl_pdf_params(config.subprocConfig.fileName,
                                            config.subprocConfig.beam1,
                                            config.subprocConfig.beam2);

    readPDFWithParameters(*pdf_params, analysis);
    delete pdf_params;

    if (!isWarmup())
    {
      // Read phase space file
      cout << "MCgrid: Reading phase space optimised APPLgrid" << endl;
      applgrid = new appl::grid(phasespaceFilePath());
      return;
    }

    applGridArch arch = config.arch;
    applgrid = new appl::grid(getBinning(histo),
                              arch.nQ,
                              config.q2min,
                              config.q2max,
                              arch.qOrd,
                              arch.nX,
                              config.xmin,
                              config.xmax,
                              arch.xOrd,
                              pdf->name(),
                              leadingOrder, 1,
                              std::string("f2")
                              );
  }

  _grid_appl::~_grid_appl()
  {
    delete applgrid;
  }

  void _grid_appl::fillReferenceHistogram(double coord, double wgt) {
    applgrid->getReference()->Fill(coord, wgt);
  }

  bool _grid_appl::isWarmup() const
  {
    // NOTE: The isOptimized member function of appl::grid is not really useful here,
    // as a even a warmup grid will be optimized at some point
    return !Rivet::fileexists(phasespaceFilePath());
  }

  void _grid_appl::exportgrid()
  {
    if (isWarmup()) {
      cout << "MCgrid: Optimising grid phase space ..." << endl;
      applgrid->optimise();
      cout << "MCgrid: ... grid optimised." << endl;
      cout << "MCgrid: Writing out phase space grid." << endl;
    } else {
      cout << "MCgrid: Exporting final " << gridInstanceString[applgridInterface];
      cout << "." << endl;
    }

    applgrid->Write(gridOrPhasespaceFilePath());
    cout << "MCgrid: Export Complete"<<endl;
  }

  void _grid_appl::scale(double const & scale)
  {
    _grid::scale(scale);
    applgrid->run() = 1.0/scale;
    applgrid->setNormalised(false);
  }

  void _grid_appl::fillUnderlyingGrid(const double x1,
                                      const double x2,
                                      const double pdfQ2,
                                      const double coord,
                                      const double ptord)
  {
    applgrid->fill_grid(x1, x2, pdfQ2, coord, weights, ptord);
  }

  std::string _grid_appl::phasespaceFileExtension() const
  {
    return gridFileExtension();
  }


  std::string _grid_appl::gridFileExtension() const
  {
    return "root";
  }

}

#endif