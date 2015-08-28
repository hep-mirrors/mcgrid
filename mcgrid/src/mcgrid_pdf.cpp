//
//  mcgrid_pdfs.cpp
//  MCgrid 19/11/2013.
//

// System
#include "config.h"

// Rivet includes
#include "Rivet/Rivet.hh"
#include "Rivet/Event.hh"

#include "mcgrid/mcgrid_pdf.hh"
#include "mcgrid/mcgrid.hh"
#include "mcgrid.hh"
#include "conventions.hh"
#include "system.hh"

// Interface-specific includes
#if APPLGRID_ENABLED
#include "appl_grid/lumi_pdf.h"
#endif
#if FASTNLO_ENABLED
#include "fastnlotk/fastNLOCreate.h"
#endif

using Rivet::cerr;
using Rivet::cout;
using Rivet::endl;

namespace MCgrid
{
  // Singleton management
  PDFHandler *PDFHandler::handlerInstance = 0;

  // basic hash
  static unsigned hash_str(const char* s)
  {
    unsigned h = 31;
    while (*s) {
      h = (h * 54059) ^ (s[0] * 76963);
      s++;
    }
    return h % 86969;
  }

  
// **********************  PDF classes  **************************

  mcgrid_base_pdf::~mcgrid_base_pdf()
  {
    if (!initialised)
      Export();

    for (int i=0; i<NumberOfSubprocesses(); i++)
      delete[] nSubPairEvents[i];

    delete[] nSubPairEvents;
    delete[] nSubEvents;
    delete[] nPairs;
  }


  template<typename T>
  void mcgrid_base_pdf::InitialiseEventCounting(T *subprocesses)
  {
    nPairs = new int[NumberOfSubprocesses()];
    nSubEvents = new uint64_t[NumberOfSubprocesses()];
    nSubPairEvents = new uint64_t*[NumberOfSubprocesses()];

    for (int i=0; i<NumberOfSubprocesses(); i++)
    {
      nPairs[i] = (*subprocesses)[i].size();
      
      nSubEvents[i] = 0;
      nSubPairEvents[i] = new uint64_t[nPairs[i]];
      
      for (int j=0; j<nPairs[i]; j++)
        nSubPairEvents[i][j] = 0;
    }

    // Attempt to read data
    initialised = Read();
  }


  // Export evtcount file
  void mcgrid_base_pdf::Export() const
  {
    // Create the directory if it doesnt already exist
    Rivet::stringstream filename;
    filename << MCgridPhasespacePath();
    createPath(filename.str());
    
    // Write event counter
    filename << "/" << pdfname << ".evtcount";
    std::ofstream file;
    file.open(filename.str().c_str());
    
    file << pdfname << endl;
    file << NumberOfSubprocesses() << endl;
    
    for (int i=0; i<NumberOfSubprocesses(); i++) {
      file << "SubProc: "<< i;
      file << " Pairs: " << nPairs[i];
      file << " SubEvents: " << nSubEvents[i] << endl;
    }
    
    for (int i=0; i<NumberOfSubprocesses(); i++) {
      file << "Subproc: " << i << "  ";
      for (int j=0; j<nPairs[i]; j++)
        file <<nSubPairEvents[i][j] << "  ";
      file << endl;
    }
  }


