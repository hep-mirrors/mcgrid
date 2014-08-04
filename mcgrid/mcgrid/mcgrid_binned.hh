//
//  mcgrid_binned.hh
//  MCgrid 04/08/2014.
//

#ifndef mcgrid_binned_hh
#define mcgrid_binned_hh

#include <map>

#include "mcgrid.hh"

// Rivet includes
#include "Rivet/Rivet.hh"
#include "Rivet/Event.hh"


using Rivet::cerr;
using Rivet::cout;
using Rivet::endl;

namespace MCgrid
{  
  // ********************** Binned grids ***********************
  template<typename T>
  class BinnedGrid {
  public:
    // BinnedGrid constructor
    BinnedGrid();
    
    const void addGrid(const T& binMin, const T& binMax, gridPtr grid);
    
    /// Fill the histogram that lies in the same region as @a bin with the value
    /// @a val of weight @a weight.
    gridPtr fill(const T& bin, const double& val, const Rivet::Event& event);
    
    /// Scale/normalise histograms
    void scale(const double& scale);
    
    // Export contained grids
    void exportgrids();
    
  private:
    std::map<T, gridPtr> gridsByUpperBound;
    std::map<T, gridPtr> gridsByLowerBound;
    std::vector<gridPtr> grids;
    std::map<gridPtr, T> binWidths;
  };
  
  
  template<class T>
  BinnedGrid<T>::BinnedGrid()
  {
    cout << "MCgrid::BinnedGrid initialised"<<endl;
    cout << " ** Warning, BinnedGrids can me extremely memory intensive **"<<endl;
    cout << " ** Ensure subprocess identification is enabled ** "<<endl;
    return;
  }
  
  template<class T>
  const void BinnedGrid<T>::addGrid(const T& binMin, const T& binMax, gridPtr grid)
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
    binWidths.insert(std::make_pair(grid, binMax-binMin));
    
    return;
  }
  
  /*
   * BinnedGrid::fill -> largely duplicated from Rivet::BinnedHistogram::fill
   */
  template<class T>
  gridPtr BinnedGrid<T>::fill(const T& bin, const double& val, const Rivet::Event& event)
  {
    typename std::map<T, gridPtr>::iterator iGrid = gridsByUpperBound.upper_bound(bin);
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
   * BinnedGrid::scale -> scales grids, taking into account relative bin widths
   */
  template<class T>
  void BinnedGrid<T>::scale(const double& scale)
  {
    typename std::map<T, gridPtr>::iterator iGrid = binWidths.begin();
    for (; iGrid != binWidths.end(); iGrid++)
    {
      (*iGrid).second->scale(scale*(*iGrid).first);
    }
  }
  
  /*
   * BinnedGrid::exportgrids -> writes out constituent grids
   */
  template<class T>
  void BinnedGrid<T>::exportgrids()
  {
    for (int i=0; i<grids.size(); i++)
      grids[i]->exportgrid();
  }
  
}

#endif
