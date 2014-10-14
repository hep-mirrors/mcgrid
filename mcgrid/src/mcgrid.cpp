//
//  mcgrid.cpp
//  MCgrid 03/10/2013.
//

// System
#include "mcgrid/config.h"

// MCGrid
#include "mcgrid/mcgrid.hh"
#include "mcgrid/mcgrid_pdf.hh"

// Rivet includes
#include "Rivet/Rivet.hh"
#include "Rivet/Event.hh"
#include "Rivet/Tools/RivetYODA.hh"

// APPLgrid includes
#include "appl_grid/appl_grid.h"

#include <string>
#include <vector>
#include <cmath>
#include <memory>

using Rivet::cerr;
using Rivet::cout;
using Rivet::endl;

namespace MCgrid
{
// ********************** MCgrid banner ***************************

static bool bannerShown = false;

static void showBanner()
{
  cout << endl;
  cout << "*****************************************************************" << endl;
  cout << "                   MCgrid plugin for Rivet                 " << endl;
  cout << "   E. Bothmann, L. Del Debbio, N.P. Hartland, S. Schumann          " << endl;
  cout << "              arXiv:1312.4460,   Version: " << PACKAGE_VERSION << "                "<<endl;
  cout << "                                                               " << endl;
  cout << "            -- Configured for "<<fillString[globalFillMode]<<" fillmode --            " << endl;
#ifndef HEPMC2NAMED
  cout << "     -- Using HepMC Compatibility mode: no named weights --    " << endl;
#endif
  cout << "*****************************************************************" << endl;
  cout << endl;
  
  // Make mcgrid folder
  createPath("mcgrid");
  
  bannerShown = true;
  
  return;
}
  
// *********************** Booked grid index **************************
  
  static std::vector<std::string> bookedGrids;
  std::string getFilename(std::string const& id)
  {
    int ind=0;
    std::string filename = id;
    while ( std::find(bookedGrids.begin(), bookedGrids.end(), filename)!=bookedGrids.end() )
    {
      ind++;
      std::stringstream icvrt;
      icvrt << id << "_" <<ind;
      filename = icvrt.str();
    }
    
    bookedGrids.push_back(filename);
    return filename;
  }

// *********************** Booking Functions **************************

// Shortcut for PDF Booking
  void bookPDF(std::string const& name, std::string const& analysis, beamType beam1, beamType beam2)
{
  if (!bannerShown)
    showBanner();
  
  PDFHandler::BookPDF(name, analysis, beam1, beam2);
  return;
}

// Book a MCgrid::grid object, returning the shared pointer
gridPtr bookGrid(   const Rivet::Histo1DPtr hist,
                    const std::string histoDir,
                    const std::string pdfname,
                    const int    LO,
                    const double xmin,
                    const double xmax,
                    const double q2min,
                    const double q2max,
                    const gridArch arch
)
{
  if (!bannerShown)
    showBanner();
    
  return gridPtr(new grid( hist,pdfname, histoDir,
                          LO,
                          xmin,xmax,
                          q2min,q2max,
                          arch
                          )
                 );
}


// ********************** fillInfo container **************************

// Restricts the HepMC PDF scale to 8 decimal places
// Improves comparison with scale bin widths
double roundScale(double PDFscale)
{
  const double prec = 1E8;
  const double rounded = round(PDFscale*prec)/prec;
  
  return rounded;
}

fillInfo::fillInfo(Rivet::Event const& event, int const& leadingOrder):
// Full event weight
wgt(event.weight()),
usr_wgt(getHepMCWeights(event)),
// Flavours and subprocess classification
fl1(pdgToLHA(event.genEvent()->pdf_info()->id1())),
fl2(pdgToLHA(event.genEvent()->pdf_info()->id2())),
// PDF Kinematics
x1(event.genEvent()->pdf_info()->x1()),
x2(event.genEvent()->pdf_info()->x2()),
pdfQ2(roundScale(pow(event.genEvent()->pdf_info()->scalePDF(),2))),
// PDFs
fx1(event.genEvent()->pdf_info()->pdf1()/x1),
fx2(event.genEvent()->pdf_info()->pdf2()/x2),
// Alpha_s
aspowers(usr_wgt[4]),
alphas(event.genEvent()->alphaQCD()),
asfac(pow(alphas/(2*M_PI),aspowers)),
ptord(aspowers - leadingOrder)
{
  return;
}
  
