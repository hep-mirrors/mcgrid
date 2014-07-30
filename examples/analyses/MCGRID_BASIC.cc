// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/Projections/FinalState.hh"
/// @todo Include more projections as required, e.g. ChargedFinalState, FastJets, ZFinder...

// MCgrid headers
#include "mcgrid/mcgrid.hh"

namespace Rivet {


  class MCgrid_BASIC : public Analysis {
  public:

    /// Constructor
    MCgrid_BASIC()
      : Analysis("MCgrid_BASIC")
    {    }


  public:

    /// @name Analysis methods
    //@{

    /// Book histograms and initialise projections before the run
    void init() {

      /// @todo Initialise and register projections here

      /// @todo Book histograms here, e.g.:
      // _h_XXXX = bookProfile1D(1, 1, 1);
      // _h_YYYY = bookHisto1D(2, 1, 1);

    }


    /// Perform the per-event analysis
    void analyze(const Event& event) {
      // MCgrid event counter
      MCgrid::PDFHandler::HandleEvent(event);
      
      // const double weight = event.weight();

      /// @todo Do the event by event analysis here

    }


    /// Normalise histograms etc., after the run
    void finalize() {

      /// @todo Normalise, scale and otherwise manipulate histograms here

      // scale(_h_YYYY, crossSection()/sumOfWeights()); // norm to cross section
      // normalize(_h_YYYY); // normalize to unity
      
      // Clear MCgrid event counter
      MCgrid::PDFHandler::ClearHandler();
    }

    //@}


  private:

    // Data members like post-cuts event weight counters go here


  private:

    /// @name Histograms
    //@{
    Profile1DPtr _h_XXXX;
    Histo1DPtr _h_YYYY;
    //@}


  };



  // The hook for the plugin system
  DECLARE_RIVET_PLUGIN(MCgrid_BASIC);

}
