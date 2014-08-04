//
//  mcgrid_binned.cpp
//  MCgrid 04/08/2014.
//

#include "mcgrid/mcgrid.hh"

#include <utility>

// Rivet includes
#include "Rivet/Rivet.hh"
#include "Rivet/Event.hh"

// HEPMC
#include "HepMC/GenEvent.h"
#include "HepMC/PdfInfo.h"

using namespace MCgrid;

using Rivet::cerr;
using Rivet::cout;
using Rivet::endl;

using std::map;

template<class T>
binnedGrid<T>::binnedGrid()
{
  cout << "MCgrid::binnedGrid initialised"<<endl;
  cout << " ** Warning, binnedGrids can me extremely memory intensive **"<<endl;
  cout << " ** Ensure subprocess identification is enabled ** "<<endl;
  return;
}

template<class T>
const void binnedGrid<T>::addGrid(const T& binMin, const T& binMax, gridPtr grid)
{
  if ( binMax <= binMin )
  {
    cerr << "MCgrid::binnnedGrid Error - invalid bin minimum ("<<binMin<<") and maximum ("<<binMax<<")."<<endl;
    exit(-1);
  }
  
  // Enter new grid
  grids.push_back(grid);
  
  const bool checkLower   = gridsByLowerBound.insert(std::make_pair(binMin,grid)).second;
  const bool checkHigher  = gridsByUpperBound.insert(std::make_pair(binMax,grid)).second;
  
  // Verify bounds
  if (!checkHigher || !checkLower)
  {
    cerr << "MCgrid::binnnedGrid Error - Duplicate bin ranges detected."<<endl;
    exit(-1);
  }
  
  // Width entry
  binWidths.push_back(std::make_pair(grid, binMax-binMin));

  return;
}

/*
 * binnedGrid::fill -> largely duplicated from Rivet::BinnedHistogram::fill
 */
template<class T>
gridPtr binnedGrid<T>::fill(const T& bin, const double& val, const Rivet::Event& event)
{
  typename map<T, gridPtr>::iterator iGrid = gridsByUpperBound.upper_bound(bin);
  if (iGrid == gridsByUpperBound.end()) {
    return gridPtr(); // should exit instead?
  }
  
  gridPtr grid = iGrid->second;
  iGrid = gridsByLowerBound.lower_bound(bin);
  
  if (iGrid == gridsByLowerBound.begin()) {
    return gridPtr();
  }
  --iGrid;
  
  if (grid != iGrid->second) {
    return gridPtr();
  }
  
  grid->fill(val, event);
  
  return grid;
}

/*
 * binnedGrid::scale -> scales grids, taking into account relative bin widths
 */
template<class T>
void binnedGrid<T>::scale(const double& scale)
{
  typename map<T, gridPtr>::iterator iGrid = binWidths.begin();
  for (; iGrid != binWidths.end(); iGrid++)
  {
    (*iGrid).second->scale(scale*(*iGrid).first);
  }
}