  // Read evtcount file
  bool mcgrid_base_pdf::Read()
  {
    Rivet::stringstream filename;
    filename << MCgridPhasespacePath() << "/" << pdfname << ".evtcount";
    if (Rivet::fileexists(filename.str()))
    {
      // Read phase space file
      std::ifstream datastream;
      datastream.open(filename.str().c_str());
      
      int testint;
      std::string teststr;
      
      datastream >> teststr;
      datastream >> testint;
      
      if (testint != NumberOfSubprocesses())
      {
        cerr << "MCGrid::mcgrid_pdf Error: Event counter information in "<< filename<<" is inconsistent with PDF config file"<<endl;
        cerr << "                          Please rerun the phase space optimisation."<<endl;
        exit(-1);
      }
      
      for (int i=0; i<NumberOfSubprocesses(); i++)
      {
        datastream >> teststr >> testint >> teststr >> nPairs[i] >> teststr >> nSubEvents[i];
        if (testint != i )
        {
          cerr << "MCGridmcgrid_pdf Error: Event counter information in "<< filename<<" is incorrectly formatted"<<endl;
          cerr << "                        Please rerun the phase space optimisation."<<endl;
          exit(-1);
        }
      }
      
      for (int i=0; i<NumberOfSubprocesses(); i++)
      {
        datastream >> teststr >> testint;
        for (int j=0; j<nPairs[i]; j++)
          datastream >> nSubPairEvents[i][j];
        
        if (testint != i )
        {
          cerr << "MCGridmcgrid_pdf Error: Event counter information in "<< filename<<" is incorrectly formatted"<<endl;
          cerr << "                        Please rerun the phase space optimisation."<<endl;
          exit(-1);
        }
        
      }
      return true;
    }
    
    return false;
  }
  
  // Count event flavours to ensure correct statistics in the combination
  void mcgrid_base_pdf::CountEvent(const int fl1, const int fl2)
  {
    const int subproc = decideSubProcess(fl1,fl2);
    const int subpair = decideSubPair(subproc, fl1, fl2);
    
    nSubEvents[subproc]++;
    nSubPairEvents[subproc][subpair]++;
  }
  
  // Returns multiplicative factor to convert Nt/Ni to Nt/Nsub
  double mcgrid_base_pdf::EventRatio(const int fl1, const int fl2)
  {
    if (!initialised)
      return 1;
    
    const int subproc = decideSubProcess(fl1,fl2);
    const int subpair = decideSubPair(subproc,fl1,fl2);
   
    // May be better to have a lookup table?
    return ((double)nSubPairEvents[subproc][subpair])/((double)nSubEvents[subproc]);
  }

  int mcgrid_base_pdf::validateSubProcessResult(const int subproc, const int iflav1, const int iflav2) const
  {    
    if (subproc == -1)
    {
      cerr << "MCgrid: Error - event cannot be classified into a subprocess!"<<endl;
      cerr << "MCgrid: Flavours: "<<iflav1<<"  "<<iflav2<<endl;
      exit(-1);
    }
    
    return subproc;
  }

  template<typename T>
  int mcgrid_base_pdf::decideSubPair(T *subprocesses, const int sub, const int fl1, const int fl2) const
  {
    for (int i=0; i<nPairs[sub]; i++)
      if (fl1 == (*subprocesses)[sub][i].first && fl2 == (*subprocesses)[sub][i].second)
        return i;

    cerr << "MCgrid: Error - event cannot be classified into a subprocess!"<<endl;
    cerr << "MCgrid: Flavours: "<<fl1<<"  "<<fl2<<endl;
    exit(-1);
    
    return -1;
  }

  /**
   * MCgrid::mcgrid_appl_pdf embeds the abstract mcgrid_base_pdf class into the
   * APPLgrid context by inheriting appl::lumi_pdf, which implements all that
   * is left virtual in mcgrid_base_pdf
   **/
#if APPLGRID_ENABLED
  class mcgrid_appl_pdf: public lumi_pdf, public mcgrid_base_pdf
  {
  public:
    mcgrid_appl_pdf(mcgrid_appl_pdf_params const& params);
    int decideSubProcess(const int iflav1, const int iflav2) const;
    int decideSubPair(const int sub, const int iflav1, const int iflav2) const;
    int NumberOfSubprocesses() const { return Nproc(); }
    const std::string name();
    const beamType beam1;
    const beamType beam2;
  };

  mcgrid_appl_pdf::mcgrid_appl_pdf(mcgrid_appl_pdf_params const& params):
    lumi_pdf(params.name),
    mcgrid_base_pdf(params, Nproc()),
    beam1(params.beam1),
    beam2(params.beam2)
  {
    mcgrid_base_pdf::InitialiseEventCounting(this);
  }
  
