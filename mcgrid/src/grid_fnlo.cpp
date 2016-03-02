//
//  grid_fnlo.cpp
//  MCgrid 03/05/2015.
//

#include "config.h"

#if FASTNLO_ENABLED

#include "fastnlotk/fastNLOCreate.h"
#include "fastnlotk/read_steer.h"

#include "grid_fnlo.hh"

using Rivet::cerr;
using Rivet::cout;
using Rivet::endl;

namespace MCgrid {

  std::string _grid_fnlo::gridInterfaceName() const
  {
    return _grid::gridInterfaceName(fastnloInterface);
  }
  _grid_fnlo::_grid_fnlo(const Rivet::Histo1DPtr histPtr,
                         const std::string _analysis,
                         fastnloConfig config):
    _grid(histPtr, _analysis, config.lo)
  {
    // For fastNLO-based grids, we need to create the grid before the pdf,
    // as it is using fastNLOCreate instance methods, for example to retrieve
    // the number of subprocesses.

    // Inform the user what we're up to
    cout << "MCgrid: Use fastNLO as underlying grid implementation" << endl;

    const std::string str = config.subprocConfig.fileName;
    const std::string steeringNameSpace = phasespaceFilePath();

    // Values passed here will overwrite a possible value in the steering
    ADD_NS("DifferentialDimension", 1, steeringNameSpace);
    std::vector<int> dimensionIsDifferential(1, 2);
    ADDARRAY_NS("DimensionIsDifferential", dimensionIsDifferential, steeringNameSpace);
    ADD_NS("CalculateBinSize", true, steeringNameSpace);
    ADD_NS("BinSizeFactor", 1.0, steeringNameSpace);
    ADDARRAY_NS("SingleDifferentialBinning", getBinning(histo), steeringNameSpace);
    ADD_NS("LeadingOrder", config.lo, steeringNameSpace);
    ADD_NS("OutputFilename", gridFileName(0), steeringNameSpace);
    ADD_NS("FlexibleScaleTable", false, steeringNameSpace);
    ADD_NS("ReadBinningFromSteering", true, steeringNameSpace);
    ADD_NS("NPDF", 2, steeringNameSpace);
    ADD_NS("NPDFDim", 2, steeringNameSpace);
    ADD_NS("IPDFdef1", 3, steeringNameSpace);
    ADD_NS("IPDFdef2", 0, steeringNameSpace);
    ADD_NS("CenterOfMassEnergy", config.centerOfMassEnergy, steeringNameSpace);
    if (!config.arch.isEmpty()) {
      ADD_NS("X_Kernel", config.arch.xkernel, steeringNameSpace);
      ADD_NS("Mu1_Kernel", config.arch.qkernel, steeringNameSpace);
      ADD_NS("X_DistanceMeasure", config.arch.xdistanceMeasure, steeringNameSpace);
      ADD_NS("Mu1_DistanceMeasure", config.arch.qdistanceMeasure, steeringNameSpace);
      ADD_NS("X_NNodes", config.arch.nX, steeringNameSpace);
      ADD_NS("Mu1_NNodes", config.arch.nQ, steeringNameSpace);
      ADD_NS("X_NoOfNodesPerMagnitude", config.arch.nXPerMagnitude, steeringNameSpace);
    }
    std::vector<double> description(1, 1.0);
    ADDARRAY_NS("ScaleVariationFactors", description, steeringNameSpace);

    // Read the steering file (it should at least contain the subproc list(s))
    READ_NS(str, steeringNameSpace);


    // Check some values for consistency if they are set in the steering

    if (EXIST_NS(PDF1, steeringNameSpace) && EXIST_NS(PDF2, steeringNameSpace)) {
       std::pair<int, int> fastNLOBeams(INT_NS(PDF1, steeringNameSpace), INT_NS(PDF2, steeringNameSpace));
       std::pair<int, int> analysisBeams(beamTypeToPDG(config.subprocConfig.beam1),
                                   beamTypeToPDG(config.subprocConfig.beam2));
      if (fastNLOBeams != analysisBeams) {
        cerr << "MCgrid::Error - The steering file specifies " << fastNLOBeams;
        cerr << ", but in the analysis we have " << analysisBeams;
        cerr << '.' << endl;
        exit(-1);
      }
    } else {
      ADD_NS("PDF1", beamTypeToPDG(config.subprocConfig.beam1), steeringNameSpace);
      ADD_NS("PDF2", beamTypeToPDG(config.subprocConfig.beam2), steeringNameSpace);
    }


    // Values passed here will also overwrite a possible value in the steering,
    // but we can check for its existence first.

    // Set ScenarioName default
    if (!EXIST_NS(ScenarioName, str)) {
      ADD_NS("ScenarioName", path, steeringNameSpace);
    }

    // Set ScenarioDescription.RIVET_ID default
    if (!CONTAINKEYARRAY_NS(RIVET_ID, ScenarioDescription, steeringNameSpace)) {
      // Generate Rivet ID skipping the leading slash '/'
      std::string rivetID("RIVET_ID=" + analysis.substr(1) + "/" + path);
      if (EXISTARRAY_NS(ScenarioDescription, steeringNameSpace)) {
        PUSHBACKARRAY_NS(rivetID, ScenarioDescription, steeringNameSpace);
      } else {
        std::vector<std::string> description(1, rivetID);
        ADDARRAY_NS("ScenarioDescription", description, steeringNameSpace);
      }
    }

    if (mode == FILL_SHERPA) {
      if (!EXISTARRAY_NS(CodeDescription, steeringNameSpace)) {
        std::vector<std::string> description(1, "Sherpa");
        ADDARRAY_NS("CodeDescription", description, steeringNameSpace);
      }
    }

    // Set OutputPrecision default
    if (!EXIST_NS(OutputPrecision, steeringNameSpace)) {
      ADD_NS("OutputPrecision", 8, steeringNameSpace);
    }

    // Set GlobalVerbosity default
    if (!EXIST_NS(GlobalVerbosity, str)) {
      ADD_NS("GlobalVerbosity", "ERROR", str);
    }

    // Set ApplyPDFReweighting default
    if (!EXIST_NS(ApplyPDFReweighting, steeringNameSpace)) {
      ADD_NS("ApplyPDFReweighting", true, steeringNameSpace);
    }

    // Set CheckScaleLimitsAgainstBins default
    if (!EXIST_NS(CheckScaleLimitsAgainstBins, steeringNameSpace)) {
      ADD_NS("CheckScaleLimitsAgainstBins", true, steeringNameSpace);
    }

    ftableBase = new fastNLOCreate(str,
                                   steeringNameSpace,
                                   false);
    ftableBase->SetOrderOfAlphasOfCalculation(config.lo);
    if (ftableBase->GetIsWarmup()) {
      ftableNLO = NULL;
    } else {
      // This is no warmup run, prepare to fill NLO events
      // As it is initialized from the same steering,
      // the NLO grid will use the same warmup values than the LO grid
      ftableNLO = new fastNLOCreate(str,
                                    steeringNameSpace,
                                    false);
      ftableNLO->SetOrderOfAlphasOfCalculation(config.lo + 1);
    }

    mcgrid_base_pdf_params *pdf_params = new mcgrid_fnlo_pdf_params(config.subprocConfig.fileName,
                                                                    ftableBase);

    readPDFWithParameters(*pdf_params, analysis);
    delete pdf_params;
  }

