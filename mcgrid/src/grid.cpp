//
//  grid.cpp
//  MCgrid 03/10/2013.
//

#include <string>
#include <vector>
#include <cmath>
#include <memory>

// System
#include "config.h"

// MCGrid
#include "mcgrid/mcgrid.hh"
#include "mcgrid.hh"
#include "grid.hh"
#include "mcgrid/mcgrid_pdf.hh"
#include "fillInfo.hh"
#include "sherpaFillInfo.hh"
#include "banner.hh"
#include "system.hh"
#if APPLGRID_ENABLED
#include "grid_appl.hh"
#endif
#if FASTNLO_ENABLED
#include "grid_fnlo.hh"
#endif

// Rivet includes
#include "Rivet/Rivet.hh"
#include "Rivet/Event.hh"
#include "Rivet/Tools/RivetYODA.hh"

using Rivet::cerr;
using Rivet::cout;
using Rivet::endl;

namespace MCgrid
{

// **********************  Utility Functions **************************

static std::string idFromPath( std::string const& fullpath)
{
  unsigned ls = fullpath.find_last_of("/\\");
  return fullpath.substr(ls+1);
}


// *********************** Booking Functions **************************

// Book a MCgrid::grid object, returning the shared pointer
template<class T>
gridPtr bookGrid(const Rivet::Histo1DPtr hist,
                 const std::string histoDir,  
                 T config)
{
  if (boolForEnvironmentVariableForKey("MCGRID_DISABLED")) {
    showDisabledInfoOnce();
    // Return a pointer to a grid dummy with empty implementations
    return gridPtr(new grid_dummy());
  }
  showBannerOnce();
  createPath(MCgridPhasespacePath());
  createPath(MCgridOutputPath());

  #if APPLGRID_ENABLED
    const applGridConfig *appl_config = dynamic_cast<const applGridConfig*>(&config);
    if (appl_config) {
      return gridPtr(new _grid_appl(hist, histoDir, *appl_config));
    }
  #endif

  #if FASTNLO_ENABLED
    const fastnloConfig *fnlo_config = dynamic_cast<const fastnloConfig*>(&config);
    if (fnlo_config) {
      return gridPtr(new _grid_fnlo(hist, histoDir, *fnlo_config));
    }
  #endif

  cerr << "MCgrid::Error - Failed attempt to read the grid configuration.";
  cerr << " Is this version of MCgrid configured for use with this grid";
  cerr << " interface?";
  exit(-1);
}

// Explicit instantiations to make them available for external code (i.e.
// MCgrid-enhanced Rivet analyses)
#if APPLGRID_ENABLED
template gridPtr bookGrid(const Rivet::Histo1DPtr, const std::string, applGridConfig);
#endif
#if FASTNLO_ENABLED
template gridPtr bookGrid(const Rivet::Histo1DPtr, const std::string, fastnloConfig);
#endif

// ************************* grid class *********************************

_grid::_grid(const Rivet::Histo1DPtr histPtr,
             const std::string _analysis,
             const int _leadingOrder,
             const bool _isUsingScaleLogGrids,
             const double _alphaSPrefactor
             ):
histo                 (histPtr),
mode                  (globalFillMode),
path                  (idFromPath(histo.get()->path())),
analysis              (_analysis),
leadingOrder          (_leadingOrder),
isUsingScaleLogGrids  (_isUsingScaleLogGrids),
alphaSPrefactor       (_alphaSPrefactor),
fl1projection         (new double[11]),
fl2projection         (new double[11])
{
  // Inform the user what we're up to
  cout << "MCgrid: Generating new grid for histogram " << path << " of analysis " << analysis << endl;

  // Convert projection arrays to LHA basis
  fl1projection+=5;
  fl2projection+=5;
}

void _grid::readPDFWithParameters(mcgrid_base_pdf_params const& params, const std::string & analysis)
{
  pdf = PDFHandler::BookPDF(params, analysis);
  nSubProc = pdf->NumberOfSubprocesses();
  weights = new double[nSubProc];

  if (Rivet::fileexists(phasespaceFilePath()))
  {
    // Check event counter is initialised
    if (!pdf->isInitialised())
    {
      cerr << "MCgrid Error: Phase space optimised grid is available, but event count data is not present."<<endl;
      cerr << "                  Please rerun the phase space run to generate event count data." <<endl;
      exit(-1);
    }
  }
}

// Grid class destructor
_grid::~_grid()
{ 
  fl1projection-=5;
  fl2projection-=5;
  
  delete[] fl1projection;
  delete[] fl2projection;
  delete[] weights;
}


/**
 *  grid::fill
 *  Provides the conversion of a HepMC event into the appropriate
 *  fill call for applgrid or fastNLO, based upon the specified fillMode
 *  and interface
 **/
void _grid::fill( double coord, const Rivet::Event& event)
{
  switch (mode)
  {
    case FILL_GENERIC: {
      fillInfo info(event);  
      fillReferenceHistogram(coord, info.wgt);
      genericFill(coord, info);
      break;
    }
      
    case FILL_SHERPA: {
      sherpaFillInfo info(event);  
      fillReferenceHistogram(coord, info.wgt);
      sherpaFill(coord, info);
      break;
    }
  }
}


// Zeros the weight container
void _grid::zeroWeights()
{
  for (int i=0; i<nSubProc; i++)
    weights[i] = 0;
}


// Populate the subprocess weight array with a single weight
void _grid::fillWeight(const int fl1,
                      const int fl2,
                      const double eventweight,
                      const bool shouldApplyEventRatio
                      )
{
  const int subproc = pdf->decideSubProcess(fl1, fl2);
  const double norm = (shouldApplyEventRatio) ? pdf->EventRatio(fl1, fl2) : 1.0;
  weights[subproc] += norm * eventweight;
}


void _grid::projectWeights ( const int fl1,  // beam 1 flavour
                            const int fl2,  // beam 2 flavour
                            const double wgt,  // weight to be projected
                            wgtProjector projectBeam1,
                            wgtProjector projectBeam2,
                            const bool shouldApplyEventRatio
                          )
{
  // Build projections
  projectBeam1(fl1, fl1projection);
  projectBeam2(fl2, fl2projection);

  // Project weights onto beam partons
  for ( int i=-5; i<=5; i++ )
    if (fl1projection[i])
      for( int j=-5; j<=5; j++ )
        if (fl2projection[j])
          fillWeight(i, j, fl1projection[i]*fl2projection[j]*wgt, shouldApplyEventRatio);
  return;
}

std::string _grid::gridInterfaceName(gridInterface interface) const
{
  std::string name(gridInstanceString[interface]);
  return name;
}

std::string _grid::analysisPath() const
{
  return analysisPathWithBasePath(MCgridOutputPath());
}
std::string _grid::phasespaceAnalysisPath() const
{
  return analysisPathWithBasePath(MCgridPhasespacePath());
}

std::string _grid::analysisPathWithBasePath(std::string basePath) const
{
  createPath(basePath);
  std::stringstream targetname;
  targetname << basePath;
  targetname << analysis << "/";
  createPath(targetname.str());
  return targetname.str();
}


std::string _grid::phasespaceFilePath() const
{
  std::stringstream targetname;
  targetname << phasespaceAnalysisPath() << "phasespace/";
  createPath(targetname.str());
  targetname << path << '.' << phasespaceFileExtension();
  return targetname.str();
}


std::string _grid::gridFilePath() const
{
  std::string analysis_path = analysisPath();
  size_t suffix_counter(0);
  while (Rivet::fileexists(analysis_path + gridFileName(suffix_counter))) {
    suffix_counter++;
  }
  return analysis_path + gridFileName(suffix_counter);
}

// Returns a unique grid file name
std::string _grid::gridFileName(size_t suffix_counter) const
{
  std::stringstream name;
  name << path << '.';
  if (suffix_counter > 0) {
    name << suffix_counter << '.';
  }
  name << gridFileExtension();
  return name.str();
}

std::string _grid::gridOrPhasespaceFilePath() const
{
  if (isWarmup()) {
    return phasespaceFilePath();
  } else {
    return gridFilePath();
  }
}

// Scale the weight output (actual implementation in superclasses)
void _grid::scale(double const& scale)
{
  cout << "MCgrid: Will set " << path << " normalisation to: " << scale << endl;
}
  
}
