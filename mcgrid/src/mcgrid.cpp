//
//  mcgrid.cpp
//  MCgrid 21/07/2015.
//

#include "mcgrid.hh"

#include "system.hh"

namespace MCgrid
{
  int numberOfActiveFlavors = 5;

  void setNumberOfActiveFlavors(const int n)
  {
    numberOfActiveFlavors = n;
  }

  std::string MCgridPhasespacePath()
  {
    std::string MCgridPhasespacePathFromEnvironment = environmentVariableForKey("MCGRID_PHASESPACE_PATH");
    if (MCgridPhasespacePathFromEnvironment != "") {
      return MCgridPhasespacePathFromEnvironment;
    } else {
      return MCgridOutputPath();
    }
  }

  std::string MCgridOutputPath()
  {
    std::string MCgridOutputPathFromEnvironment = environmentVariableForKey("MCGRID_OUTPUT_PATH");
    if (MCgridOutputPathFromEnvironment != "") {
      return MCgridOutputPathFromEnvironment;
    } else {
      return "mcgrid";
    }
  }
}
