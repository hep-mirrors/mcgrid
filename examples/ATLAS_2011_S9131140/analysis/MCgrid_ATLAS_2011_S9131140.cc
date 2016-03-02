// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/Projections/ZFinder.hh"

// Include MCgrid header
#include "mcgrid/mcgrid.hh"

namespace Rivet {

  using namespace Cuts;


  /// @brief ATLAS Z pT in Drell-Yan events at 7 TeV with MCgrid interface
  /// @author Enrico Bothmann, adapted from Elena Yatsenko, Judith Katzy
  class MCgrid_ATLAS_2011_S9131140 : public Analysis {
  public:

    /// Constructor
    MCgrid_ATLAS_2011_S9131140()
    : Analysis("MCgrid_ATLAS_2011_S9131140")
    {
      _sumw_el_bare = 0;
      _sumw_mu_bare = 0;
    }


    /// @name Analysis methods
    //@{

    void init() {

      // Set up projections
      FinalState fs;
      Cut cut = etaIn(-2.4,2.4) & (pT >= 20.0*GeV);

      // Set up projections
      ZFinder zfinder_bare_el(fs, cut, PID::ELECTRON, 66.0*GeV, 116.0*GeV, 0.0, ZFinder::NOCLUSTER);
      addProjection(zfinder_bare_el, "ZFinder_bare_el");
      ZFinder zfinder_bare_mu(fs, cut, PID::MUON, 66.0*GeV, 116.0*GeV, 0.0, ZFinder::NOCLUSTER);
      addProjection(zfinder_bare_mu, "ZFinder_bare_mu");

      // Book histograms
      _hist_zpt_el_bare        = bookHisto1D(1, 1, 3);  // electron "bare"
      _hist_zpt_mu_bare        = bookHisto1D(2, 1, 3);  // muon "bare"

#if USE_APPL
      //const string subproc_config_appl_file("MCgrid_ATLAS_2011_S9131140.config");
      const string subproc_config_appl_file("basic.config");
      MCgrid::subprocessConfig subproc_config_appl(subproc_config_appl_file,
                                                   MCgrid::BEAM_PROTON,
                                                   MCgrid::BEAM_PROTON);
      MCgrid::applGridArch arch_appl(50, 1, 5, 0);
      MCgrid::applGridConfig config_appl(0, subproc_config_appl, arch_appl, 1E-5, 1, 8315.18, 8315.18, false, "f2");
      _appl_zpt_el_bare = MCgrid::bookGrid(_hist_zpt_el_bare, histoDir(), config_appl);
      _appl_zpt_mu_bare = MCgrid::bookGrid(_hist_zpt_mu_bare, histoDir(), config_appl);
#endif

#if USE_FNLO
      //const string subproc_config_fnlo_file("MCgrid_ATLAS_2011_S9131140.str");
      const string subproc_config_fnlo_file("basic.str");
      MCgrid::subprocessConfig subproc_config_fnlo(subproc_config_fnlo_file,
                                                   MCgrid::BEAM_PROTON,
                                                   MCgrid::BEAM_PROTON);
      MCgrid::fastnloGridArch arch_fnlo(50, 1, "Lagrange", "OneNode", "sqrtlog10", "linear");
      MCgrid::fastnloConfig config_fnlo(0, subproc_config_fnlo, arch_fnlo, 7000.0);
      _fnlo_zpt_el_bare = MCgrid::bookGrid(_hist_zpt_el_bare, histoDir(), config_fnlo);
      _fnlo_zpt_mu_bare = MCgrid::bookGrid(_hist_zpt_mu_bare, histoDir(), config_fnlo);
#endif
    }


    /// Do the analysis
    void analyze(const Event& evt) {

      // Tell MCgrid to handle the event
      MCgrid::PDFHandler::HandleEvent(evt, histoDir());

      const double weight = evt.weight();

      const ZFinder& zfinder_bare_el = applyProjection<ZFinder>(evt, "ZFinder_bare_el");
      if (!zfinder_bare_el.bosons().empty()) {
        _sumw_el_bare += weight;
        const FourMomentum pZ = zfinder_bare_el.bosons()[0].momentum();
        _hist_zpt_el_bare->fill(pZ.pT()/GeV, weight);
#if USE_APPL
        _appl_zpt_el_bare->fill(pZ.pT()/GeV, evt);
#endif
#if USE_FNLO
        _fnlo_zpt_el_bare->fill(pZ.pT()/GeV, evt);
#endif
      }

      const ZFinder& zfinder_bare_mu = applyProjection<ZFinder>(evt, "ZFinder_bare_mu");
      if (!zfinder_bare_mu.bosons().empty()) {
        _sumw_mu_bare += weight;
        const FourMomentum pZ = zfinder_bare_mu.bosons()[0].momentum();
        _hist_zpt_mu_bare->fill(pZ.pT()/GeV, weight);
#if USE_APPL
        _appl_zpt_mu_bare->fill(pZ.pT()/GeV, evt);
#endif
#if USE_FNLO
        _fnlo_zpt_mu_bare->fill(pZ.pT()/GeV, evt);
#endif
      }

    }


    void finalize() {
      if (_sumw_el_bare    != 0)
      {
        scale(_hist_zpt_el_bare, 1/_sumw_el_bare);
#if USE_APPL
        _appl_zpt_el_bare->scale(1/_sumw_el_bare);
#endif
#if USE_FNLO
        _fnlo_zpt_el_bare->scale(1/_sumw_el_bare);
#endif
      }
      if (_sumw_mu_bare    != 0)
      {
        scale(_hist_zpt_mu_bare, 1/_sumw_mu_bare);
#if USE_APPL
        _appl_zpt_mu_bare->scale(1/_sumw_mu_bare);
#endif
#if USE_FNLO
        _fnlo_zpt_mu_bare->scale(1/_sumw_mu_bare);
#endif
      }

#if USE_APPL
      _appl_zpt_el_bare->exportgrid();
      _appl_zpt_mu_bare->exportgrid();
#endif
#if USE_FNLO
      _fnlo_zpt_el_bare->exportgrid();
      _fnlo_zpt_mu_bare->exportgrid();
#endif

      // Clear MCgrid to finalize
      MCgrid::PDFHandler::CheckOutAnalysis(histoDir());
    }

    //@}


  private:

    double _sumw_el_bare;
    double _sumw_mu_bare;
    
    Histo1DPtr _hist_zpt_el_bare;
    Histo1DPtr _hist_zpt_mu_bare;
    
    // Grids
#if USE_APPL
    MCgrid::gridPtr _appl_zpt_el_bare;
    MCgrid::gridPtr _appl_zpt_mu_bare;
#endif
#if USE_FNLO
    MCgrid::gridPtr _fnlo_zpt_el_bare;
    MCgrid::gridPtr _fnlo_zpt_mu_bare;
#endif
  };
  
  
  // Hook for the plugin system
  DECLARE_RIVET_PLUGIN(MCgrid_ATLAS_2011_S9131140);
  
}
