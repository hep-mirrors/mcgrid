//
//  mcgrid.hh
//  MCgrid 22/08/2013.
//

#ifndef MCgrid_h
#define MCgrid_h

#include "mcgrid_pdf.hh"

// Rivet includes
#include "Rivet/Rivet.hh"
#include "Rivet/Event.hh"
#include "Rivet/Tools/RivetYODA.hh"

// HEPMC
#include "HepMC/GenEvent.h"
#include "HepMC/PdfInfo.h"

#include <sys/stat.h>
#include <string>
#include <vector>
#include <cmath>

// Forward decl
namespace appl{ class grid; }

namespace MCgrid {
  
  // Denotes the fill method to be used
  const std::string fillString[2] = {"GENERIC", "SHERPA "};
  typedef enum fillMode {FILL_GENERIC, FILL_SHERPA} fillMode;
  
#ifdef FILLMODE_SHERPA
  const fillMode globalFillMode = FILL_SHERPA;
#else
  const fillMode globalFillMode = FILL_GENERIC;
#endif

  // Weight projection typedef
  typedef void (*wgtProjector)(const int, double*);
  
  // Convert PDG codes to LHAPDF for partons
  static inline int pdgToLHA( int pdg )
  {
    if (pdg == 21 ) return 0;
    return pdg;
  }
  
  // Create directory structure
  static inline void createPath(std::string path)
  {
    // Now just use a system call to avoid linking to boost
    mkdir(path.c_str(),0777);
  }
      
  // Fwd Decls
  class mcgrid_pdf;
  
  // ********************* fillInfo ************************

  /*
   * Basic information required in most fills
   */
  class fillInfo
  {
  public:
    fillInfo( Rivet::Event const&, int const& leadingOrder);

    const double wgt;
    const std::vector<double> usr_wgt;
    
    const int fl1;
    const int fl2;
    
    const double x1;
    const double x2;
    const double pdfQ2;
    
    const double fx1;
    const double fx2;

    const int aspowers;
    const double alphas;
    const double asfac;
    const double ptord;
    
  private:
    // Convert HepMC formatted usr_weights to local ordering
    std::vector<double> getHepMCWeights(Rivet::Event const&);
  };
  
  // ********************* appl::grid architectures ************************
  
  struct gridArch
  {
    gridArch( const int _nX,
              const int _nQ,
              const int _xOrd,
              const int _qOrd ):
    nX(_nX),
    nQ(_nQ),
    xOrd(_xOrd),
    qOrd(_qOrd)
    {}
    
    const int nX;       //!< Number of points in x-grid
    const int nQ;       //!< Number of points in Q-grid
    const int xOrd;     //!< Order of interpolation in x
    const int qOrd;     //!< Order of interpolation in Q
  };
  
  // Typical grid precisions
  static const gridArch highPrec(30,20,5,5);
  static const gridArch medPrec(20,10,3,3);
  static const gridArch lowPrec(10,5,2,2);
  
  // *********************** Booking Functions **************************
  
  // Book an mcgrid_pdf object based upon a subprocess combination file
  // Don't need to keep track of the pointer, it's kept by APPLgrid
  void bookPDF(std::string const& name, std::string const& analysis, beamType beam1, beamType beam);
  
  // Book a MCgrid::grid object, returning the shared pointer
  class grid; typedef boost::shared_ptr<grid> gridPtr;
  gridPtr bookGrid( const Rivet::Histo1DPtr hist,         // Corresponding Rivet Histogram
                    const std::string histoDir,           // Rivet Histogram directory
                    const std::string pdfname,            // APPLgrid pdf id string
                    const int    LO,                      // Leading order power of alpha_s
                    const double xmin,                    // Minimum value of parton x in the grid
                    const double xmax,                    // Maximum value of parton x
                    const double q2min,                   // Minimum event scale
                    const double q2max,                   // Maximum event scale
                    const gridArch arch = highPrec        // grid architecture
  );
 
  // **********************  rivet_app::grid Class **************************

  /**
   * MCgrid::grid provides a wrapper for an appl_grid object,
   * corresponding to a single Rivet histogram
   **/
  class grid {
  public:
    
    // Create a new APPLgrid based upon a YODA histogram
    grid(const Rivet::Histo1DPtr hist,
         const std::string pdfname,
         const std::string analysis,
         const int    LO,
         const double xmin,
         const double xmax,
         const double q2min,
         const double q2max,
         const gridArch arch
         );
    
    ~grid();
    
    // Fill the applgrid with an event
    void fill( double coord, const Rivet::Event& event);
    
    // Write the APPLgrid to file
    void exportgrid();
    
    // Scale the weight output of the APPLgrid
    void scale( double const& scale);
    
  private:
  
    // Zeros the internal weight container;
    void zeroWeights();
    
    // Fillmodes specify the conversion from HepMC to APPLgrid
    void genericFill( double coord, fillInfo const&); // Basic fillmode
    void sherpaFill ( double coord, fillInfo const&); // SHERPA fillmode
    
    // Populate the subprocess weight array with a single weight
    void fillWeight(const int fl1,
                    const int fl2,
                    const double eventweight
                    );
    
    // Project a weight across other parton channels based upon the basic
    // initial beam flavours.
    void projectWeights( const int fl1,   // beam 1 flavour
                         const int fl2,   // beam 2 flavour
                         const double w,  // weight to be projected
                         wgtProjector,    // beam 1 projector
                         wgtProjector     // beam 2 projector
    );
    
    // **************************** Attributes ****************************
    
    const Rivet::Histo1DPtr histo;  //!< Pointer to associated rivet histogram
    const fillMode  mode;           //!< Determines fill method call
    
    const int   leadingOrder;       //!< Leading order of process
    const std::string path;         //!< Path identifier obtained from histogram
    const std::string analysis;     //!< Analysis subdirectory
    
    mcgrid_pdf*   applpdf;          //!< PDF for subprocess classification
    appl::grid*  applgrid;          //!< Pointer to APPLgrid
    
    const int nSubProc;             //!< Number of active subprocesses
    double*    weights;             //!< Array of subprocess weights to be passed to appl::grid::fill
    
    double* fl1projection;          //!< Projection of weights over beam 1 partons
    double* fl2projection;          //!< Projection of weights over beam 2 partons
    
  };
  
   // ********************* /MCgrid *****************************
}




#endif
