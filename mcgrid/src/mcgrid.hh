//
//  mcgrid.hh
//  MCgrid 21/07/2015.
//

#ifndef mcgrid_private_hh
#define mcgrid_private_hh

#include <string>

#include "config.h"

namespace MCgrid
{
  // Denotes the fill method to be used, i.e. the source format
  const std::string fillString[2] = {"GENERIC", "SHERPA"};
  typedef enum fillMode {FILL_GENERIC, FILL_SHERPA} fillMode;
#ifdef FILLMODE_SHERPA
  const fillMode globalFillMode = FILL_SHERPA;
#else
  const fillMode globalFillMode = FILL_GENERIC;
#endif

  // Denotes the grid interface to be used, i.e. the target format
  const std::string gridString[2] = {"APPLgrid", "fastNLO"};
  const std::string gridInstanceString[2] = {"APPLgrid", "fastNLO table"};
  typedef enum gridInterface {noInterface = -1, applgridInterface, fastnloInterface} gridInterface;

  // Denotes the number of active flavours. The default is 5, i.e. to exclude
  // the top only. Use the global function setNumberOfActiveFlavors defined in
  // the public mcgrid header to change this.
  extern int numberOfActiveFlavors;

  // The root mcgrid directories
  std::string MCgridPhasespacePath();  // defaults to `MCgridOutputPath()`
  std::string MCgridOutputPath();      // defaults to `./mcgrid`
}

#endif