  // Retreive hepMC event weights
#ifdef HEPMC2NAMED
  
  // Stuff for converting the wierdly formatted hepmc named usr_wgts container
static const int nMaxHepMC = 27;
static const std::string wgtNames[27] = {"0","1","2","3","4","5",
  "6","7","8","9","10","11",
  "12","13","14","15","16","17",
  "18","19","20","21","22","23",
  "24","25","26"};

std::vector<double> fillInfo::getHepMCWeights(Rivet::Event const& event)
{
  const int nHepMC = event.genEvent()->weights().size();
  
  if (nHepMC > nMaxHepMC)
  {
    cerr << "MCgrid::Error -  Unexpected number of weights in usr_weights container"<<endl;
    event.genEvent()->weights().print(cerr);
    exit(-1);
  }
  
  std::vector<double> wgts;
  for (int i=0; i<nHepMC; i++)
    wgts.push_back(event.genEvent()->weights()[wgtNames[i]]);
  
  return wgts;
}

#else
  std::vector<double> fillInfo::getHepMCWeights(Rivet::Event const& event)
{
  const int nHepMC = event.genEvent()->weights().size();

  std::vector<double> wgts;
  for (int i=0; i<nHepMC; i++)
    wgts.push_back(event.genEvent()->weights()[i]);
  
  return wgts;
}
#endif


// **********************  Utility Functions **************************

// Returns vector of lower bin edges for appl_grid constructor
static std::vector<double> getBinning( const Rivet::Histo1DPtr histo)
{
  std::vector<double> lowEdges;
  for (size_t i=0; i<histo.get()->numBins(); i++)
    lowEdges.push_back( histo.get()->bin(i).xMin() );
  
  // Root needs the highest bin also
  lowEdges.push_back(histo.get()->bin(lowEdges.size()-1).xMax());
  
  return lowEdges;
}

static std::string idFromPath( std::string const& fullpath)
{
  unsigned ls = fullpath.find_last_of("/\\");
  return fullpath.substr(ls+1);
}

// ************************* grid class *********************************

grid::grid(const Rivet::Histo1DPtr histPtr,
           const std::string pdfname,
           const std::string _analysis,
           const int    LO,
           const double xmin,
           const double xmax,
           const double q2min,
           const double q2max,
           const gridArch arch):
histo         ( histPtr ),
mode          ( globalFillMode ),
leadingOrder  ( LO ),
path          ( idFromPath(histo.get()->path()) ),
analysis      ( _analysis ),
applpdf       ( static_cast<mcgrid_pdf*>( appl::appl_pdf::getpdf(pdfname) ) ),
nSubProc      ( applpdf->Nproc() ),
weights       ( new double[nSubProc] ),
fl1projection ( new double[11] ),
fl2projection ( new double[11] )
{
  // Convert projection arrays to LHA basis
  fl1projection+=5;
  fl2projection+=5;
  
  cout << "MCgrid:: grid initialised for";
  switch (mode)
  {
    case FILL_GENERIC:
      cout << " generic fill "<<endl;
      break;
      
    case FILL_SHERPA:
      cout << " sherpa fill "<<endl;
      break;
  }
  
  // Target to check if phase space file exists
  std::stringstream phasename;
  phasename << "mcgrid"<<analysis << "/phasespace/"<<path<<".root";
  
  if (Rivet::fileexists(phasename.str()))
  {
    // Check event counter is initialised
    if (!applpdf->isInitialised())
    {
      cerr << "MCgrid Error: Phase space optimised grid is available, but event count data is not present."<<endl;
      cerr << "                  Please rerun the phase space run to generate event count data." <<endl;
      exit(-1);
    }
    
    // Read phase space file
    cout << "MCgrid: Reading phase space optimised grid"<<endl;
    applgrid = new appl::grid(phasename.str());
    return;
  }
  
  // If no phase space grid found, generate a new one
  // Init test grid based on histogram to get correct binning
  cout << "MCgrid: Generating new APPLgrid for histogram " << path;
  cout << " of analysis " << analysis << endl;
  createPath("mcgrid"+analysis+"/phasespace/");
  
  applgrid = new appl::grid(getBinning(histo),
                            arch.nQ,  q2min, q2max,  arch.qOrd,
                            arch.nX,  xmin, xmax,  arch.xOrd,
                            applpdf->name(),
                            leadingOrder, 1,
                            std::string("f2")
                            );
}

// Grid class destructor
grid::~grid()
{
  delete applgrid;
  
  fl1projection-=5;
  fl2projection-=5;
  
  delete[] fl1projection;
  delete[] fl2projection;
  delete[] weights;
}

/**
 *  grid::fill
 *  Provides the conversion of a HepMC event into the appropriate
 *  fill call for applgrid, based upon the specified fillMode
 **/
void grid::fill( double coord, const Rivet::Event& event)
{
  // Verify event is correctly filled
  const HepMC::PdfInfo *pdfinfo = event.genEvent()->pdf_info();
  if (!pdfinfo) {
    cerr << "MCgrid: Error: No PDF info in HepMC record!"<<endl;
    return;
  }

  // Check for minimum weight info
  if (event.genEvent()->weights().size() < 5)
  {
    cerr << "MCgrid: Error: HepMC usr_weights vector incorrectly formatted for use with MCgrid"<<endl;
    cerr << "                   Event must fill minimum required information as specified in documentation."<<endl;
    return;
  }
  
  // Fetch basic fill info from the event
  const fillInfo info(event, leadingOrder);
  
  if (info.ptord < 0 || info.ptord > 1)
  {
    cerr << "MCgrid: Error - event perturbative order is invalid: "<<info.ptord<<endl<<"MCgrid: ensure the process LO power is correctly set, currently: "<<leadingOrder<<endl;
    exit(-1);
  }
  
  // Fill reference histogram
  applgrid->getReference()->Fill(coord, info.wgt);
  
  switch (mode)
  {
    case FILL_GENERIC:
      genericFill(coord, info);
      break;
      
    case FILL_SHERPA:
      sherpaFill(coord, info);
      break;
  }

  return;
}

// Zeros the weight container
void grid::zeroWeights()
{
  for (int i=0; i<nSubProc; i++)
    weights[i] = 0;
}

// Populate the subprocess weight array with a single weight
void grid::fillWeight(const int fl1,
                      const int fl2,
                      const double eventweight
                      )
{
  // Identify subprocess for event/subevent
  const int subproc = applpdf->decideSubProcess(fl1,fl2);
  weights[subproc] += eventweight;
}

void grid::projectWeights ( const int fl1,  // beam 1 flavour
                            const int fl2,  // beam 2 flavour
                            const double wgt,  // weight to be projected
                            wgtProjector projectBeam1,
                            wgtProjector projectBeam2
                          )
{
  // Build projections
  projectBeam1(fl1, fl1projection);
  projectBeam2(fl2, fl2projection);

  // Project weights onto beam partons
  for ( int i=-5; i<=5; i++ )
    if (fl1projection[i])
      for( int j=-5; j<=5; j++ )
        if (fl2projection[j])
          fillWeight(i, j, fl1projection[i]*fl2projection[j]*wgt);
  return;
}


// Write the APPLgrid to file
void grid::exportgrid()
{
  // Base filename
  std::stringstream targetname;
  targetname << "mcgrid";
  createPath(targetname.str());
  targetname << analysis<<"/";
  createPath(targetname.str());

  // Check to see if this was just a phase space fill run
  if (!(applgrid->isOptimised()))
  {
    cout << "MCgrid: Optimising grid phase space ..."<<endl;
    applgrid->optimise();
    cout << "MCgrid: ... grid optimised, writing out phase space grid."<<endl;
    targetname << "phasespace/";
    createPath(targetname.str());

    targetname <<path<<".root";
  } else
  {
    cout << "MCgrid: Exporting final "<<path<<" APPLgrid."<<endl;
    targetname << path<<".root";
  }
  
  // Write out grid
  applgrid->Write(targetname.str());
  cout << "MCgrid: Export Complete"<<endl;
  
  return;
}

// Scale the weight output of the APPLgrid
void grid::scale( double const& scale)
{
  applgrid->run() = 1.0/scale;
  applgrid->setNormalised(false);
  
  cout << "MCgrid:  "<<path<<" Grid Normalisation set: "<< 1.0/applgrid->run()<< endl;
}
  
}
