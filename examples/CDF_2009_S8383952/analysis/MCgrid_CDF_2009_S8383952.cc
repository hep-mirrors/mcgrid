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
      ZFinder zfinder(FinalState(), Cuts::open(), PID::ELECTRON,
                      66*GeV, 116*GeV, 0.2, ZFinder::CLUSTERNODECAY, ZFinder::TRACK);
      addProjection(zfinder, "ZFinder");
      
      /// Book histograms here
      _h_xs = bookHisto1D(1, 1, 1);
      _h_yZ = bookHisto1D(2, 1, 1);
      
#if USE_APPL
      // Subprocess identification or full set of subprocesses
      //const string subproc_config_appl_file("MCgrid_CDF_2009_S8383952.config");
      const string subproc_config_appl_file("basic.config");
      MCgrid::subprocessConfig subproc_config_appl(subproc_config_appl_file,
                                                   MCgrid::BEAM_PROTON,
                                                   MCgrid::BEAM_ANTIPROTON);
      // Grid architecture suitable for fixed/flexible scales
      //MCgrid::applGridArch arch_appl(50, 1, 5, 0);
      //const double minstartingscale(8315.18);
      //const double maxstartingscale(8315.18);
      MCgrid::applGridArch arch_appl(MCgrid::highPrecAPPLgridArch);
      const double minstartingscale(1);
      const double maxstartingscale(1E7);
#if USE_SCALE_LOG_GRIDS
      const bool should_use_scale_log_grids(true);
#else
      const bool should_use_scale_log_grids(false);
#endif
      MCgrid::applGridConfig config_appl(0, subproc_config_appl, arch_appl,
		                         1E-5, 1,
                                         minstartingscale, maxstartingscale,
					 should_use_scale_log_grids, "f2");
      _appl_xs = MCgrid::bookGrid(_h_xs, histoDir(), config_appl);
      _appl_yZ = MCgrid::bookGrid(_h_yZ, histoDir(), config_appl);
#endif

#if USE_FNLO
      // Subprocess identification or full set of subprocesses
      //const string subproc_config_fnlo_file("MCgrid_CDF_2009_S8383952.str");
      const string subproc_config_fnlo_file("basic.str");
      MCgrid::subprocessConfig subproc_config_fnlo(subproc_config_fnlo_file,
                                                   MCgrid::BEAM_PROTON,
                                                   MCgrid::BEAM_ANTIPROTON);
      // Grid architecture suitable for fixed/flexible scales
      /* MCgrid::fastnloGridArch arch_fnlo(50, 1, "Lagrange", "OneNode", "sqrtlog10", "linear"); */
      MCgrid::fastnloGridArch arch_fnlo(MCgrid::highPrecFastNLOgridArch);
      MCgrid::fastnloConfig config_fnlo(0, subproc_config_fnlo, arch_fnlo, 1960.0);
      _fnlo_xs = MCgrid::bookGrid(_h_xs, histoDir(), config_fnlo);
      _fnlo_yZ = MCgrid::bookGrid(_h_yZ, histoDir(), config_fnlo);
#endif
    }


    /// Perform the per-event analysis
    void analyze(const Event& event) {
      
      // Handle APPL event
      MCgrid::PDFHandler::HandleEvent(event, histoDir());
      
      const double weight = event.weight();

      const ZFinder& zfinder = applyProjection<ZFinder>(event, "ZFinder");
      if (zfinder.bosons().size() == 1) {
        double yZ = fabs(zfinder.bosons()[0].momentum().rapidity());
        _h_yZ->fill(yZ, weight);
        _h_xs->fill(1960.0, weight);
        
#if USE_APPL
        _appl_yZ->fill(yZ,event);
        _appl_xs->fill(1960.0,event);
#endif
#if USE_FNLO
        _fnlo_yZ->fill(yZ,event);
        _fnlo_xs->fill(1960.0,event);
#endif

      }
      else {
        MSG_DEBUG("no unique lepton pair found.");
      }

    }


    /// Normalise histograms etc., after the run
    void finalize() {
      scale(_h_xs, crossSection()/sumOfWeights());

      // Data seems to have been normalized for the avg of the two sides
      // (+ve & -ve rapidity) rather than the sum, hence the 0.5:
      scale(_h_yZ, 0.5*crossSection()/sumOfWeights());

      // Export grids
#if USE_APPL
      _appl_yZ->scale(0.5*crossSection()/sumOfWeights());
      _appl_xs->scale(crossSection()/sumOfWeights());
      _appl_yZ->exportgrid();
      _appl_xs->exportgrid();
#endif
#if USE_FNLO
      _fnlo_yZ->scale(0.5*crossSection()/sumOfWeights());
      _fnlo_xs->scale(crossSection()/sumOfWeights());
      _fnlo_yZ->exportgrid();
      _fnlo_xs->exportgrid();
#endif

      // Clear event handler
      MCgrid::PDFHandler::CheckOutAnalysis(histoDir());
    }

    //@}


  private:

    /// @name Histograms
    //@{
    Histo1DPtr _h_yZ;
    Histo1DPtr _h_xs;
    //@}

    // Grids
#if USE_APPL
    MCgrid::gridPtr _appl_yZ;
    MCgrid::gridPtr _appl_xs;
#endif
#if USE_FNLO
    MCgrid::gridPtr _fnlo_yZ;
    MCgrid::gridPtr _fnlo_xs;
#endif

  };



  // The hook for the plugin system
  DECLARE_RIVET_PLUGIN(MCgrid_CDF_2009_S8383952);

}
