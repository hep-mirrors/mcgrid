//
//  mcgrid.hh
//  MCgrid 03/05/2015.
//

#ifndef MCgrid_grid_h
#define MCgrid_grid_h

#include <string>

#include "mcgrid/mcgrid.hh"
#include "mcgrid/mcgrid_pdf.hh"

#include "mcgrid.hh"

// Forward decl
namespace MCgrid{ class fillInfo; class sherpaFillInfo; }

namespace MCgrid {

typedef void (*wgtProjector)(const int, double*);

// Returns vector of lower bin edges for appl_grid constructor
static std::vector<double> getBinning( const Rivet::Histo1DPtr histo)
{
  std::vector<double> lowEdges;
  for (size_t i=0; i<histo.get()->numBins(); i++)
    lowEdges.push_back( histo.get()->bin(i).xMin() );
  
  // Root needs the highest bin also
  lowEdges.push_back(histo.get()->bin(lowEdges.size()-1).xMax());
  
  return lowEdges;
}

// Empty implementation for (abstract) grid declaration in public header
// Used in `bookGrid` to return a dummy object when the MCGRID_DISABLED env var is set
class grid_dummy : public grid {
public:
  void fill( double coord, const Rivet::Event& event) {}
  void exportgrid() {}
  void scale( double const& scale) {}
};

// Concrete implementation for (abstract) grid declaration in public header
class _grid : public grid {
public:

  // Create a new grid based upon a YODA histogram
  _grid(const Rivet::Histo1DPtr,
        const std::string _analysis);

  ~_grid();

protected:
  
  std::string gridInterfaceName(gridInterface) const;
  std::string phasespaceFilePath() const;
  std::string gridOrPhasespaceFilePath() const;
  std::string gridFileName(size_t suffix_counter) const;

  // Setup subprocess configuration PDF
  void readPDFWithParameters(mcgrid_base_pdf_params const &, const std::string & analysis);

  // Scale the weight output of the grid
  virtual void scale( double const& scale);

  const Rivet::Histo1DPtr histo;  //!< Pointer to associated rivet histogram
  const std::string path;         //!< Path identifier obtained from histogram
  const std::string analysis;     //!< Analysis subdirectory

  int   leadingOrder;             //!< Leading order of process
  const fillMode  mode;           //!< Determines fill method call

  int        nSubProc;            //!< Number of active subprocesses
  mcgrid_base_pdf* pdf;           //!< PDF for subprocess classification
  double*    weights;             //!< Array of subprocess weights to be passed to appl::grid::fill or fastNLOCreate::fill

private:

  // Fill the grid with an event
  void fill( double coord, const Rivet::Event& event);
  virtual void fillReferenceHistogram(double coord, double wgt) = 0;
  
  // Write the grid to file
  virtual void exportgrid() = 0;

  // Zeros the internal weight container;
  void zeroWeights();
  
  // Fillmodes specify the conversion from HepMC
  void genericFill(double coord, fillInfo const&);  // Basic fillmode
  void sherpaFill( double coord, sherpaFillInfo const&); // SHERPA fillmode
  void sherpaBLikeFill(double coord, double norm, fillInfo const&, int ptord);
  void sherpaKPFill(double coord, double norm, sherpaFillInfo const&);

  virtual void fillUnderlyingGrid(const double x1,
                                  const double x2,
                                  const double pdfQ2,
                                  const double coord,
                                  const double ptord) = 0;
  
  // Populate the subprocess weight array with a single weight
  // The weight is multiplied by the return value of the EventRatio function,
  // if shouldApplyEventRatio is true
  void fillWeight(const int fl1,
                  const int fl2,
                  const double eventweight,
                  const bool shouldApplyEventRatio
                  );
  
  // Project a weight across other parton channels based upon the basic
  // initial beam flavours.
  void projectWeights( const int fl1,                     // beam 1 flavour
                       const int fl2,                     // beam 2 flavour
                       const double w,                    // weight to be projected
                       wgtProjector,                      // beam 1 projector
                       wgtProjector,                      // beam 2 projector
                       const bool shouldApplyEventRatio   // passed to fillWeight function
  );

  virtual std::string gridInterfaceName() const = 0;
  virtual std::string phasespaceFileExtension() const = 0;
  virtual std::string gridFileExtension() const = 0;
  std::string analysisPath() const;
  std::string phasespaceAnalysisPath() const;
  std::string analysisPathWithBasePath(std::string basePath) const;
  std::string gridFilePath() const;
  virtual bool isWarmup() const = 0;
  
  // **************************** Attributes ****************************
  
  double* fl1projection;          //!< Projection of weights over beam 1 partons
  double* fl2projection;          //!< Projection of weights over beam 2 partons
};

}

#endif
