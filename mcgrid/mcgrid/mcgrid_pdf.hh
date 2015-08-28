//
//  mcgrid_appl_pdfs.hh
//  MCgrid 19/11/2013.
//

#ifndef __appl_plugin__mcgrid_appl_pdfs__
#define __appl_plugin__mcgrid_appl_pdfs__

#include <iostream>
#include <stdint.h>
#include <set>

#include "Rivet/Rivet.hh"

// Forward definitions
class fastNLOCreate;

// Output stream operator overload for pairs
template <class T1, class T2>
std::ostream& operator<<(std::ostream& out, const std::pair<T1, T2>& value)
{
    out << '(' << value.first << ", " << value.second << ')';
    return out;
}

namespace MCgrid
{
  // Beam types (just proton/antiproton for the moment)
  typedef enum beamType {BEAM_PROTON = 1, BEAM_ANTIPROTON = -1} beamType;
  static inline int beamTypeToPDG( beamType type ) {
    if (type == BEAM_PROTON) {
      return 2212;
    } else {
      return -2212;
    }
  }

  /**
   * MCgrid::mcgrid_base_pdf_params is used to construct general new subprocess
   * definitions
   **/
  struct mcgrid_base_pdf_params
  {
    mcgrid_base_pdf_params(std::string const& _name):
      name(_name) {};
    // A virtual destructor to mark this struct as polymorphic
    virtual ~mcgrid_base_pdf_params() {};
    const std::string name;
  };

  

  /**
   * MCgrid::mcgrid_appl_pdf_params is used to construct new subprocess
   * definitions for APPLgrid, which has a a txt file with all subprocesses,
   * but does not know about the sign of the incoming protons (such that we
   * have to keep track of this and to perform sign flips when necessary).
   **/
  struct mcgrid_appl_pdf_params : mcgrid_base_pdf_params
  {
    mcgrid_appl_pdf_params(std::string const& _name,
                           const beamType _beam1,
                           const beamType _beam2):
      mcgrid_base_pdf_params(_name),
      beam1                 (_beam1),
      beam2                 (_beam2) {};

    const beamType beam1;
    const beamType beam2;
  };


  /**
   * MCgrid::mcgrid_fnlo_pdf_params is used to construct new subprocess
   * definitions for fastNLO, which loads the subprocesses from a grid steering
   * file, such that we need a pointer to the fastNLO class that reads that.
   **/
  struct mcgrid_fnlo_pdf_params : mcgrid_base_pdf_params
  {
    mcgrid_fnlo_pdf_params(std::string const& _name, fastNLOCreate * const _ftable):
      mcgrid_base_pdf_params(_name),
      ftable                (_ftable) {};

    fastNLOCreate * const ftable;
  };

  /**
   * MCgrid::mcgrid_base_pdf handles the counting of subevents per subprocess
   * to ensure correct treatment of subprocess statistics.
   * It exports an event count after the phase space run.
   **/
  class mcgrid_base_pdf {
  public:
    ~mcgrid_base_pdf();

    // Subprocess statistics
    void CountEvent(const int fl1, const int fl2);
    double EventRatio(const int fl1, const int fl2);

    // Subprocess information
    virtual int NumberOfSubprocesses() const { return nSubprocesses; };
    virtual int decideSubPair(const int sub, const int iflav1, const int iflav2) const = 0;
    virtual int decideSubProcess(const int iflav1, const int iflav2) const = 0;

    // State
    bool isInitialised() const { return initialised; };
    virtual const std::string name() const { return pdfname; };

  protected:
    mcgrid_base_pdf(mcgrid_base_pdf_params const& params,
                    const int _nSubprocesses):
      nSubprocesses(_nSubprocesses),
      initialised(false),
      pdfname(params.name)
    {};

    // Subprocess statistics
    template<typename T>
    void InitialiseEventCounting(T *subprocesses);

    // Subprocess information
    int validateSubProcessResult(const int sub, const int iflav1, const int iflav2) const;
    template<typename T>
    int decideSubPair(T *subprocesses, const int subproc, const int iflav1, const int iflav2) const;
    int nSubprocesses;           //!< Number of subprocesses
    int *nPairs;                 //!< Number of partonic channels per subproccess

    // State
    bool initialised;            //!< Phase space run flag

  private:
    // Subprocess statistics
    void Export() const;         //!< Write event data to file
    bool Read();                 //!< Read event data from exported file
    uint64_t *nSubEvents;        //!< Number of events per subprocess
    uint64_t **nSubPairEvents;   //!< Number of events per partonic channel

    // State
    const std::string pdfname;   //!< Name of Subproc PDF
  };

  // *********************** PDFHandler *******************************

  /**
   * MCgrid::PDFHandler provides a global instance to keep track of initialised
   * mcgrid_base_pdf instances. Interface for passing events to the
   * mcgrid_base_pdf instances to count subprocess sub-events. Also provides an
   * interface for booking mcgrid_base_pdfs.
   **/
  class PDFHandler
  {
  public:

    static mcgrid_base_pdf* BookPDF(mcgrid_base_pdf_params const& params,
                                    std::string const& analysis);

    // Passes an event to mapped subprocess PDFs for counting.
    static void HandleEvent(Rivet::Event const&, std::string const& analysis);

    // Removes analysis from analyses and calls ClearHandler if no analysis is
    // left. This has to be used instead of ClearHandler in the finalize()
    // method of an analysis if more than one analysis is being used
    static void CheckOutAnalysis(std::string const& analysis)
    {
      GetHandler(analysis)->analyses.erase(analysis);
      if (GetHandler(analysis)->analyses.empty())
        ClearHandler();
    }

    static uint64_t NEvents() { return (handlerInstance == 0) ? 0 : handlerInstance->nEvents; };
    
  private:

    PDFHandler(std::string const& eventCounterAnalysis):
      nEvents(0),
      eventCounterAnalysis(eventCounterAnalysis)
    {};

    ~PDFHandler();

    static void HandleEvent(Rivet::Event const& event);
    
    static PDFHandler* GetHandler(std::string const& eventCounterAnalysis)
    {
      if (handlerInstance == 0)
        handlerInstance = new PDFHandler(eventCounterAnalysis);
      
      return handlerInstance;
    }
    static PDFHandler* GetHandler()
    {
      return GetHandler("");
    }

    static void ClearHandler()
    {
      delete handlerInstance;
      handlerInstance = 0;
    }
    
    static PDFHandler* handlerInstance;      //!< Singleton

    std::map<int, mcgrid_base_pdf*> pdfMap;  //!< Map of subprocess PDFs
    uint64_t nEvents;                        //!< Total event counter of current run
    std::set<std::string> analyses;          //!< Set of analyses used to keep track
                                             //!< of the active analyses

    /*!
      (Arbitrary) analysis to be used to count events. We get a HandleEvent
      call from each analysis used in the run and don't want to double count.
      This might be an empty string if the older single-analysis API is used
    */
    const std::string eventCounterAnalysis;

  };
}

#endif /* defined(__appl_plugin__mcgrid_appl_pdfs__) */
