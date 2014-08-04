//
//  mcgrid_binned.hh
//  MCgrid 04/08/2014.
//


#include <map>

#ifndef mcgrid_grids_hh
#define mcgrid_grids_hh

namespace MCgrid
{
  
  // Forwards
  class grid; typedef boost::shared_ptr<grid> gridPtr;

  
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
    
  private:
    std::map<T, gridPtr> gridsByUpperBound;
    std::map<T, gridPtr> gridsByLowerBound;
    std::vector<gridPtr> grids;
    std::map<gridPtr, T> binWidths;
  };
  
}
#endif