  int mcgrid_appl_pdf::decideSubProcess(const int iflav1, const int iflav2) const
  {
    // Switch flavours if using antiproton beams
    const int subproc = lumi_pdf::decideSubProcess(beam1*iflav1, beam2*iflav2);
    return mcgrid_base_pdf::validateSubProcessResult(subproc, beam1*iflav1, beam2*iflav2);
  }
  
  // Find the subprocess pair corresponding to the provided flavours
  int mcgrid_appl_pdf::decideSubPair(const int sub, const int fl1, const int fl2) const
  {
    return mcgrid_base_pdf::decideSubPair(this, sub, beam1*fl1, beam2*fl2);
  }

  const std::string mcgrid_appl_pdf::name()
  {
    return lumi_pdf::name();
  }
#endif

  /**
   * MCgrid::mcgrid_fastnlo_pdf embeds the abstract mcgrid_base_pdf class into
   * the fastNLO context by using the fastNLOCreate instance associated with
   * it, which implements all that is left virtual in mcgrid_base_pdf
   **/
#if FASTNLO_ENABLED
  class mcgrid_fastnlo_pdf: public mcgrid_base_pdf
  {
  public:
    mcgrid_fastnlo_pdf(mcgrid_fnlo_pdf_params const&);
    const std::vector<std::pair<int, int> >& operator[](int i) const;
    int decideSubProcess(const int iflav1, const int iflav2) const;
    int decideSubPair(const int sub, const int iflav1, const int iflav2) const;
    int NumberOfSubprocesses() const { return ftable->GetNSubprocesses(); }

  private:
    void InitialiseReverseLookupTable();

    fastNLOCreate * const ftable;

    // The lumi_pdf class of APPLgrid has this (used in decideSubProcess), but
    // it is missing in fastNLO, so we create it here ourselves
    std::vector<std::vector<int> > subprocessLookupTable;
  };

  const std::vector<std::pair<int, int> >& mcgrid_fastnlo_pdf::operator[](int i) const {
    return ftable->GetTheCoeffTable()->GetPDFCoeff()[i];
  }

  mcgrid_fastnlo_pdf::mcgrid_fastnlo_pdf(mcgrid_fnlo_pdf_params const& params):
    mcgrid_base_pdf(params, params.ftable->GetNSubprocesses()),
    ftable(params.ftable)
  {
    mcgrid_base_pdf::InitialiseEventCounting(this);
    InitialiseReverseLookupTable();
  };

  void mcgrid_fastnlo_pdf::InitialiseReverseLookupTable()
  {
    subprocessLookupTable = std::vector<std::vector<int> >(13, std::vector<int>(13, -1) );
    for (unsigned int i = 0; i < NumberOfSubprocesses(); i++) {
      std::vector<std::pair<int, int> > pairs = (*this)[i];
      for (unsigned int j = 0; j < pairs.size(); j++) {
        subprocessLookupTable[pairs[j].first+6][pairs[j].second+6] = i;
      }
    }
  }

  int mcgrid_fastnlo_pdf::decideSubProcess(const int iflav1, const int iflav2) const
  {
    // Switch flavours if using antiproton beams
    const int subproc = subprocessLookupTable[iflav1+6][iflav2+6];
    return mcgrid_base_pdf::validateSubProcessResult(subproc, iflav1, iflav2);
  }
  
  // Find the subprocess pair corresponding to the provided flavours
  int mcgrid_fastnlo_pdf::decideSubPair(const int sub, const int fl1, const int fl2) const
  {
    return mcgrid_base_pdf::decideSubPair(this, sub, fl1, fl2);
  }

#endif

// **********************  PDFHandler **************************

