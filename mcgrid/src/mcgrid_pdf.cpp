//
//  mcgrid_pdfs.cpp
//  MCgrid 19/11/2013.
//

#include "mcgrid/mcgrid_pdf.hh"
#include "mcgrid/mcgrid.hh"

// Rivet includes
#include "Rivet/Rivet.hh"
#include "Rivet/Event.hh"

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
  
// **********************  PDF class  **************************

  mcgrid_pdf::mcgrid_pdf(std::string const& name, beamType _beam1, beamType _beam2):
  lumi_pdf(name),
  pdfname(name.substr(0,name.find("."))),
  initialised(false),
  beam1(_beam1),
  beam2(_beam2),
  nPairs(new int[Nproc()]),
  nSubEvents(new uint64_t[Nproc()]),
  nSubPairEvents(new uint64_t*[Nproc()])
  {
    for (int i=0; i<Nproc(); i++)
    {
      nPairs[i] = (*this)[i].size();
      
      nSubEvents[i] = 0;
      nSubPairEvents[i] = new uint64_t[nPairs[i]];
      
      for (int j=0; j<nPairs[i]; j++)
        nSubPairEvents[i][j] = 0;
    }
    
    // Attempt to read data
    initialised = Read();
  }
  
  mcgrid_pdf::~mcgrid_pdf()
  {
    if (!initialised)
      Export();
    
    for (int i=0; i<Nproc(); i++)
      delete[] nSubPairEvents[i];
    
    delete[] nSubPairEvents;
    delete[] nSubEvents;
    delete[] nPairs;
  }
  
  // Count event flavours to ensure correct statistics in the combination
  void mcgrid_pdf::CountEvent(const int fl1, const int fl2)
  {
    const int subproc = decideSubProcess(fl1,fl2);
    const int subpair = decideSubPair(subproc, fl1, fl2);
    
    nSubEvents[subproc]++;
    nSubPairEvents[subproc][subpair]++;
  }
  
  // Returns multiplicative factor to convert Nt/Ni to Nt/Nsub
  double mcgrid_pdf::EventRatio(const int fl1, const int fl2)
  {
    if (!initialised)
      return 1;
    
    const int subproc = decideSubProcess(fl1,fl2);
    const int subpair = decideSubPair(subproc,fl1,fl2);
   
    // May be better to have a lookup table?
    return ((double)nSubPairEvents[subproc][subpair])/((double)nSubEvents[subproc]);
  }
  
  // Export evtcount file
  void mcgrid_pdf::Export() const
  {
    // Create the directory if it doesnt already exist
    Rivet::stringstream filename;
    filename << "mcgrid";
    createPath(filename.str());
    
    // Write event counter
    filename <<"/"<<pdfname<<".evtcount";
    std::ofstream file;
    file.open(filename.str().c_str());
    
    file <<pdfname<<endl;
    file <<Nproc()<<endl;
    
    for (int i=0; i<Nproc(); i++)
      file << "SubProc: "<< i << " Pairs: "<<nPairs[i]<<" SubEvents: "<<nSubEvents[i]<<endl;
    
    for (int i=0; i<Nproc(); i++)
    {
      file << "Subproc: "<<i<<"  ";
      for (int j=0; j<nPairs[i]; j++)
        file <<nSubPairEvents[i][j]<<"  ";
      file <<endl;
    }
  }
  
  // Read evtcount file
  bool mcgrid_pdf::Read()
  {
    Rivet::stringstream filename;
    filename << "mcgrid/" << pdfname<<".evtcount";
    if (Rivet::fileexists(filename.str()))
    {
      // Read phase space file
      std::ifstream datastream;
      datastream.open(filename.str().c_str());
      
      int testint;
      std::string teststr;
      
      datastream >> teststr;
      datastream >> testint;
      
      if (testint != Nproc())
      {
        cerr << "MCGrid::mcgrid_pdf Error: Event counter information in "<< filename<<" is inconsistent with PDF config file"<<endl;
        cerr << "                          Please rerun the phase space optimisation."<<endl;
        exit(-1);
      }
      
      for (int i=0; i<Nproc(); i++)
      {
        datastream >> teststr >> testint >> teststr >> nPairs[i] >> teststr >> nSubEvents[i];
        if (testint != i )
        {
          cerr << "MCGridmcgrid_pdf Error: Event counter information in "<< filename<<" is incorrectly formatted"<<endl;
          cerr << "                        Please rerun the phase space optimisation."<<endl;
          exit(-1);
        }
      }
      
      for (int i=0; i<Nproc(); i++)
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
  
  int mcgrid_pdf::decideSubProcess(const int iflav1, const int iflav2)
  {
    // Switch flavours if using antiproton beams
    const int subproc = lumi_pdf::decideSubProcess(beam1*iflav1, beam2*iflav2);
    
    if (subproc == -1)
    {
      cerr << "MCgrid: Error - event cannot be classified into a subprocess!"<<endl;
      cerr << "MCgrid: Flavours: "<<beam1*iflav1<<"  "<<beam2*iflav2<<endl;
      exit(-1);
    }
    
    return subproc;
  }
  
  // Find the subprocess pair corresponding to the provided flavours
  int mcgrid_pdf::decideSubPair(const int sub, const int fl1, const int fl2) const
  {
    for (int i=0; i<nPairs[sub]; i++)
      if (beam1*fl1 == (*this)[sub][i].first && beam2*fl2 == (*this)[sub][i].second)
        return i;

    cerr << "MCgrid: Error - event cannot be classified into a subprocess!"<<endl;
    cerr << "MCgrid: Flavours: "<<beam1*fl1<<"  "<<beam2*fl2<<endl;
    exit(-1);
    
    return -1;
  }

// **********************  PDFHandler **************************

  PDFHandler::PDFHandler(std::string const& eventCounterAnalysis):
  pdfMap(),
  eventCounterAnalysis(eventCounterAnalysis)
  {
  }

  PDFHandler::~PDFHandler()
  {
    for (std::map<int,mcgrid_pdf*>::iterator iCount = pdfMap.begin(); iCount != pdfMap.end(); iCount++)
      delete (*iCount).second;
  }

  void PDFHandler::BookPDF(std::string const& name, std::string const& analysis, beamType beam1, beamType beam2)
  {
    // Add the analysis to our set of analyses
    GetHandler(analysis)->analyses.insert(analysis);

    const int hashval = hash_str(name.c_str());
    
    std::map<int, mcgrid_pdf*>::iterator iterator = GetHandler(analysis)->pdfMap.find(hashval);
    if (iterator != GetHandler(analysis)->pdfMap.end())
    {
      // It's okay to book a pdf twice, because the user might be using
      // multiple analyses and he shouldn't be forced to remove the calls from
      // subsequent analyses.
      // It's not okay though if the beam types do not match, so let's check
      if (iterator->second->beam1 != beam1 || iterator->second->beam2 != beam2) {
        cout << "MCgrid::PDFHandler::BookPDF Warning - Subprocess PDF ";
        cout << name << " is already booked and the";
        cout << "beam types do not match!" << endl;
        exit(-1);
      } else {
        return;
      }
    }
    
    GetHandler(analysis)->pdfMap.insert(std::make_pair(hashval, new mcgrid_pdf(name, beam1, beam2) ));
    cout << "MCgrid::PDFHandler::BookPDF Added subprocess PDF " << name;
    cout << ", hash: " << hashval << endl;
    
    return;
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
    for (std::map<int,mcgrid_pdf*>::iterator iCount = GetHandler()->pdfMap.begin(); iCount != GetHandler()->pdfMap.end(); iCount++)
      if (!(*iCount).second->isInitialised())
        (*iCount).second->CountEvent(pdgToLHA(event.genEvent()->pdf_info()->id1()),pdgToLHA(event.genEvent()->pdf_info()->id2()));
  }

}
