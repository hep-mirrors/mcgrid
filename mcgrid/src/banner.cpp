//
//  banner.cpp
//  MCgrid 17/03/2015.
//

#include <string>
#include <sstream>

#include "config.h"

#include "banner.hh"

#include "mcgrid/mcgrid.hh"
#include "mcgrid.hh"

using Rivet::cerr;
using Rivet::cout;
using Rivet::endl;

namespace MCgrid {

static const std::string::size_type bannerWidth = 65;
static const char bannerFrameChar = '*';

static void showBanner();
static void showBannerFrameLine();
static void showBannerContentLine(std::string const &);

void showDisabledInfoOnce()
{
  static bool infoShown = false;
  if (!infoShown)
    cout << "MCgrid: MCGRID_DISABLED environment variable is set. Will do nothing" << endl;
  infoShown = true;
}

void showBannerOnce()
{
  static bool bannerShown = false;
  if (!bannerShown)
    showBanner();
  bannerShown = true;
}

static void showBanner()
{
  cout << endl;
  showBannerFrameLine();
  showBannerContentLine("MCgrid plugin for Rivet");
  showBannerContentLine("E. Bothmann, L. Del Debbio, N.P. Hartland, S. Schumann");
  std::stringstream referenceStream;
  referenceStream << "arXiv:1312.4460, Version: " << PACKAGE_VERSION;
  showBannerContentLine(referenceStream.str());
  showBannerContentLine("");

  // Output fill mode
  std::stringstream fillModeStream;
  fillModeStream << "-- Configured for " << fillString[globalFillMode] << " fillmode --";
  showBannerContentLine(fillModeStream.str());

  // Output supported grid interfaces for this installation
  std::stringstream interfaceListStream;
  interfaceListStream << "-- Grid interfaces:";

#if APPLGRID_ENABLED
  interfaceListStream << " " << gridString[applgridInterface];
#endif

#if FASTNLO_ENABLED
#if APPLGRID_ENABLED
  interfaceListStream << ",";
#endif
  interfaceListStream << " " << gridString[fastnloInterface];
#endif

  interfaceListStream << " --";
  showBannerContentLine(interfaceListStream.str());

  showBannerFrameLine();
  cout << endl;
}

static void showBannerFrameLine()
{
  static const std::string frame(bannerWidth, bannerFrameChar);
  cout << frame << endl;
}

static void showBannerContentLine(std::string const & line)
{
  const std::string::size_type totalMarginSize = bannerWidth - line.size() - 2;
  const std::string::size_type oneSideMarginSize = totalMarginSize / 2;
  const std::string margin(oneSideMarginSize, ' ');
  cout << bannerFrameChar << margin << line << margin;
  if (totalMarginSize % 2 == 1) {
    cout << ' ';
  }
  cout << bannerFrameChar << endl;
}

}