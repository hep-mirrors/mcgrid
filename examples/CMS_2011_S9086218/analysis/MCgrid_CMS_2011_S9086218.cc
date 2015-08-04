// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/Projections/FinalState.hh"
#include "Rivet/Projections/FastJets.hh"
#include "Rivet/Tools/BinnedHistogram.hh"

// MCgrid headers
#include "mcgrid/mcgrid.hh"
#include "mcgrid/mcgrid_binned.hh"

// How many of the rapidity bins should be translated to grids
#define N_HISTOS 6

namespace Rivet {

  // Inclusive jet pT
  class MCgrid_CMS_2011_S9086218 : public Analysis {
  public:

    // Constructor
    MCgrid_CMS_2011_S9086218() : Analysis("MCgrid_CMS_2011_S9086218") {}


    // Book histograms and initialize projections:
    void init() {

#if USE_APPL && USE_FNLO
      cerr << "MCgrid error: This analysis is configured to generate both an APPLgrid and a fastNLO table." << endl;
      cerr << "But using multiple binned histograms is not supported yet." << endl;
      exit(-1);
#endif

      const FinalState fs;

      // Initialize the projectors:
      addProjection(FastJets(fs, FastJets::ANTIKT, 0.5),"Jets");

      // Book histograms:
      _hist_sigma.addHistogram(0.0, 0.5, bookHisto1D(1, 1, 1));
      _hist_sigma.addHistogram(0.5, 1.0, bookHisto1D(2, 1, 1));
      _hist_sigma.addHistogram(1.0, 1.5, bookHisto1D(3, 1, 1));
      _hist_sigma.addHistogram(1.5, 2.0, bookHisto1D(4, 1, 1));
      _hist_sigma.addHistogram(2.0, 2.5, bookHisto1D(5, 1, 1));
      _hist_sigma.addHistogram(2.5, 3.0, bookHisto1D(6, 1, 1));
      
      const vector<Histo1DPtr> histos = _hist_sigma.getHistograms();
      string subprocFileName("basic");
      // string subprocFileName("MCgrid_CMS_2011_S9086218");
#if USE_APPL
      subprocFileName += ".config";
#elif USE_FNLO
      subprocFileName += ".str";
#endif
      MCgrid::subprocessConfig subproc(subprocFileName,
                                       MCgrid::BEAM_PROTON,
                                       MCgrid::BEAM_PROTON);


#if USE_APPL
      MCgrid::applGridConfig config(2, subproc, MCgrid::highPrecAPPLgridArch, 1E-5, 1, 10, 1E7);
#elif USE_FNLO
      MCgrid::fastnloGridArch arch(15, 6, "Lagrange", "Lagrange", "sqrtlog10", "loglog025");
      MCgrid::fastnloConfig config(2, subproc, arch, 7000.0);
#endif

      // Book grids, and push into BinnedGrid instance
      MCgrid::gridPtr grid;
      for (size_t i(0); i < N_HISTOS; i++) {
        grid = MCgrid::bookGrid(histos[i], histoDir(), config);
        _grid_sigma.addGrid(i * 0.5, (i + 1) * 0.5, grid);
      }

    }

    // Analysis
    void analyze(const Event &event) {
      
      // MCgrid event counter
      MCgrid::PDFHandler::HandleEvent(event, histoDir());
      
      const double weight = event.weight();
      const FastJets &fj = applyProjection<FastJets>(event,"Jets");
      const Jets& jets = fj.jets(Cuts::ptIn(18*GeV, 1100.0*GeV) && Cuts::absrap < 4.7);

      // Fill the relevant histograms:
      foreach(const Jet &j, jets) {
        _hist_sigma.fill(j.absrap(), j.pT(), weight);
        if (j.absrap() <= N_HISTOS * 0.5) {
          _grid_sigma.fill(j.absrap(), j.pT(), event);
        }
      }
    }

    // Finalize
    void finalize() {
      _hist_sigma.scale(crossSection()/sumOfWeights()/2.0, this);
      _grid_sigma.scale(crossSection()/sumOfWeights()/2.0);
      
      _grid_sigma.exportgrids();
      
      // Clear MCgrid event counter
      MCgrid::PDFHandler::CheckOutAnalysis(histoDir());
    }

  private:
    BinnedHistogram<double> _hist_sigma;
    MCgrid::BinnedGrid<double> _grid_sigma;
  };

  // This global object acts as a hook for the plugin system.
  DECLARE_RIVET_PLUGIN(MCgrid_CMS_2011_S9086218);

}
