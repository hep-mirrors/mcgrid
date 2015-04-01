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

      // Book APPLgrids
      MCgrid::gridArch arch(50,1,5,0);
      const string PDFname = "MCgrid_D0_2007_S7075677.config";
      
      MCgrid::bookPDF(PDFname, histoDir(), MCgrid::BEAM_PROTON, MCgrid::BEAM_ANTIPROTON);
      _a_yZ = MCgrid::bookGrid(_h_yZ, histoDir(), PDFname, 0, 1E-5, 1, 8315.18, 8315.18, arch);
      _a_xs = MCgrid::bookGrid(_h_xs, histoDir(), PDFname, 0, 1E-5, 1, 8315.18, 8315.18, arch);

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
        }
      } else {
        MSG_DEBUG("No unique lepton pair found.");
      }
    }


    // Finalize
    void finalize() {
      // Data seems to have been normalized for the avg of the two sides
      // (+ve & -ve rapidity) rather than the sum, hence the 0.5:
      normalize(_h_yZ, 0.5);

      _a_yZ->scale(0.5*crossSection()/sumOfWeights());
      _a_xs->scale(crossSection()/sumOfWeights());

            // Export APPLgrids
      _a_yZ->exportgrid();
      _a_xs->exportgrid();
      
      // Clear event handler
      MCgrid::PDFHandler::CheckOutAnalysis(histoDir());
    }

    //@}


  private:

    /// @name Histograms
    //@{
    Histo1DPtr _h_yZ;
    Histo1DPtr _h_xs;

    // APPLgrids
    MCgrid::gridPtr _a_yZ;
    MCgrid::gridPtr _a_xs;
    //@}

  };



  // The hook for the plugin system
  DECLARE_RIVET_PLUGIN(MCgrid_D0_2007_S7075677);

}
