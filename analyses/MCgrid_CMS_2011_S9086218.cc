// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/Projections/FinalState.hh"
#include "Rivet/Projections/FastJets.hh"
#include "Rivet/Tools/BinnedHistogram.hh"

// MCgrid headers
#include "mcgrid/mcgrid.hh"
#include "mcgrid/mcgrid_binned.hh"

namespace Rivet {

  // Inclusive jet pT
  class MCgrid_CMS_2011_S9086218 : public Analysis {
  public:

    // Constructor
    MCgrid_CMS_2011_S9086218() : Analysis("MCgrid_CMS_2011_S9086218") {}


    // Book histograms and initialize projections:
    void init() {
      const FinalState fs;
      
      // Initialise the subprocess PDF and grid architecture
      const string PDFname("MCgrid_CMS_2011_S9086218.config");
      MCgrid::bookPDF(PDFname,
                      histoDir(),
                      MCgrid::BEAM_PROTON,
                      MCgrid::BEAM_PROTON
                      );
      MCgrid::gridArch arch(30,20,5,5);

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
      
      // Book grids, and push into BinnedGrid instance
      MCgrid::gridPtr agrid = MCgrid::bookGrid(histos[0], histoDir(), PDFname, 2, 1E-5, 1, 10, 1E6, arch);
      _appl_sigma.addGrid(0.0, 0.5, agrid);
      
      agrid = MCgrid::bookGrid(histos[1], histoDir(), PDFname, 2, 1E-5, 1, 10, 1E6, arch);
      _appl_sigma.addGrid(0.5, 1.0, agrid);
      
      agrid = MCgrid::bookGrid(histos[2], histoDir(), PDFname, 2, 1E-5, 1, 10, 1E6, arch);
      _appl_sigma.addGrid(1.0, 1.5, agrid);
      
      agrid = MCgrid::bookGrid(histos[3], histoDir(), PDFname, 2, 1E-5, 1, 10, 1E6, arch);
      _appl_sigma.addGrid(1.5, 2.0, agrid);
      
      agrid = MCgrid::bookGrid(histos[4], histoDir(), PDFname, 2, 1E-5, 1, 10, 1E6, arch);
      _appl_sigma.addGrid(2.0, 2.5, agrid);
      
      agrid = MCgrid::bookGrid(histos[5], histoDir(), PDFname, 2, 1E-5, 1, 10, 1E6, arch);
      _appl_sigma.addGrid(2.5, 3.0, agrid);
    }

    // Analysis
    void analyze(const Event &event) {
      
      // MCgrid event counter
      MCgrid::PDFHandler::HandleEvent(event);
      
      
      const double weight = event.weight();
      const FastJets &fj = applyProjection<FastJets>(event,"Jets");
      const Jets& jets = fj.jets(18.0*GeV, 1100.0*GeV, -4.7, 4.7, RAPIDITY);

      // Fill the relevant histograms:
      foreach(const Jet &j, jets) {
        _hist_sigma.fill(j.absrap(), j.pT(), weight);
        _appl_sigma.fill(j.absrap(), j.pT(), event);
      }
    }

    // Finalize
    void finalize() {
      _hist_sigma.scale(crossSection()/sumOfWeights()/2.0, this);
      _appl_sigma.scale(crossSection()/sumOfWeights()/2.0);
      
      _appl_sigma.exportgrids();
      
      // Clear MCgrid event counter
      MCgrid::PDFHandler::ClearHandler();
    }

  private:
    BinnedHistogram<double> _hist_sigma;
    MCgrid::BinnedGrid<double> _appl_sigma;
  };

  // This global object acts as a hook for the plugin system.
  DECLARE_RIVET_PLUGIN(MCgrid_CMS_2011_S9086218);

}