  PDFHandler::~PDFHandler()
  {
    for (std::map<int,mcgrid_base_pdf*>::iterator iCount = pdfMap.begin(); iCount != pdfMap.end(); iCount++) {
#if APPLGRID_ENABLED
      if (mcgrid_appl_pdf *pdf = dynamic_cast<mcgrid_appl_pdf*>((*iCount).second)) {
        delete pdf;
        continue;
      }
#endif
#if FASTNLO_ENABLED
      if (mcgrid_fastnlo_pdf *pdf = dynamic_cast<mcgrid_fastnlo_pdf*>((*iCount).second)) {
        delete pdf;
        continue;
      }
#endif
    }
  }

  mcgrid_base_pdf* PDFHandler::BookPDF(mcgrid_base_pdf_params const& params, std::string const& analysis)
  {
    // Add the analysis to our set of analyses
    GetHandler(analysis)->analyses.insert(analysis);

    const int hashval = hash_str(params.name.c_str());
    
    std::map<int, mcgrid_base_pdf*>::iterator iterator = GetHandler(analysis)->pdfMap.find(hashval);
    if (iterator != GetHandler(analysis)->pdfMap.end())
    {
      // It's okay to book a pdf twice, because the user might use the same pdf
      // (that might be written down in a fnlo steering file) for multiple histos
      // For an appl pdf, we at least check first that the beam types match ...
#if APPLGRID_ENABLED
      const mcgrid_appl_pdf *appl_pdf = dynamic_cast<const mcgrid_appl_pdf*>(iterator->second);
      if (appl_pdf) {
        const mcgrid_appl_pdf_params& appl_params =
        dynamic_cast<const mcgrid_appl_pdf_params&>(params);
        if (appl_pdf->beam1 != appl_params.beam1 || appl_pdf->beam2 != appl_params.beam2) {
          cout << "MCgrid::PDFHandler::BookPDF Error - Subprocess PDF ";
          cout << appl_params.name << " is already booked and the";
          cout << "beam types do not match!" << endl;
          exit(-1);
        }
      }
#endif
      return iterator->second;
    }

    // We can now safely insert the new pdf
    mcgrid_base_pdf *pdf = NULL;
#if APPLGRID_ENABLED
    const mcgrid_appl_pdf_params *appl_params =
    dynamic_cast<const mcgrid_appl_pdf_params*>(&params);
    if (appl_params) {
      mcgrid_appl_pdf *appl_pdf = new mcgrid_appl_pdf(*appl_params);
      pdf = appl_pdf;
      cout << "MCgrid::PDFHandler::BookPDF Adding subprocess for use with APPLgrid ..." << endl;
    }
#endif
#if FASTNLO_ENABLED
    if (pdf == NULL) {
      const mcgrid_fnlo_pdf_params *fnlo_params =
      dynamic_cast<const mcgrid_fnlo_pdf_params*>(&params);
      if (fnlo_params) {
        pdf = new mcgrid_fastnlo_pdf(*fnlo_params);
      } else {
        cout << "MCgrid::Error - Unknown pdf parameter type";
        exit(-1);
      }
      cout << "MCgrid::PDFHandler::BookPDF Adding subprocess for use with fastNLO ..." << endl;
    }
#endif
    GetHandler(analysis)->pdfMap.insert(std::make_pair(hashval, pdf));
    cout << "MCgrid::PDFHandler::BookPDF Added subprocess PDF " << params.name;
    cout << ", hash: " << hashval << endl;
    return pdf;
  }

  void PDFHandler::HandleEvent(Rivet::Event const& event, std::string const& analysis)
  {
    // Check if we are using the right analysis
    if (analysis != GetHandler(analysis)->eventCounterAnalysis)
      return;

    HandleEvent(event);
  }

  void PDFHandler::HandleEvent(Rivet::Event const& event)
  {
    GetHandler()->nEvents++;
    for (std::map<int,mcgrid_base_pdf*>::iterator iCount = GetHandler()->pdfMap.begin(); iCount != GetHandler()->pdfMap.end(); iCount++)
      if (!(*iCount).second->isInitialised())
        (*iCount).second->CountEvent(pdgToLHA(event.genEvent()->pdf_info()->id1()),pdgToLHA(event.genEvent()->pdf_info()->id2()));
  }

}
