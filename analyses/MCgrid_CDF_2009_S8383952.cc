// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/Projections/FinalState.hh"
#include "Rivet/Projections/ZFinder.hh"

#include "mcgrid/mcgrid.hh"

namespace Rivet {

  /// @brief CDF Z boson rapidity measurement
  /// modified to generate APPLgrid files
  class MCgrid_CDF_2009_S8383952 : public Analysis {
  public:

    /// @name Constructors etc.
    //@{

    /// Constructor
    MCgrid_CDF_2009_S8383952()
      : Analysis("MCgrid_CDF_2009_S8383952")
    {    }

    //@}


  public:

    /// @name Analysis methods
    //@{

    /// Book histograms and initialise projections before the run
    void init() {

      /// Initialise and register projections here
      // this seems to have been corrected completely for all selection cuts,
      // i.e. eta cuts and pT cuts on leptons.
      FinalState fs;
      ZFinder zfinder(FinalState(), -MAXDOUBLE, MAXDOUBLE, 0*GeV, PID::ELECTRON,
                      66*GeV, 116*GeV, 0.2, ZFinder::CLUSTERNODECAY, ZFinder::TRACK);
      addProjection(zfinder, "ZFinder");
      
      /// Book histograms here
      _h_xs = bookHisto1D(1, 1, 1);
      _h_yZ = bookHisto1D(2, 1, 1);
      
      // Book APPLgrids
      MCgrid::gridArch arch(50,1,5,0);
      const string PDFname = "MCgrid_CDF_2009_S8383952.config";
      
      MCgrid::bookPDF(PDFname, histoDir(), MCgrid::BEAM_PROTON, MCgrid::BEAM_ANTIPROTON);
      _a_yZ = MCgrid::bookGrid(_h_yZ, histoDir(), PDFname, 0, 1E-5, 1, 8315.18, 8315.18, arch);
      _a_xs = MCgrid::bookGrid(_h_xs, histoDir(), PDFname, 0, 1E-5, 1, 8315.18, 8315.18, arch);

    }


    /// Perform the per-event analysis
    void analyze(const Event& event) {
      
      // Handle APPL event
      MCgrid::PDFHandler::HandleEvent(event);
      
      const double weight = event.weight();

      const ZFinder& zfinder = applyProjection<ZFinder>(event, "ZFinder");
      if (zfinder.bosons().size() == 1) {
        double yZ = fabs(zfinder.bosons()[0].momentum().rapidity());
        _h_yZ->fill(yZ, weight);
        _h_xs->fill(1960.0, weight);
        
        _a_yZ->fill(yZ,event);
        _a_xs->fill(1960.0,event);

      }
      else {
        MSG_DEBUG("no unique lepton pair found.");
      }

    }


    /// Normalise histograms etc., after the run
    void finalize() {
      scale(_h_xs, crossSection()/sumOfWeights());
      _a_xs->scale(crossSection()/sumOfWeights());

      // Data seems to have been normalized for the avg of the two sides
      // (+ve & -ve rapidity) rather than the sum, hence the 0.5:
      scale(_h_yZ, 0.5*crossSection()/sumOfWeights());
      _a_yZ->scale(0.5*crossSection()/sumOfWeights());

      // Export APPLgrids
      _a_yZ->exportgrid();
      _a_xs->exportgrid();
      
      // Clear event handler
      MCgrid::PDFHandler::ClearHandler();
    }

    //@}


  private:

    /// @name Histograms
    //@{
    Histo1DPtr _h_yZ;
    Histo1DPtr _h_xs;
    //@}
    
    // APPLgrids
    MCgrid::gridPtr _a_yZ;
    MCgrid::gridPtr _a_xs;

  };



  // The hook for the plugin system
  DECLARE_RIVET_PLUGIN(MCgrid_CDF_2009_S8383952);

}
