//
//  mcgrid.hh
//  MCgrid 22/08/2013.
//

#ifndef MCgrid_h
#define MCgrid_h

#include <string>

#include "Rivet/Rivet.hh"
#include "HepMC/GenEvent.h"

#include "mcgrid/mcgrid_pdf.hh"

namespace MCgrid {

  // ********************* subprocess configuration *******************

  struct subprocessConfig
  {
    subprocessConfig(std::string const & fileName, const beamType beam1, const beamType beam2):
      fileName(fileName),
      beam1(beam1),
      beam2(beam2) {}
    std::string const fileName; //!< Can be an APPLgrid subprocess config file or a fastNLO steering file
    const beamType beam1;
    const beamType beam2;   
  };

  // ********************* grid architectures *************************

  struct baseGridArch
  {
    baseGridArch(const int _nX, const int _nQ):
      nX(_nX),
      nQ(_nQ) {}
    const bool isEmpty() const { return (nX == 0 || nQ == 0); }
    const int nX;       //!< Number of points in x-grid
    const int nQ;       //!< Number of points in Q-grid
  };

  struct applGridArch : public baseGridArch
  {
    applGridArch(const int _nX, const int _nQ,
                 const int _xOrd, const int _qOrd):
      baseGridArch(_nX, _nQ),
      xOrd(_xOrd),
      qOrd(_qOrd) {}

    const int xOrd;     //!< Order of interpolation in x
    const int qOrd;     //!< Order of interpolation in Q
  };

  // Typical APPLgrid precisions
  static const applGridArch highPrecAPPLgridArch(30,20,5,5);
  static const applGridArch medPrecAPPLgridArch(20,10,3,3);
  static const applGridArch lowPrecAPPLgridArch(10,5,2,2);

  struct fastnloGridArch : public baseGridArch
  {
    fastnloGridArch(const int _nX, const int _nQ,
                    std::string const _xkernel,
                    std::string const _qkernel,
                    std::string const _xdistanceMeasure,
                    std::string const _qdistanceMeasure,
                    bool _nXPerMagnitude = false):
      baseGridArch(_nX, _nQ),
      xkernel(_xkernel),
      qkernel(_qkernel),
      xdistanceMeasure(_xdistanceMeasure),
      qdistanceMeasure(_qdistanceMeasure),
      nXPerMagnitude(_nXPerMagnitude) {}

    const bool isEmpty() const {
      return (baseGridArch::isEmpty()
        || xkernel == "" || qkernel == ""
        || xdistanceMeasure == "" || qdistanceMeasure == "");
    }

    // Interpolation kernels as of fastnlo_toolkit-2.3.1pre-1871:
    // Catmull, Lagrange, OneNode, Linear
    std::string const xkernel; //!< Value for the x interpolation kernel
    std::string const qkernel; //!< Value for the Q interpolation kernel

    // Distance measures as of fastnlo_toolkit-2.3.1pre-1871:
    // linear, loglog025, log10, sqrtlog10
    std::string const xdistanceMeasure; //!< Value for the x distance measure
    std::string const qdistanceMeasure; //!< Value for the Q distance measure

    const bool nXPerMagnitude;
  };

  // Typical APPLgrid precisions
  static const fastnloGridArch emptyFastNLOgridArch(0,0,"","", "", "");
  static const fastnloGridArch highPrecFastNLOgridArch(30,20,"Lagrange","Lagrange", "sqrtlog10", "loglog025");
  static const fastnloGridArch medPrecFastNLOgridArch(20,10,"Lagrange","Lagrange", "sqrtlog10", "loglog025");
  static const fastnloGridArch lowPrecFastNLOgridArch(10,5,"Lagrange","Lagrange", "sqrtlog10", "loglog025");
  
  // ********************* grid configurations ************************

  struct gridConfig
  {
    gridConfig(const int _lo):
      lo(_lo) {}

    // This is just to make this type polymorphic for later downcasts
    virtual ~gridConfig() {}

    const int lo;
  };

  struct applGridConfig : public gridConfig
  {
    applGridConfig(const int _lo,
                   const subprocessConfig _subprocConfig,
                   const applGridArch _arch,
                   const double _xmin,
                   const double _xmax,
                   const double _q2min,
                   const double _q2max,
                   const bool _shouldUseScaleLogGrids = false,
                   const std::string _xMappingFunctionName = "f2"): 
      gridConfig(_lo),
      subprocConfig(_subprocConfig),
      arch(_arch),
      xmin(_xmin),
      xmax(_xmax),
      q2min(_q2min),
      q2max(_q2max),
      shouldUseScaleLogGrids(_shouldUseScaleLogGrids),
      xMappingFunctionName(_xMappingFunctionName) {}

    const subprocessConfig subprocConfig;
    const applGridArch arch;
    const double xmin;
    const double xmax;
    const double q2min;
    const double q2max;
    const bool shouldUseScaleLogGrids;      //!< Whether extra scale log grids should be used
    const std::string xMappingFunctionName; //!< x-to-y mapping function name, can be f, f0, ..., f4, cf. appl_igrid.h
  };

  struct fastnloConfig: public gridConfig
  {
    fastnloConfig(const int _lo,
                  const subprocessConfig _subprocConfig,
                  const fastnloGridArch _arch,
                  const double _centerOfMassEnergy):
      gridConfig(_lo),
      subprocConfig(_subprocConfig),
      arch(_arch),
      centerOfMassEnergy(_centerOfMassEnergy) {}

    const subprocessConfig subprocConfig;
    const fastnloGridArch arch;
    const double centerOfMassEnergy;    //!< Center of mass energy in GeV
  };

  // **********************   Config Functions **************************

  // Set the number of active flavours, this affects Catani Seymour KP terms.
  // The default number of active flavours is 5, i.e. the top is excluded.
  // If you have a massive bottom quark, you might want to set this to 4.
  void setNumberOfActiveFlavors(const int n);
  
  // *********************** Booking Functions **************************
    
  // Book a MCgrid::grid object, returning the shared pointer
  class grid; typedef std::shared_ptr<grid> gridPtr;
  template<class T>
  gridPtr bookGrid(const Rivet::Histo1DPtr hist,     // Corresponding Rivet Histogram
                   const std::string histoDir,       // Rivet Histogram directory
                   T config                          // Either a fastNLOConfig or an applGridConfig instance
                   );
 
  // **********************  MCgrid::grid Class **************************

  /**
   * MCgrid::grid provides a wrapper for an appl_grid or fast_nlo object,
   * corresponding to a single Rivet histogram
   **/

  class grid {
  public:
    
    // Fill the grid with an event
    virtual void fill( double coord, const Rivet::Event& event) = 0;
    
    // Write the grid to file
    virtual void exportgrid() = 0;
    
    // Scale the weight output of the grid
    virtual void scale( double const& scale) = 0;

  };
  
}

#endif
