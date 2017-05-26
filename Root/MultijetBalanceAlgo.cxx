// EL include(s):
#include <EventLoop/Job.h>
#include <EventLoop/Worker.h>
#include "EventLoop/OutputStream.h"

// EDM include(s):
#include "AthContainers/ConstDataVector.h"
#include "AthContainers/DataVector.h"
#include "xAODTracking/VertexContainer.h"
#include <xAODJet/JetContainer.h>
#include "xAODEventInfo/EventInfo.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"

#include "PathResolver/PathResolver.h"

// ROOT include(s):
#include "TFile.h"
#include "TEnv.h"
#include "TSystem.h"
#include "TKey.h"

// c++ includes(s):
#include <iostream>
#include <fstream>

// package include(s):
//#include <xAODAnaHelpers/tools/ReturnCheck.h>
//#include <xAODAnaHelpers/tools/ReturnCheckConfig.h>
#include <MultijetBalance/MultijetBalanceAlgo.h>
#include <xAODAnaHelpers/HelperFunctions.h>
#include <MultijetBalance/MultijetHists.h>

// external tools include(s):
#include "JetCalibTools/JetCalibrationTool.h"
#include "JetUncertainties/JetUncertaintiesTool.h"
#include "JetResolution/JERTool.h"
#include "JetResolution/JERSmearingTool.h"

#include "JetSelectorTools/JetCleaningTool.h"
#include "JetMomentTools/JetVertexTaggerTool.h"
#include "JetJvtEfficiency/JetJvtEfficiency.h"

#include "xAODBTaggingEfficiency/BTaggingSelectionTool.h"
#include "xAODBTaggingEfficiency/BTaggingEfficiencyTool.h"

//!!#include "SystTool/SystContainer.h"

#include "JetTileCorrection/JetTileCorrectionTool.h"

#include "InsituSelections.h"
#include "JetCalibrations.h"

using namespace std;
//using namespace std::placeholders;

// this is needed to distribute the algorithm to the workers
ClassImp(MultijetBalanceAlgo)



MultijetBalanceAlgo :: MultijetBalanceAlgo (std::string name) :
  Algorithm(),
  m_name(name),
  m_JetCalibrationTool_handle("JetCalibrationTool/JetCalibrationTool_"+name),
  m_JetUncertaintiesTool_handle("JetUncertaintiesTool/JetUncertaintiesTool_"+name),
  m_JERTool_handle("JERTool/JERTool_"+name),
  m_JERSmearingTool_handle("JERSmearingTool/JERSmearingTool_"+name),
  m_JetCleaningTool_handle("JetCleaningTool/JetCleaningTool_"+name),
  m_JVTUpdateTool_handle("JetVertexTaggerTool/JVTUpdateTool_"+name),
  m_JetJVTEfficiencyTool_handle("CP::JetJVTEfficiency/JVTEfficiencyTool_"+name),
  m_JetJVTEfficiencyTool_handle_up("CP::JetJVTEfficiency/JVTEfficiencyToolUp_"+name),
  m_JetJVTEfficiencyTool_handle_down("CP::JetJVTEfficiency/JVTEfficiencyToolDown_"+name),
  m_JetTileCorrectionTool_handle("CP::JetTileCorrectionTool/JetTileCorrectionTool_"+name)
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().

  // configuration variables set by user
  m_validation = false;
  m_closureTest = false;
  m_bootstrap = false;

  m_TileCorrection = false;

  m_inContainerName_jets = "";
  m_inContainerName_photons = "";
  m_triggerAndPt = "";
  m_MJBIteration = 0;
  m_MJBIterationThreshold = "999999";
  m_MJBCorrectionFile = "";
  m_binning = "";
  m_VjetCalibFile = "";
  m_leadingGSC  = false;
  m_systTool_nToys = 100;

  m_sysVariations = "Nominal";
  m_MJBStatsOn = false;

  m_numJets = 3;
  m_ptAsymVar = 0.8;
  m_ptAsymMin = 0.;
  m_alpha = 0.3;
  m_beta = 1.0;
  m_betaPtVar = 0.;
  m_ptThresh = 25.;
  m_leadJetPtThresh = 0.;

  m_writeTree = false;
  m_writeNominalTree = false;
  m_MJBDetailStr = "";
  m_eventDetailStr = "";
  m_jetDetailStr = "";
  m_trigDetailStr = "";

  m_debug = false;
  m_MCPileupCheckContainer = "AntiKt4TruthJets";
  m_isAFII = false;
  m_useCutFlow = true;
  m_XSFile = "$ROOTCOREBIN/data/MultijetBalance/XsAcc_13TeV.txt"; 


  m_bTag = true;
  m_bTagFileName = "$ROOTCOREBIN/data/xAODAnaHelpers/2015-PreRecomm-13TeV-MC12-CDI-October23_v1.root";
  m_bTagVar    = "MV2c20";
//  m_bTagOP = "FixedCutBEff_70";
  m_useDevelopmentFile = true;
  m_useConeFlavourLabel = true;
  m_bTagWPsString = "";


  //config for Jet Tools
  m_jetDef = "";
  m_jetCalibSequence = "";
  m_jetCalibConfig = "";
  m_jetCleanCutLevel = "";
  m_jetCleanUgly = false;
  m_JVTWP = "Medium";
  m_jetUncertaintyConfig = "";

  m_JERUncertaintyConfig = "";
  m_JERApplySmearing = false;
  m_JERSystematicMode = "Simple";


  MJBmode = false;
  Gmode = true;
  Zmode = false;

}

MultijetBalanceAlgo ::~MultijetBalanceAlgo(){
}

