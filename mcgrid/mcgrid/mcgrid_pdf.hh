//
//  mcgrid_pdfs.hh
//  MCgrid 19/11/2013.
//

#ifndef __appl_plugin__mcgrid_pdfs__
#define __appl_plugin__mcgrid_pdfs__

#include <iostream>
#include <stdint.h>
#include <set>

// Rivet includes
#include "Rivet/Rivet.hh"
#include "Rivet/Event.hh"

// APPLgrid includes
#include "appl_grid/lumi_pdf.h"

namespace MCgrid
{
  
  // Beam types (just proton/antiproton for the moment)
  typedef enum beamType {BEAM_PROTON = 1, BEAM_ANTIPROTON = -1} beamType;
  
  /**
   * MCgrid::mcgrid_pdf provides a wrapper for an APPLgrid lumi_pdf object,
   * handles the counting of subevents per subprocess to ensure correct
   * treatment of subprocess statistics. Exports an eventcount after the phase
   * space run.
   **/
  class mcgrid_pdf: public lumi_pdf
  {
  public:
    mcgrid_pdf(std::string const& name, std::string const& analysis, beamType beam1, beamType beam2);
    ~mcgrid_pdf();
    
    bool isInitialised() const {return initialised;};
    
    void CountEvent(const int fl1, const int fl2);
    double EventRatio(const int fl1, const int fl2);
    
    // Overloaded to flip flavours in fill
    int  decideSubProcess(const int iflav1, const int iflav2) const;
    int  decideSubPair(const int sub, const int iflav1, const int iflav2) const;
    
  private:
    void Export() const;         //!< Write event data to file
    bool Read();                 //!< Read event data from exported file
    
    const std::string pdfname;   //!< Name of Subproc PDF
    const std::string analysis;  //!< Name of Analysis
    bool initialised;            //!< Phase space run flag
    
    const beamType beam1;        //!< Type of beam 1 (p/ppbar)
    const beamType beam2;        //!< Type of beam 2 (p/ppbar)
    
    int *nPairs;                 //!< Number of partonic channels per subproccess
    
    uint64_t *nSubEvents;        //!< Number of events per subprocess
    uint64_t **nSubPairEvents;   //!< Number of events per partonic channel
  };

  // *********************** PDFHandler *******************************

  /**
   * MCgrid::PDFHandler provides a global instance to keep track of initialised
   * mcgrid_pdf instances. Interface for passing events to the mcgrid_pdfs to count
   * subprocess sub-events. Also provides an interface for booking mcgrid_pdfs.
   **/
  class PDFHandler
  {
  public:
    // Book a new mcgrid_pdf based upon a config file and beamtypes
    static void BookPDF(std::string const& name, std::string const& analysis, beamType beam1, beamType beam2);
    // Passes an event to mapped subprocess PDFs for counting.
    static void HandleEvent(Rivet::Event const&);

    // Removes analysis from analyses and calls ClearHandler if no analysis is
    // left. This has to be used instead of ClearHandler in the finalize()
    // method of an analysis if more than one analysis is being used
    static void CheckOutAnalysis(std::string const& analysis)
    {
      GetHandler(analysis)->analyses.erase(analysis);
      if (GetHandler(analysis)->analyses.empty())
        ClearHandler();
    }
    
    static void ClearHandler()
    {
      delete handlerInstance;
      handlerInstance = 0;
    }
    
  private:
    PDFHandler();
    ~PDFHandler();
    
    static PDFHandler* GetHandler()
    {
      if (handlerInstance == 0)
        handlerInstance = new PDFHandler();
      
      return handlerInstance;
    }
    
    // singleton
    static PDFHandler* handlerInstance;
    
    // Map of subprocess PDFs
    std::map<int, mcgrid_pdf*> pdfMap;

    // Set of analyses used to keep track of the active analyses
    std::set<std::string> analyses;

    // (Arbitrary) analysis to be used to count events. We get a HandleEvent
    // call from each analysis used in the run and don't want to double count.
    // This might be an empty string if the older single-analysis API is used
    const std::string eventCounterAnalysis;

  };


}
#endif /* defined(__appl_plugin__mcgrid_pdfs__) */