  _grid_fnlo::~_grid_fnlo() {
    delete ftableBase;
    if (ftableNLO != NULL) {
      delete ftableNLO;
    }
  }

  void _grid_fnlo::fillReferenceHistogram(double coord, double wgt) {
    // Does fastNLO support reference histograms?
  }

  bool _grid_fnlo::isWarmup() const
  {
    return ftableBase->GetIsWarmup();
  }

  void _grid_fnlo::exportgrid()
  {
    if (isWarmup()) {
      cout << "MCgrid: Writing out phase space grid." << endl;
    } else {
      cout << "MCgrid: Exporting final " << gridInstanceString[fastnloInterface];
      cout << "." << endl;
    }

    const uint64_t nEvents(PDFHandler::NEvents());
    ftableBase->SetNumberOfEvents(nEvents);
    if (isWarmup()) {
      ftableBase->WriteTable();
    } else {
      ftableNLO->SetNumberOfEvents(nEvents);
      scaleTables(nEvents);

      // fastNLOCreate objects cannot contain more than one contribution.
      // Its superclass however can handle this. So let's upcast.
      fastNLOTable *ftable = static_cast<fastNLOTable*>(ftableBase);
      ftable->AddTable(static_cast<fastNLOTable>(*ftableNLO));

      // Determine file name and write
      ftable->SetFilename(gridOrPhasespaceFilePath());
      ftable->WriteTable();
    }

    cout << "MCgrid: Export Complete"<<endl;
  }

  void _grid_fnlo::scale(double const & scale)
  {
    _grid::scale(scale);
    if (!isWarmup()) {
      scaleTables(scale);
    }
  }

  void _grid_fnlo::scaleTables(double const & scale)
  {
    if (scale != 1.0) {
      ftableBase->MultiplyCoefficientsByConstant(scale);
      ftableNLO->MultiplyCoefficientsByConstant(scale);
    }
  }

  void _grid_fnlo::fillUnderlyingGrid(const double x1,
                                      const double x2,
                                      const double pdfQ2,
                                      const double coord,
                                      const termType termType)
  {
    // we do not yet support scale log tables in fastNLO
    assert(termType == LO || termType == NLO);

    // Determine which table should be filled
    fastNLOCreate *ftable;
    if (isWarmup() || !(perturbativeOrderForTermType(termType) == 1)) {
      ftable = ftableBase;
    } else {
      ftable = ftableNLO;
    }

    // Fill table
    // For warmup runs, only the combination (x1, x2, Q2)
    // is relevant to update the ranges of the grid dimensions.
    // So filling one subproc is enough
    const int nSubProcToBeFilled = isWarmup() ? 1 : nSubProc;
    for (int i=0; i<nSubProcToBeFilled; i++) {
      ftable->fEvent.SetProcessId(i);
      ftable->fEvent.SetWeight(weights[i]/x1/x2);
      ftable->fEvent.SetX1(x1);
      ftable->fEvent.SetX2(x2);
      ftable->fScenario.SetObservable0(coord);
      ftable->fScenario.SetObsScale1(sqrt(pdfQ2));
      ftable->Fill(0);
      ftable->fEvent.Reset();
    }
  }

  std::string _grid_fnlo::phasespaceFileExtension() const
  {
    return "txt";
  }


  std::string _grid_fnlo::gridFileExtension() const
  {
    return "tab";
  }
}

#endif
