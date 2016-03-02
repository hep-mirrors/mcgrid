// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/Projections/ZFinder.hh"

#include "mcgrid/mcgrid.hh"

namespace Rivet {


  /// @brief Measurement of D0 Run II Z \f$ p_\perp \f$ diff cross-section shape
  /// @author Andy Buckley
  /// @author Gavin Hesketh
  /// @author Frank Siegert
  class MCgrid_D0_2007_S7075677 : public Analysis {
  public:

    /// Default constructor.
    MCgrid_D0_2007_S7075677()
      : Analysis("MCgrid_D0_2007_S7075677")
    {    }


    /// @name Analysis methods
    //@{

    /// Book histograms
    void init() {
      ZFinder zfinder(FinalState(), Cuts::open(), PID::ELECTRON,
                      71*GeV, 111*GeV, 0.2, ZFinder::CLUSTERNODECAY, ZFinder::TRACK);
      addProjection(zfinder, "ZFinder");

      _h_yZ = bookHisto1D(1, 1, 1);
      _h_xs = bookHisto1D("d02-x01-y01", 1, 0.0, 1.0);

#if USE_APPL
      //const string subproc_config_appl_file("MCgrid_D0_2007_S7075677.config");
      const string subproc_config_appl_file("basic.config");
      MCgrid::subprocessConfig subproc_config_appl(subproc_config_appl_file,
                                                   MCgrid::BEAM_PROTON,
                                                   MCgrid::BEAM_ANTIPROTON);
      MCgrid::applGridArch arch_appl(50, 1, 5, 0);
      MCgrid::applGridConfig config_appl(0, subproc_config_appl, arch_appl, 1E-5, 1, 8315.18, 8315.18, false, "f2");
      _appl_xs = MCgrid::bookGrid(_h_xs, histoDir(), config_appl);
      _appl_yZ = MCgrid::bookGrid(_h_yZ, histoDir(), config_appl);
#endif

#if USE_FNLO
      //const string subproc_config_fnlo_file("MCgrid_D0_2007_S7075677.str");
      const string subproc_config_fnlo_file("basic.str");
      MCgrid::subprocessConfig subproc_config_fnlo(subproc_config_fnlo_file,
                                                   MCgrid::BEAM_PROTON,
                                                   MCgrid::BEAM_ANTIPROTON);
      MCgrid::fastnloGridArch arch_fnlo(50, 1, "Lagrange", "OneNode", "sqrtlog10", "linear");
      MCgrid::fastnloConfig config_fnlo(0, subproc_config_fnlo, arch_fnlo, 1960.0);
      _fnlo_xs = MCgrid::bookGrid(_h_xs, histoDir(), config_fnlo);
      _fnlo_yZ = MCgrid::bookGrid(_h_yZ, histoDir(), config_fnlo);
#endif      
    }


    /// Do the analysis
    void analyze(const Event & e) {
      const double weight = e.weight();

      // Handle APPL event
      MCgrid::PDFHandler::HandleEvent(e, histoDir());

      const ZFinder& zfinder = applyProjection<ZFinder>(e, "ZFinder");
      if (zfinder.bosons().size() == 1) {
        const Particles& el(zfinder.constituents());
        if (el[0].pT() > 25*GeV || el[1].pT() > 25*GeV) {
          _h_yZ->fill(fabs(zfinder.bosons()[0].rapidity()), weight);
          _a_yZ->fill(fabs(zfinder.bosons()[0].rapidity()), e);
          _a_xs->fill(0.5, e);
#if USE_APPL
          _appl_yZ->fill(fabs(zfinder.bosons()[0].rapidity(),e);
          _appl_xs->fill(0.5,e);
#endif
#if USE_FNLO
          _fnlo_yZ->fill(fabs(zfinder.bosons()[0].rapidity(),e);
          _fnlo_xs->fill(0.5,e);
#endif
        }
      } else {
        MSG_DEBUG("No unique lepton pair found.");
      }
    }


    // Finalize
    void finalize() {
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
  DECLARE_RIVET_PLUGIN(MCgrid_D0_2007_S7075677);

}