EL::StatusCode  MultijetBalanceAlgo :: configure (){
  Info("configure()", "Configuring MultijetBalanceAlgo Interface.");

  if( m_inContainerName_jets.empty() ) {
    Error("configure()", "InputContainer is empty!");
    return EL::StatusCode::FAILURE;
  }


  // Save binning to use
  std::stringstream ssb(m_binning);
  std::string thisSubStr;
  std::string::size_type sz;
  while (std::getline(ssb, thisSubStr, ',')) {
    m_bins.push_back( std::stof(thisSubStr, &sz) );
  }
  Info("configure()", "Setting binning to %s", m_binning.c_str());

  // Save b-tag WPs to use
  std::stringstream ssbtag(m_bTagWPsString);
  while (std::getline(ssbtag, thisSubStr, ',')) {
    m_bTagWPs.push_back( thisSubStr );
  }
  Info("configure()", "Setting b-tag WPs to %s", m_bTagWPsString.c_str());

  // Save triggers to use
  std::stringstream ss(m_triggerAndPt);
  while (std::getline(ss, thisSubStr, ',')) {
    m_triggers.push_back( thisSubStr.substr(0, thisSubStr.find_first_of(':')) );
    m_triggerThresholds.push_back( std::stof(thisSubStr.substr(thisSubStr.find_first_of(':')+1, thisSubStr.size()) , &sz) *1e3 );
  }

  // Save subleading pt thresholds to use
  std::stringstream ss2(m_MJBIterationThreshold);
  while( std::getline(ss2, thisSubStr, ',')) {
    m_subjetThreshold.push_back( std::stof(thisSubStr, &sz) * 1e3 );
  }

  //If using MCPileupCheck
  if( m_MCPileupCheckContainer.compare("None") == 0 )
    m_useMCPileupCheck = false;
  else
    m_useMCPileupCheck = true;
  

  if( m_validation ){
    if( m_MJBIteration > 0 ){
      Error("config","Trying to run validation mode on an MJBIteration above 0, which is not allowed. Maybe you want to use m_closureTest instead? Exiting.");
      return EL::StatusCode::FAILURE;
    }
    Info("config", "Setting subleading pt threshold to a large number in validation mode");
    m_subjetThreshold.at(0) = 9999999999.;
  }// if m_validation

  // Setup bootstrap depending upon iteration
  // For data, bootstrap toys will be propagated from MJB correction histograms
  m_iterateBootstrap = false;
  if( m_bootstrap && !m_isMC && m_MJBIteration > 0){
    Info("configure()", "Running bootstrap mode on subsequent iteration.  Turning off cutflow, ttree, and unnecessary histograms.");
    m_iterateBootstrap = true;
    m_bootstrap = false;
    m_useCutFlow = false;
    m_jetDetailStr += " bootstrapIteration";
    m_writeTree = false;
    m_writeNominalTree = false;
  }

  if( m_writeNominalTree )
    m_writeTree = true;

  // Determine if special V+jets calibrations are to be used
  if (m_VjetCalibFile.size() > 0){
    m_VjetCalib = true;
  } else {
    m_VjetCalib = false;
  }
  // if subleading threshold 0 is set to -1000, the value will be determined from the code.
  // Don't let this happen if we're not using m_VjetCalib
  if( !m_VjetCalib && (m_subjetThreshold.at(0) == -1000) ){
    Error("config", "Not running on VjetCalib, yet first MJBIterationThreshold is set to -1.  Exiting.");
    return EL::StatusCode::FAILURE;
  }
  ///// Jet calib and uncertainty Tool Config parameters /////
  if ( !m_isMC && m_jetCalibSequence.find("Insitu") == std::string::npos){
    m_jetCalibSequence += "_Insitu";
    Warning("configure()", "Adding _Insitu to data Jet Calibration");
  }

  if( m_isMC && m_jetCalibSequence.find("Insitu") != std::string::npos ){
    Error("configure()", "Attempting to use an Insitu calibration sequence on MC.  Exiting.");
    return EL::StatusCode::FAILURE;
  }

  Info("configure()", "JetCalibTool: %s, %s", m_jetCalibConfig.c_str(), m_jetCalibSequence.c_str() );
  Info("configure()", "MultijetBalanceAlgo Interface succesfully configured! \n");

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetBalanceAlgo :: setupJob (EL::Job& job)
{
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.
  job.useXAOD();
  xAOD::Init( "MultijetBalanceAlgo" ).ignore(); // call before opening first file

  EL::OutputStream outForTree("tree");
  job.outputAdd (outForTree);

  if( m_bootstrap )
    job.outputAdd(EL::OutputStream("SystToolOutput"));


  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MultijetBalanceAlgo :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.
  Info("histInitialize()", "Calling histInitialize \n");



  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MultijetBalanceAlgo :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



//EL::StatusCode MultijetBalanceAlgo :: changeInput (bool firstFile)
//{
//  // Here you do everything you need to do when we change input files,
//  // e.g. resetting branch addresses on trees.  If you are using
//  // D3PDReader or a similar service this method is not needed.
//  return EL::StatusCode::SUCCESS;
//}



EL::StatusCode MultijetBalanceAlgo :: initialize ()
{
  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files.  You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed.  So any objects
  // you create here won't be available in the output if you have no
  // input events.

  m_event = wk()->xaodEvent();
  m_store = wk()->xaodStore();
  m_eventCounter = -1;

  const xAOD::EventInfo* eventInfo = 0;
  ANA_CHECK( HelperFunctions::retrieve(eventInfo, "EventInfo", m_event, m_store, msg()) );
  m_isMC = ( eventInfo->eventType( xAOD::EventInfo::IS_SIMULATION ) ) ? true : false;

  ANA_CHECK( this->configure() );

  ANA_CHECK(getSampleWeights(eventInfo) );

  // load all variations
  setupJetCalibrationStages();
  ANA_CHECK(loadJetUncertaintyTool());
  ANA_CHECK(loadJetResolutionTool());
  ANA_CHECK(loadSystematics());

  //load Calibration and systematics files
  ANA_CHECK(loadJetCalibrationTool());
  ANA_CHECK(loadJetCleaningTool());
  ANA_CHECK(loadJVTTool());
  ANA_CHECK(loadBTagTools());
  if( m_TileCorrection )
    ANA_CHECK(loadJetTileCorrectionTool());
  if( m_VjetCalib )
    ANA_CHECK(loadVjetCalibration());

  ANA_CHECK(loadMJBCalibration());

//!!  if( m_bootstrap ){
//!!    systTool = new SystContainer(m_sysName, m_bins, m_systTool_nToys);
//!!  }
//

    //// Add the defined selections, in correct order /////
    std::vector< std::string > cutflowNames; 
    if( MJBmode ){

      std::function<bool(void)> func_LeadEta = std::bind(&MultijetBalanceAlgo::cut_LeadEta, this);
      m_selections.push_back(func_LeadEta);
      cutflowNames.push_back( "LeadEta" );
      m_selType.push_back( PRE );

      std::function<bool(void)> func_JetEta = std::bind(&MultijetBalanceAlgo::cut_JetEta, this);
      m_selections.push_back(func_JetEta);
      cutflowNames.push_back( "JetEta" );
      m_selType.push_back( PRE );

      std::function<bool(void)> func_SubPt = std::bind(&MultijetBalanceAlgo::cut_SubPt, this);
      m_selections.push_back(func_SubPt);
      cutflowNames.push_back( "SubPt" );
      m_selType.push_back( PRE );

      std::function<bool(void)> func_MCCleaning = std::bind(&MultijetBalanceAlgo::cut_MCCleaning, this);
      m_selections.push_back(func_MCCleaning);
      cutflowNames.push_back( "MCCleaning" );
      m_selType.push_back( PRE );

      std::function<bool(void)> func_JetPtThresh = std::bind(&MultijetBalanceAlgo::cut_JetPtThresh, this);
      m_selections.push_back(func_JetPtThresh);
      cutflowNames.push_back( "JetPtThresh" );
      m_selType.push_back( SYST );

      std::function<bool(void)> func_JVT = std::bind(&MultijetBalanceAlgo::cut_JVT, this);
      m_selections.push_back(func_JVT);
      cutflowNames.push_back( "JVT" );
      m_selType.push_back( SYST );

      std::function<bool(void)> func_TriggerEffRecoil = std::bind(&MultijetBalanceAlgo::cut_TriggerEffRecoil, this);
      m_selections.push_back(func_TriggerEffRecoil);
      cutflowNames.push_back( "TriggerEffRecoil" );
      m_selType.push_back( RECOIL );

      std::function<bool(void)> func_PtAsym = std::bind(&MultijetBalanceAlgo::cut_PtAsym, this);
      m_selections.push_back(func_PtAsym);
      cutflowNames.push_back( "PtAsym" );
      m_selType.push_back( RECOIL );

      std::function<bool(void)> func_Alpha = std::bind(&MultijetBalanceAlgo::cut_Alpha, this);
      m_selections.push_back(func_Alpha);
      cutflowNames.push_back( "Alpha" );
      m_selType.push_back( RECOIL );

      std::function<bool(void)> func_Beta = std::bind(&MultijetBalanceAlgo::cut_Beta, this);
      m_selections.push_back(func_Beta);
      cutflowNames.push_back( "Beta" );
      m_selType.push_back( RECOIL );

      std::function<bool(void)> func_CleanJet = std::bind(&MultijetBalanceAlgo::cut_CleanJet, this);
      m_selections.push_back(func_CleanJet);
      cutflowNames.push_back( "CleanJet" );
      m_selType.push_back( RECOIL );

    }
    
    if( Gmode ){
      
      std::function<bool(void)> func_TriggerEffRecoil = std::bind(&MultijetBalanceAlgo::cut_TriggerEffRecoil, this);
      m_selections.push_back(func_TriggerEffRecoil);
      cutflowNames.push_back( "TriggerEffRecoil" );
      m_selType.push_back( PRE );

      std::function<bool(void)> func_JetEta = std::bind(&MultijetBalanceAlgo::cut_JetEta, this);
      m_selections.push_back(func_JetEta);
      cutflowNames.push_back( "JetEta" );
      m_selType.push_back( PRE );
     
      ///// Photon E/P 
      std::function<bool(void)> func_ConvPhot = std::bind(&MultijetBalanceAlgo::cut_ConvPhot, this);
      m_selections.push_back(func_ConvPhot);
      cutflowNames.push_back( "ConvPhot" );
      m_selType.push_back( PRE );

// jet-photon overlap removal      
//      std::function<bool(void)> func_JVT = std::bind(&MultijetBalanceAlgo::cut_JVT, this);
//      m_selections.push_back(func_JVT);
//      cutflowNames.push_back( "JVT" );
//      m_selType.push_back( SYST );

      std::function<bool(void)> func_MCCleaning = std::bind(&MultijetBalanceAlgo::cut_MCCleaning, this);
      m_selections.push_back(func_MCCleaning);
      cutflowNames.push_back( "MCCleaning" );
      m_selType.push_back( PRE );

      std::function<bool(void)> func_JVT = std::bind(&MultijetBalanceAlgo::cut_JVT, this);
      m_selections.push_back(func_JVT);
      cutflowNames.push_back( "JVT" );
      m_selType.push_back( SYST );

      ////// Here we now have the leading jet /////
      std::function<bool(void)> func_LeadJetPtThresh = std::bind(&MultijetBalanceAlgo::cut_LeadJetPtThresh, this);
      m_selections.push_back(func_LeadJetPtThresh);
      cutflowNames.push_back( "LeadJetPtThresh" );
      m_selType.push_back( SYST );

      std::function<bool(void)> func_LeadEta = std::bind(&MultijetBalanceAlgo::cut_LeadEta, this);
      m_selections.push_back(func_LeadEta);
      cutflowNames.push_back( "LeadEta" );
      m_selType.push_back( SYST );
      
      std::function<bool(void)> func_PtAsym = std::bind(&MultijetBalanceAlgo::cut_PtAsym, this);
      m_selections.push_back(func_PtAsym);
      cutflowNames.push_back( "PtAsym" );
      m_selType.push_back( RECOIL );

      //// This is delta phi ////////////
      std::function<bool(void)> func_Alpha = std::bind(&MultijetBalanceAlgo::cut_Alpha, this);
      m_selections.push_back(func_Alpha);
      cutflowNames.push_back( "Alpha" );
      m_selType.push_back( RECOIL );

      std::function<bool(void)> func_CleanJet = std::bind(&MultijetBalanceAlgo::cut_CleanJet, this);
      m_selections.push_back(func_CleanJet);
      cutflowNames.push_back( "CleanJet" );
      m_selType.push_back( RECOIL );

    }



  for(unsigned int iVar=0; iVar < m_sysName.size(); ++iVar){
    Info("initialize", "Systematic var %i is %s at index %i", iVar, m_sysName.at(iVar).c_str(), m_sysDetail.at(iVar) );
  }

  if(m_useCutFlow) {
    Info("initialize", "Setting Cutflow");

    //std::string newName;
    TFile *file = wk()->getOutputFile ("cutflow");
    TH1D* origCutflowHist = (TH1D*)file->Get("cutflow");
    TH1D* origCutflowHistW = (TH1D*)file->Get("cutflow_weighted");

    //for(int i=0; i < origCutflowHist->GetNbinsX()+1; ++i){
    //  std::cout << origCutflowHist->GetXaxis()->GetBinLabel(i) << std::endl;
    //}

    for(unsigned int iCut = 0; iCut < cutflowNames.size(); ++iCut){
      if( iCut == 0)
        m_cutflowFirst = origCutflowHist->GetXaxis()->FindBin( cutflowNames.at(iCut).c_str() );
      else
        origCutflowHist->GetXaxis()->FindBin( cutflowNames.at(iCut).c_str() );

      origCutflowHistW->GetXaxis()->FindBin( cutflowNames.at(iCut).c_str() );
    }
    //Add a cutflow for each variation
    for(unsigned int iVar=0; iVar < m_sysName.size(); ++iVar){
      m_cutflowHist.push_back( (TH1D*) origCutflowHist->Clone() );
      m_cutflowHistW.push_back( (TH1D*) origCutflowHistW->Clone() );
      m_cutflowHist.at(iVar)->SetName( ("cutflow_"+m_sysName.at(iVar)).c_str() );
      m_cutflowHistW.at(iVar)->SetName( ("cutflow_weighted_"+m_sysName.at(iVar)).c_str() );
      m_cutflowHist.at(iVar)->SetTitle( ("cutflow_"+m_sysName.at(iVar)).c_str() );
      m_cutflowHistW.at(iVar)->SetTitle( ("cutflow_weighted_"+m_sysName.at(iVar)).c_str() );
      m_cutflowHist.at(iVar)->SetDirectory( file );
      m_cutflowHistW.at(iVar)->SetDirectory( file );

      //Need to retroactively fill original bins of these histograms
      for(unsigned int iBin=1; iBin < m_cutflowFirst; ++iBin){
        m_cutflowHist.at(iVar)->SetBinContent(iBin, origCutflowHist->GetBinContent(iBin) );
        m_cutflowHistW.at(iVar)->SetBinContent(iBin, origCutflowHistW->GetBinContent(iBin) );
      }//for iBin
    }//for each m_sysName
  } //m_useCutflow

  //Add output hists for each variation
  m_ss << m_MJBIteration;
  for(unsigned int iVar=0; iVar < m_sysName.size(); ++iVar){
    std::string histOutputName = "Iteration"+m_ss.str()+"_"+m_sysName.at(iVar);

    // If the output is a bootstrap, then we need histograms to be written directly to the root file, and not in
    // TDirectory structures.  This is done b/c hadding too many TDirectories is too slow.
    // To turn off TDirectory structure, the name must end in "_"
    if (m_iterateBootstrap || m_bootstrap)
      histOutputName += "_";
    MultijetHists* thisJetHists = new MultijetHists( histOutputName, (m_jetDetailStr+" "+m_MJBDetailStr).c_str() );
    m_jetHists.push_back(thisJetHists);
    m_jetHists.at(iVar)->initialize(m_binning);
    m_jetHists.at(iVar)->record( wk() );
  }
  m_ss.str("");



  //Writing nominal tree only requies this sample to have the nominal output
  if( m_writeNominalTree && m_NominalIndex < 0){
    m_writeNominalTree = false;
  }

  if( m_writeTree){
    Info("initialize", "Setting TTrees");
    TFile * treeFile = wk()->getOutputFile ("tree");
    if( !treeFile ) {
      Error("initialize()","Failed to get file for output tree!");
      return EL::StatusCode::FAILURE;
    }
    for(int unsigned iVar=0; iVar < m_sysName.size(); ++iVar){
      if (m_writeNominalTree && (int) iVar != m_NominalIndex)
        continue;

      TTree * outTree = new TTree( ("outTree_"+m_sysName.at(iVar)).c_str(), ("outTree_"+m_sysName.at(iVar) ).c_str());
      if( !outTree ) {
        Error("initialize()","Failed to get output tree!");
        return EL::StatusCode::FAILURE;
      }
      outTree->SetDirectory( treeFile );
      MiniTree* thisMiniTree = new MiniTree(m_event, outTree, treeFile);
      m_treeList.push_back(thisMiniTree);
    }//for iVar

    for( unsigned int iTree=0; iTree < m_treeList.size(); ++iTree){
      m_treeList.at(iTree)->AddEvent(m_eventDetailStr);
      if (m_bTagWPsString.size() > 0){
        m_treeList.at(iTree)->AddJets( (m_jetDetailStr+" MJBbTag_"+m_bTagWPsString).c_str());
      }else{
        m_treeList.at(iTree)->AddJets( (m_jetDetailStr).c_str());
      }
      m_treeList.at(iTree)->AddTrigger( m_trigDetailStr );
    }//for iTree

  }//if m_writeTree

  Info("initialize()", "Succesfully initialized output TTree! \n");

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MultijetBalanceAlgo :: execute ()
{
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.
  if(m_debug) Info("execute()", "Begin Execute");
  ++m_eventCounter;

  if(m_eventCounter %100000 == 0)
    Info("execute()", "Event # %i", m_eventCounter);


  m_iCutflow = m_cutflowFirst; //for cutflow histogram automatic filling

  //----------------------------
  // Event information
  //---------------------------
  ///////////////////////////// Retrieve Containers /////////////////////////////////////////
  if(m_debug) Info("execute()", "Retrieve Containers ");

  m_eventInfo = 0;
  //const xAOD::EventInfo* eventInfo = 0;
  ANA_CHECK( HelperFunctions::retrieve(m_eventInfo, "EventInfo", m_event, m_store, msg()) );
  m_mcEventWeight = (m_isMC ? m_eventInfo->mcEventWeight() : 1.) ;

  const xAOD::VertexContainer* vertices = 0;
  ANA_CHECK( HelperFunctions::retrieve(vertices, "PrimaryVertices", m_event, m_store, msg()) );
  m_pvLocation = HelperFunctions::getPrimaryVertexLocation( vertices );  //Get primary vertex for JVF cut

  const xAOD::JetContainer* inJets = 0;
  ANA_CHECK( HelperFunctions::retrieve(inJets, m_inContainerName_jets, m_event, m_store, msg()) );

  m_truthJets = 0;
  //const xAOD::JetContainer* m_truthJets = 0;
  if(m_useMCPileupCheck && m_isMC){
    ANA_CHECK( HelperFunctions::retrieve(m_truthJets, m_MCPileupCheckContainer, m_event, m_store, msg()) );
  }

  
  /// Create recoilParticle ///  
  const xAOD::PhotonContainer* inPhotons = 0;
  if( Gmode ){
    ANA_CHECK( HelperFunctions::retrieve(inPhotons, m_inContainerName_photons, m_event, m_store, msg()) );

    m_recoilParticle = inPhotons->at(0);
    m_recoilTLV.SetPtEtaPhiE(m_recoilParticle->pt(), m_recoilParticle->eta(), m_recoilParticle->phi(), m_recoilParticle->e() );
  }

  
  ///// For jets, create an editable shallow copy container & vector where jets are removable //////
  std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > jetsSC = xAOD::shallowCopyContainer( *inJets );

  m_jets = new std::vector< xAOD::Jet* >();
  //std::vector< xAOD::Jet*>* jets = new std::vector< xAOD::Jet* >();
  for( auto thisJet : *(jetsSC.first) ) {
     m_jets->push_back( thisJet );
   }


  ////////////////////////////////// Apply jet calibrations ////////////////////////////////////////
  if(m_debug) Info("execute()", "Apply Jet Calibration Tool ");
  ANA_CHECK( applyJetCalibrationTool( m_jets ) );
  reorderJets( m_jets );


  // Apply dedicated V+jet calibration on subleading jets if MJB mode //
  if( MJBmode && m_VjetCalib){ // Only apply if we're running MJB and Vjet Calibration is turned on
    ANA_CHECK( applyVjetCalibration( m_jets ) );
  }
 

  ////// Do the event selections before looping over systematics  //////
  for(unsigned int iS = 0; iS < m_selections.size(); ++iS){
    if( m_selType.at(iS) != PRE )
      continue;
    
    if( m_selections.at(iS)() ){
      fillCutflowAll(iS);
    }else{
      // If it fails a preselection, exit to next event //
      delete jetsSC.first; delete jetsSC.second; delete m_jets;
      wk()->skipEvent();  return EL::StatusCode::SUCCESS;
    }
  }// for preselections
  
  // MJB: A vector to save the calibrated P4 of jets
  std::vector<xAOD::JetFourMom_t> jets_calibratedP4;
  for(unsigned int iJet=0; iJet < m_jets->size(); ++iJet){
    xAOD::JetFourMom_t thisJet;
    thisJet.SetCoordinates(m_jets->at(iJet)->pt(), m_jets->at(iJet)->eta(), m_jets->at(iJet)->phi(), m_jets->at(iJet)->m());
    jets_calibratedP4.push_back(thisJet);
  }

  //// MJB: create new container to save the good jets up to this point ////
  std::vector< xAOD::Jet*>* savedJets = new std::vector< xAOD::Jet* >();
  *savedJets = *m_jets;
//for (unsigned int iJet = 0; iJet < m_jets->size(); ++iJet){
//std::cout << "mjets " << m_jets->at(iJet)->pt() << std::endl;
//}

  ////// Loop over systematic variations of recoil objects /////////////
  for(unsigned int iSys=0; iSys < m_sysName.size(); ++iSys){

    m_iSys = iSys;

    *m_jets = *savedJets;  // Reset the vector of pointers
    //// If MJB, need to reset 4-mom of jets and apply new systematic variations
    if( MJBmode ){

      //Reset each jet pt to calibrated copy (including V+jet if applicable)
      for (unsigned int iJet = 0; iJet < m_jets->size(); ++iJet){

        if(m_sysType.at(iSys) == JCS){
          // A systematic variation that lets us use any jet calibration stage for all jets
          int iCalibStage = m_sysDetail.at(iSys);
          xAOD::JetFourMom_t jetCalibStageCopy = m_jets->at(iJet)->getAttribute<xAOD::JetFourMom_t>( m_JCSStrings.at(iCalibStage).c_str() );
          m_jets->at(iJet)->setJetP4( jetCalibStageCopy );
        } else {

          m_jets->at(iJet)->setJetP4( jets_calibratedP4.at(iJet) );
  
        }
      }// for each jet

      // Apply additional jet corrections based on systematic variation.
      // Will apply iterative MJB calibration or JES / JER uncertainties. 
      applyJetSysVariation(m_jets, iSys);

      // Apply tile correction tool last
      applyJetTileCorrectionTool(m_jets, iSys);

      reorderJets( m_jets );
    }//If MJB mode


    ////// Do the event selections for each systematic before the recoil object is built //////
    bool hasFailedSyst = false;
    for(unsigned int iS = 0; iS < m_selections.size(); ++iS){
      if( m_selType.at(iS) != SYST )
        continue;
      if( m_selections.at(iS)() ){
        fillCutflow(iS, iSys);
      }else{
        hasFailedSyst = true;
        break;
      }
    }
    /// If it failed a syst selection, continue to the next systematic variation without filling output ///
    if( hasFailedSyst )
      continue;


    ////////// Build the recoil object for MJB //////////
    if( MJBmode ){
      //Create recoilJets object from all nonleading, passing jets
      m_recoilTLV.SetPtEtaPhiM(0,0,0,0);
      for (unsigned int iJet = 1; iJet < m_jets->size(); ++iJet){
        TLorentzVector tmpJet;
        tmpJet.SetPtEtaPhiE(m_jets->at(iJet)->pt(), m_jets->at(iJet)->eta(), m_jets->at(iJet)->phi(), m_jets->at(iJet)->e());
        m_recoilTLV += tmpJet;
      }
    }else if( Gmode ){
      m_recoilTLV.SetPtEtaPhiM(0,0,0,0);
    }

    ////// Do the event selections for each systematic after the recoiling object is built //////
    bool hasFailed = false;
    for(unsigned int iS = 0; iS < m_selections.size(); ++iS){
      if( m_selType.at(iS) != RECOIL )
        continue;
      if( m_selections.at(iS)() ){
        fillCutflow(iS, iSys);
      }else{
        hasFailed = true;
        break;
      }
    }
    /// If it failed a syst selection, continue to the next systematic variation without filling output ///
    if( hasFailed )
      continue;


    //%%%%%%%%%%%%%%%%%%%%%%%%%%% End Selections %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%5

    ////////////////////// Add Extra Variables //////////////////////////////
    m_eventInfo->auxdecor< int >("njet") = m_jets->size();
    m_eventInfo->auxdecor< float >( "recoilPt" ) = m_recoilTLV.Pt();
    m_eventInfo->auxdecor< float >( "recoilEta" ) = m_recoilTLV.Eta();
    m_eventInfo->auxdecor< float >( "recoilPhi" ) = m_recoilTLV.Phi();
    m_eventInfo->auxdecor< float >( "recoilM" ) = m_recoilTLV.M();
    m_eventInfo->auxdecor< float >( "recoilE" ) = m_recoilTLV.E();
    m_eventInfo->auxdecor< float >( "ptBal" ) = m_jets->at(0)->pt() / m_recoilTLV.Pt();

    for(unsigned int iJet=0; iJet < m_jets->size(); ++iJet){

      vector<float> thisEPerSamp = m_jets->at(iJet)->auxdata< vector< float> >("EnergyPerSampling");
      float TotalE = 0., TileE = 0.;
      for( int iLayer=0; iLayer < 24; ++iLayer){
        TotalE += thisEPerSamp.at(iLayer);
      }

      TileE += thisEPerSamp.at(12);
      TileE += thisEPerSamp.at(13);
      TileE += thisEPerSamp.at(14);

      m_jets->at(iJet)->auxdecor< float >( "TileFrac" ) = TileE / TotalE;

    }


    m_eventInfo->auxdecor< float >("weight_mcEventWeight") = m_mcEventWeight;
    m_eventInfo->auxdecor< float >("weight_prescale") = m_prescale;
    m_eventInfo->auxdecor< float >("weight_xs") = m_weight_xs * m_weight_kfactor;
    float weight_pileup = 1.;
    if(m_eventInfo->isAvailable< float >("PileupWeight") ){
      weight_pileup = m_eventInfo->auxdecor< float >("PileupWeight");
    }



    if(m_isMC)
      m_eventInfo->auxdecor< float >("weight") = m_mcEventWeight*m_weight_xs*m_weight_kfactor*weight_pileup;
    else
      m_eventInfo->auxdecor< float >("weight") = m_prescale;

    /////////////// Output Plots ////////////////////////////////
    if(m_debug) Info("execute()", "Begin Hist output for %s", m_sysName.at(iSys).c_str() );
    m_jetHists.at(iSys)->execute( m_jets, m_eventInfo);


    if(m_debug) Info("execute()", "Begin TTree output for %s", m_sysName.at(iSys).c_str() );
    ///////////////// Optional MiniTree Output for Nominal Only //////////////////////////
    if( m_writeTree ) {
      if(!m_writeNominalTree ||  m_NominalIndex == (int) iSys) {
      //!! The following is a bit slow!
//        std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > originalSignalJetsSC = xAOD::shallowCopyContainer( *inJets );
        xAOD::JetContainer* plottingJets = new xAOD::JetContainer();
        xAOD::JetAuxContainer* plottingJetsAux = new xAOD::JetAuxContainer();
        plottingJets->setStore( plottingJetsAux );
        for(unsigned int iJet=0; iJet < m_jets->size(); ++iJet){
          xAOD::Jet* newJet = new xAOD::Jet();
          newJet->makePrivateStore( *(m_jets->at(iJet)) );
          plottingJets->push_back( newJet );
        }

        int iTree = iSys;
        if( m_writeNominalTree)
          iTree = 0;
        if(m_eventInfo)   m_treeList.at(iTree)->FillEvent( m_eventInfo    );
        if(m_jets)  m_treeList.at(iTree)->FillJets(  plottingJets);
        m_treeList.at(iTree)->FillTrigger( m_eventInfo );
        m_treeList.at(iTree)->Fill();

        delete plottingJets;
        delete plottingJetsAux;
      }//If it's not m_writeNominalTree or else we're on the nominal sample
    }//if m_writeTree


    /////////////////////////////////////// SystTool ////////////////////////////////////////
//!!    if( m_bootstrap ){
//!!      systTool->fillSyst(m_sysName.at(iSys), eventInfo->runNumber(), eventInfo->eventNumber(), recoilJets.Pt()/GeV, (signalJets->at(0)->pt()/recoilJets.Pt()), eventInfo->auxdecor< float >("weight") );
//!!    }

  } // loop over systematic variations of recoil objects


  delete jetsSC.first; delete jetsSC.second; delete m_jets; delete savedJets;

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetBalanceAlgo :: postExecute ()
{
  if(m_debug) Info("postExecute()", "postExecute");
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetBalanceAlgo :: finalize ()
{
  if(m_debug) Info("finalize()", "finalize");
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.
  for(unsigned int iVar=0; iVar < m_sysName.size(); ++iVar){
    m_jetHists.at(iVar)->finalize();
  }

  if(m_debug) Info("finalize()", "Done with jes hists");

//!!  if( m_bootstrap ){
//!!    systTool->writeToFile(wk()->getOutputFile("SystToolOutput"));
//!!    //delete systTool;  systTool = nullptr;
//!!  }


  //Need to retroactively fill original bins of these histograms
  if(m_useCutFlow) {
    TFile *file = wk()->getOutputFile ("cutflow");
    TH1D* origCutflowHist = (TH1D*)file->Get("cutflow");
    TH1D* origCutflowHistW = (TH1D*)file->Get("cutflow_weighted");

    for(unsigned int iVar=0; iVar < m_sysName.size(); ++iVar){
      for(unsigned int iBin=1; iBin < m_cutflowFirst; ++iBin){
        m_cutflowHist.at(iVar)->SetBinContent(iBin, origCutflowHist->GetBinContent(iBin) );
        m_cutflowHistW.at(iVar)->SetBinContent(iBin, origCutflowHistW->GetBinContent(iBin) );
      }//for iBin
    }//for each m_sysName

    //Add one cutflow histogram to output for number of initial events
    std::string thisName;
    if(m_isMC)
      m_ss << m_mcChannelNumber;
    else
      m_ss << m_runNumber;

    // Get Nominal Cutflow if it's available
    TH1D *histCutflow, *histCutflowW;
    if( m_NominalIndex >= 0){
      histCutflow = (TH1D*) m_cutflowHist.at(m_NominalIndex)->Clone();
      histCutflowW = (TH1D*) m_cutflowHistW.at(m_NominalIndex)->Clone();
    } else {
      histCutflow = (TH1D*) m_cutflowHist.at(0)->Clone();
      histCutflowW = (TH1D*) m_cutflowHistW.at(0)->Clone();
    }
    histCutflow->SetName( ("cutflow_"+m_ss.str()).c_str() );
    histCutflowW->SetName( ("cutflow_weighted_"+m_ss.str()).c_str() );

    wk()->addOutput(histCutflow);
    wk()->addOutput(histCutflowW);
  }//m_useCutFlow

  if(m_debug) Info("finalize()", "Done with cutflow hists");
  //Only if Nominal is available
  if( m_writeTree && m_NominalIndex >= 0) {
    TH1D *treeCutflow, *treeCutflowW;
    if( m_NominalIndex >= 0){
      treeCutflow = (TH1D*) m_cutflowHist.at(m_NominalIndex)->Clone();
      treeCutflowW = (TH1D*) m_cutflowHistW.at(m_NominalIndex)->Clone();
    }else{
      treeCutflow = (TH1D*) m_cutflowHist.at(m_NominalIndex)->Clone();
      treeCutflowW = (TH1D*) m_cutflowHistW.at(m_NominalIndex)->Clone();
    }

    treeCutflow->SetName( ("cutflow_"+m_ss.str()).c_str() );
    treeCutflowW->SetName( ("cutflow_weighted_"+m_ss.str()).c_str() );

    TFile * treeFile = wk()->getOutputFile ("tree");
    treeCutflow->SetDirectory( treeFile );
    treeCutflowW->SetDirectory( treeFile );
  }

  m_ss.str( std::string() );
  if(m_debug) Info("finalize()", "Done finalize");

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MultijetBalanceAlgo :: histFinalize ()
{
  if(m_debug) Info("histFinalize()", "histFinalize");
  // This method is the mirror image of histInitialize(), meaning it
  // gets called after the last event has been processed on the worker
  // node and allows you to finish up any objects you created in
  // histInitialize() before they are written to disk.  This is
  // actually fairly rare, since this happens separately for each
  // worker node.  Most of the time you want to do your
  // post-processing on the submission node after all your histogram
  // outputs have been merged.  This is different from finalize() in
  // that it gets called on all worker nodes regardless of whether
  // they processed input events.



  return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetBalanceAlgo::fillCutflow(int iSel, int iVar){
  if(m_useCutFlow) {
    if(m_debug) Info("fillCutflow()", "Passing Cut %i-%i", iSel, iVar);
    m_cutflowHist.at(iVar)->Fill(iSel+m_cutflowFirst, 1);
    m_cutflowHistW.at(iVar)->Fill(iSel+m_cutflowFirst, m_mcEventWeight);
  }

return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetBalanceAlgo::fillCutflowAll(int iSel){
  if(m_useCutFlow) {
    if(m_debug) Info("fillCutflowAll()", "Passing Cut");
    for(unsigned int iVar=0; iVar < m_sysName.size(); ++iVar){
      m_cutflowHist.at(iVar)->Fill(iSel+m_cutflowFirst, 1);
      m_cutflowHistW.at(iVar)->Fill(iSel+m_cutflowFirst, m_mcEventWeight);
    }
    m_iCutflow++;
  }

return EL::StatusCode::SUCCESS;
}

//This grabs luminosity, acceptace, and eventNumber information from the respective text file
EL::StatusCode MultijetBalanceAlgo::getSampleWeights(const xAOD::EventInfo* eventInfo) {
  if(m_debug) Info("getSampleWeights()", "getSampleWeights");

  float weight_xs = 1.0, weight_kfactor = 1.0, weight_eff = 1.0;
  if(!m_isMC){
    m_runNumber = eventInfo->runNumber();
  }else{
    m_mcChannelNumber = eventInfo->mcChannelNumber();
//    ifstream fileIn(  PathResolverFindCalibFile( "$ROOTCOREBIN/data/MultijetBalance/susy_crosssections_13TeV.txt" ) );
    ifstream fileIn(  PathResolverFindCalibFile( "MultijetBalance/susy_crosssections_13TeV.txt" ) );

    // Search the input file for the correct MCChannelNumber, and find the XS and kfactor numbers
    std::string mcChanStr = std::to_string( m_mcChannelNumber );
    bool foundXS = false;
    std::string line;
    std::string subStr;
    while (getline(fileIn, line)){
      istringstream iss(line);
      // DSID should be first item in the line, so put it into subStr and see if mcChanStr matches
      iss >> subStr;
      if (subStr.find(mcChanStr) != std::string::npos){
        iss >> subStr; // Get Name
        iss >> subStr; // Get XS (ipb)
        sscanf(subStr.c_str(), "%e", &weight_xs);
        iss >> subStr; // Get kfactor
        sscanf(subStr.c_str(), "%e", &weight_kfactor);
        iss >> subStr; // Get Efficiency
        sscanf(subStr.c_str(), "%e", &weight_eff);
        //iss >> subStr; // Get relative uncertainty
        Info("getSampleWeights", "Setting xs=%f, efficiency=%f, acceptance=%f", weight_xs, weight_eff, weight_kfactor);
        foundXS = true;
        break;
      }
    }
    if( !foundXS){
      cerr << "ERROR: Could not find proper file information for MC sample " << mcChanStr << endl;
      return EL::StatusCode::FAILURE;
    }

    // Decorate eventInfo with information //
    m_weight_xs = weight_xs*weight_eff;
    m_weight_kfactor = weight_kfactor;
  }

  return EL::StatusCode::SUCCESS;
}

//Calculate DeltaPhi
double MultijetBalanceAlgo::DeltaPhi(double phi1, double phi2){
  phi1=TVector2::Phi_0_2pi(phi1);
  phi2=TVector2::Phi_0_2pi(phi2);
  return fabs(TVector2::Phi_mpi_pi(phi1-phi2));
}

//Calculate DeltaR
double MultijetBalanceAlgo::DeltaR(double eta1, double phi1,double eta2, double phi2){
  phi1=TVector2::Phi_0_2pi(phi1);
  phi2=TVector2::Phi_0_2pi(phi2);
  double dphi = DeltaPhi( phi1, phi2);
  double deta = eta1-eta2;
  return sqrt(deta*deta+dphi*dphi);
}

EL::StatusCode MultijetBalanceAlgo :: loadSystematics (){
  if(m_debug) Info("loadSystematics()", "loadSystematics");

  // Define the All systematic // 
  if( m_sysVariations.find("All") != std::string::npos){
    if(m_isMC){
      m_sysVariations = "Nominal-EvSel-JER";
    }else{
      m_sysVariations = "Nominal-EvSel-JES";
    }
  }

  m_NominalIndex = -1; //The index of the nominal

  // Turn into a vector of all systematics names
  std::vector< std::string> varVector;
  size_t pos = 0;
  while ((pos = m_sysVariations.find("-")) != std::string::npos){
    varVector.push_back( m_sysVariations.substr(0, pos) );
    m_sysVariations.erase(0, pos+1);
  }
  varVector.push_back( m_sysVariations ); //append final one

  for( unsigned int iVar = 0; iVar < varVector.size(); ++iVar ){

    /////////////////////////////// Nominal ///////////////////////////////
    if( varVector.at(iVar).compare("Nominal") == 0 ){
      m_sysName.push_back( "Nominal" ); m_sysType.push_back( NOMINAL ); m_sysDetail.push_back( -1 );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
      m_NominalIndex = m_sysName.size()-1;

    /////////////////// Every Jet Calibration Stage ////////////////
    }else if( varVector.at(iVar).compare("JCS") == 0 ){
      if( m_JCSTokens.size() <= 0){
        Error( "loadSystematics()", "JetCalibSequence is empty.  This will not be added to the systematics");
      }
      Info( "loadSystematics()", "Adding JetCalibSequence");
      for( unsigned int iJCS = 0; iJCS < m_JCSTokens.size(); ++iJCS){
        //Name - JetCalibTool - Variation Number - sign
        m_sysName.push_back("JCS_"+m_JCSTokens.at(iJCS) ); m_sysType.push_back( JCS ); m_sysDetail.push_back( iJCS );
        m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
      }

    ////////////////////////////////// JES Uncertainties /////////////////////////////////////////
    } else if( varVector.at(iVar).find("JES") != std::string::npos ){
      const CP::SystematicSet recSysts = m_JetUncertaintiesTool_handle->recommendedSystematics();
      std::vector<CP::SystematicSet> JESSysList = HelperFunctions::getListofSystematics( recSysts, "All", 1, msg() ); //All sys at +-1 sigma
      for(unsigned int i=1; i < JESSysList.size(); ++i){
        m_sysName.push_back( JESSysList.at(i).name() );   m_sysType.push_back( JES ); m_sysDetail.push_back( i ); m_sysSet.push_back( JESSysList.at(i) );
      }

    //////////////////////////////////////// JER  /////////////////////////////////////////
    } else if( varVector.at(iVar).find("JER") != std::string::npos ){
      const CP::SystematicSet recSysts = m_JERSmearingTool_handle->recommendedSystematics();
      std::vector<CP::SystematicSet> JERSysList = HelperFunctions::getListofSystematics( recSysts, "All", 1, msg() ); //All sys at +-1 sigma
      for(unsigned int i=1; i < JERSysList.size(); ++i){
        m_sysName.push_back( JERSysList.at(i).name() );   m_sysType.push_back( JER ); m_sysDetail.push_back( i ); m_sysSet.push_back( JERSysList.at(i) );
      }
    
    } else if( varVector.at(iVar).find("JVT") != std::string::npos ){
      m_sysName.push_back( "JVT__1down" );   m_sysType.push_back( JVT ); m_sysDetail.push_back( -1 ); 
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
      
      m_sysName.push_back( "JVT__1up" );   m_sysType.push_back( JVT ); m_sysDetail.push_back( 1 ); 
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );

    //////////////////////////////////////// Event Selection  /////////////////////////////////////////
    } else if( varVector.at(iVar).compare("EvSel") == 0 ){
      //Name - EvSel Variation - EvSel Value - sign

      //Alpha systematics are +-.1  (*100)
      m_sysName.push_back("EvSel_Alpha"+to_string(int(round(m_alpha*100))-10)+"__1down" );   m_sysType.push_back( CUTAlpha ); m_sysDetail.push_back( -0.1 );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
      m_sysName.push_back("EvSel_Alpha"+to_string(int(round(m_alpha*100))+10)+"__1up" );   m_sysType.push_back( CUTAlpha ); m_sysDetail.push_back( 0.1 );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );

      //Beta systematics are +-.5 (*10)
      m_sysName.push_back("EvSel_Beta"+to_string(int(round(m_beta*10))-5)+"__1down" );   m_sysType.push_back( CUTBeta ); m_sysDetail.push_back( -0.5 );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
      m_sysName.push_back("EvSel_Beta"+to_string(int(round(m_beta*10))+5)+"__1up" );     m_sysType.push_back( CUTBeta ); m_sysDetail.push_back( 0.5 );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );

      //pt Asymmetry systematics are +-.1 (*100)
      if( MJBmode ){
        m_sysName.push_back("EvSel_Asym"+to_string(int(round(m_ptAsymVar*100))-10)+"__1down" );   m_sysType.push_back( CUTAsym ); m_sysDetail.push_back( -0.1 );
        m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
        m_sysName.push_back("EvSel_Asym"+to_string(int(round(m_ptAsymVar*100))+10)+"__1up" );     m_sysType.push_back( CUTAsym ); m_sysDetail.push_back( 0.1 );
        m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
      } else {
        m_sysName.push_back("EvSel_Asym"+to_string(int(round(m_ptAsymVar*100))-5)+"__1down" );   m_sysType.push_back( CUTAsym ); m_sysDetail.push_back( -0.05 );
        m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
        m_sysName.push_back("EvSel_Asym"+to_string(int(round(m_ptAsymVar*100))+5)+"__1up" );     m_sysType.push_back( CUTAsym ); m_sysDetail.push_back( 0.05 );
        m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
      }

      //pt threshold systematics are +- 5
      m_sysName.push_back("EvSel_Threshold"+to_string(int(round(m_ptThresh))-5)+"__1down" );   m_sysType.push_back( CUTPt ); m_sysDetail.push_back( -5 );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
      m_sysName.push_back("EvSel_Threshold"+to_string(int(round(m_ptThresh))+5)+"__1up" );     m_sysType.push_back( CUTPt ); m_sysDetail.push_back( 5 );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );

    }

  }//for varVector
  return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetBalanceAlgo :: setupJetCalibrationStages() {

  if(m_debug) Info("setupJetCalibrationStages", "setupJetCalibrationStages");
  // Setup calibration stages tools //
  // Create a map from the CalibSequence string components to the xAOD aux data
  std::map <std::string, std::string> JCSMap;
  if (m_inContainerName_jets.find("EMTopo") != std::string::npos || m_inContainerName_jets.find("EMPFlow") != std::string::npos)
    JCSMap["RAW"] = "JetEMScaleMomentum";
  else if( m_inContainerName_jets.find("LCTopo") != std::string::npos )
    JCSMap["RAW"] = "JetConstitScaleMomentum";
  else{
    Error( "setupJetCalibrationStages()", " Input jets are not EMScale, EMPFlow or LCTopo.  Exiting.");
    return EL::StatusCode::FAILURE;
  }
  JCSMap["JetArea"] = "JetPileupScaleMomentum";
  JCSMap["Origin"] = "JetOriginConstitScaleMomentum";
  JCSMap["EtaJES"] = "JetEtaJESScaleMomentum";
  JCSMap["GSC"] = "JetGSCScaleMomentum";
  JCSMap["Insitu"] = "JetInsituScaleMomentum";


  //// Now break up the Jet Calib string into the components
  // m_JCSTokens will be a vector of fields in the m_jetCalibSequence
  // m_JCSStrings will be a vector of the Jet momentum States
  size_t pos = 0;
  std::string JCSstring = m_jetCalibSequence;
  std::string token;
  m_JCSTokens.push_back( "RAW" );  //The original xAOD value
  m_JCSStrings.push_back( JCSMap["RAW"] );
  while( JCSstring.size() > 0){
    pos = JCSstring.find("_");
    if (pos != std::string::npos){
      token = JCSstring.substr(0, pos);
      JCSstring.erase(0, pos+1);
    }else{
      token = JCSstring;
      JCSstring.erase();
    }
    //Skip the Residual one, it seems JetArea_Residual == Pileup
    if (token.find("Residual") == std::string::npos){
      m_JCSTokens.push_back( token );
      m_JCSStrings.push_back( JCSMap[token] );
    }
  }

  return EL::StatusCode::SUCCESS;
}

// initialize and configure B-tagging efficiency tools
EL::StatusCode MultijetBalanceAlgo :: loadBTagTools(){

  if (! m_bTag){
    return EL::StatusCode::SUCCESS;
  }

  for(unsigned int iB=0; iB < m_bTagWPs.size(); ++iB){ 

    // Initialize & Configure the BJetSelectionTool
    asg::AnaToolHandle<IBTaggingSelectionTool> this_BTaggingSelectionTool_handle;
    this_BTaggingSelectionTool_handle.setTypeAndName("BTaggingSelectionTool/BTaggingSelectionTool_"+m_bTagWPs.at(iB)+"_"+m_name);

    if( !this_BTaggingSelectionTool_handle.isUserConfigured() ){
      ANA_CHECK( ASG_MAKE_ANA_TOOL( this_BTaggingSelectionTool_handle, BTaggingSelectionTool ) );

      std::string thisBTagOP = "FixedCutBEff_"+m_bTagWPs.at(iB);
      ANA_CHECK( this_BTaggingSelectionTool_handle.setProperty("MaxEta",2.5) );
      ANA_CHECK( this_BTaggingSelectionTool_handle.setProperty("MinPt",20000.) );
      ANA_CHECK( this_BTaggingSelectionTool_handle.setProperty("FlvTagCutDefinitionsFileName",m_bTagFileName.c_str()) );
      ANA_CHECK( this_BTaggingSelectionTool_handle.setProperty("TaggerName",          m_bTagVar) );
      ANA_CHECK( this_BTaggingSelectionTool_handle.setProperty("OperatingPoint",      thisBTagOP) );
      ANA_CHECK( this_BTaggingSelectionTool_handle.setProperty("JetAuthor",           (m_jetDef+"Jets").c_str()) );

      ANA_CHECK( this_BTaggingSelectionTool_handle.retrieve() );
      Info("loadBTagTools()", "BTaggingSelectionTool initialized : %s ", this_BTaggingSelectionTool_handle.name().c_str() );

    }
    m_AllBTaggingSelectionTool_handles.push_back( this_BTaggingSelectionTool_handle );

    // Initialize & Configure the BJetEfficiencyCorrectionTool

    if( m_isMC) {
      asg::AnaToolHandle<IBTaggingEfficiencyTool> this_BTaggingEfficiencyTool_handle;
      this_BTaggingEfficiencyTool_handle.setTypeAndName("BTaggingEfficiencyTool/BTaggingEfficiencyTool_"+m_bTagWPs.at(iB)+"_"+m_name);

      if( !this_BTaggingEfficiencyTool_handle.isUserConfigured() ){
        ANA_CHECK( ASG_MAKE_ANA_TOOL( this_BTaggingEfficiencyTool_handle, BTaggingEfficiencyTool ) );

        std::string thisBTagOP = "FixedCutBEff_"+m_bTagWPs.at(iB);
        ANA_CHECK( this_BTaggingEfficiencyTool_handle.setProperty("TaggerName",          m_bTagVar) );
        ANA_CHECK( this_BTaggingEfficiencyTool_handle.setProperty("OperatingPoint",      thisBTagOP) );
        ANA_CHECK( this_BTaggingEfficiencyTool_handle.setProperty("JetAuthor",           (m_jetDef+"Jets").c_str()) );
        ANA_CHECK( this_BTaggingEfficiencyTool_handle.setProperty("ScaleFactorFileName", m_bTagFileName.c_str()) );
        ANA_CHECK( this_BTaggingEfficiencyTool_handle.setProperty("UseDevelopmentFile",  m_useDevelopmentFile) );
        ANA_CHECK( this_BTaggingEfficiencyTool_handle.setProperty("ConeFlavourLabel",    m_useConeFlavourLabel) );

        ANA_CHECK( this_BTaggingEfficiencyTool_handle.retrieve() );
        Info("loadBTagTools()", "BTaggingEfficiencyTool initialized : %s ", this_BTaggingEfficiencyTool_handle.name().c_str() );

      }
      m_AllBTaggingEfficiencyTool_handles.push_back( this_BTaggingEfficiencyTool_handle );
    }
  }//for iB working points

  return EL::StatusCode::SUCCESS;
}
