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

using namespace std;
using namespace std::placeholders;

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
  m_JetJVTEfficiencyTool_handle("CP::JetJVTEfficiency/JBTEfficiencyTool_"+name),
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

  m_inContainerName = "";
  m_triggerAndPt = "";
  m_MJBIteration = 0;
  m_MJBIterationThreshold = "";
  m_MJBCorrectionFile = "";
  m_binning = "";
  m_VjetCalibFile = "";
  m_leadingGSC  = false;
  m_systTool_nToys = 100;

  m_sysVariations = "Nominal";
  m_MJBStatsOn = false;

  m_numJets = 3;
  m_ptAsym = 0.8;
  m_alpha = 0.3;
  m_beta = 1.0;
  m_ptThresh = 25.;
  m_looseBetaCut = false;

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


  MJBmode = "true";

}

MultijetBalanceAlgo ::~MultijetBalanceAlgo(){
}

EL::StatusCode  MultijetBalanceAlgo :: configure (){
  Info("configure()", "Configuring MultijetBalanceAlgo Interface.");

  if( m_inContainerName.empty() ) {
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
  // if 
  // subleading threshold 0 is set to -1000, the value will be determined from the code.
  // Don't let this happen if we're not using m_VjetCalib
  if( !m_VjetCalib && (m_subjetThreshold.at(0) == -1000) ){
    Error("config", "Not running on VjetCalib, yet first MJBIterationThreshold is set to -1.  Exiting.");
    return EL::StatusCode::FAILURE;
  }

  ///// Jet calib and uncertainty Tool Config parameters /////

  m_jetUncertaintyConfig = gSystem->ExpandPathName( m_jetUncertaintyConfig.c_str() );

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
  //RETURN_CHECK("init", HelperFunctions::retrieve(eventInfo, "EventInfo", m_event, m_store), "Failed to retrieve EventInfo");
  m_isMC = ( eventInfo->eventType( xAOD::EventInfo::IS_SIMULATION ) ) ? true : false;

  ANA_CHECK( this->configure() );

  ANA_CHECK(getLumiWeights(eventInfo) );

  // load all variations
  setupJetCalibrationStages();
//!!  ANA_CHECK(loadJetUncertaintyTool());
//!!  ANA_CHECK(loadJetResolutionTool());
  ANA_CHECK(loadVariations());

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

      std::function<bool(void)> func_MCCleaning = std::bind(&MultijetBalanceAlgo::cut_MCCleaning, this);
      m_selections.push_back(func_MCCleaning);
      cutflowNames.push_back( "MCCleaning" );
      m_selType.push_back( PRE );

      std::function<bool(void)> func_SubPt = std::bind(&MultijetBalanceAlgo::cut_SubPt, this);
      m_selections.push_back(func_SubPt);
      cutflowNames.push_back( "SubPt" );
      m_selType.push_back( PRE );

      std::function<bool(void)> func_JetPtThresh = std::bind(&MultijetBalanceAlgo::cut_JetPtThresh, this);
      m_selections.push_back(func_JetPtThresh);
      cutflowNames.push_back( "JetPtThresh" );
      m_selType.push_back( SYST );

      std::function<bool(void)> func_JVT = std::bind(&MultijetBalanceAlgo::cut_JVT, this);
      m_selections.push_back(func_JVT);
      cutflowNames.push_back( "JVT" );
      m_selType.push_back( SYST );

      std::function<bool(void)> func_CleanJet = std::bind(&MultijetBalanceAlgo::cut_CleanJet, this);
      m_selections.push_back(func_CleanJet);
      cutflowNames.push_back( "CleanJet" );
      m_selType.push_back( RECOIL );

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

//    std::vector< std::string > cutflowNames = {
//      "njets", "centralLead", "detEta",
//      "mcCleaning", "ptSub", "ptThreshold",
//      "JVT", "cleanJet", "TriggerEff",
//      "ptAsym", "alpha", "beta"
//    };
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
  ANA_CHECK( HelperFunctions::retrieve(inJets, m_inContainerName, m_event, m_store, msg()) );

  m_truthJets = 0;
  //const xAOD::JetContainer* m_truthJets = 0;
  if(m_useMCPileupCheck && m_isMC){
    ANA_CHECK( HelperFunctions::retrieve(m_truthJets, m_MCPileupCheckContainer, m_event, m_store, msg()) );
  }

  
  /// Create an empty recoilObject ///  
  TLorentzVector recoilObject;

  
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
  
  // MJB: A vector to save the calibrated P4 of jets
  std::vector<xAOD::JetFourMom_t> jets_calibratedP4;
  for(unsigned int iJet=0; iJet < m_jets->size(); ++iJet){
    xAOD::JetFourMom_t thisJet;
    thisJet.SetCoordinates(m_jets->at(iJet)->pt(), m_jets->at(iJet)->eta(), m_jets->at(iJet)->phi(), m_jets->at(iJet)->m());
    jets_calibratedP4.push_back(thisJet);
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

  //// MJB: create new container to save the good jets up to this point ////
  std::vector< xAOD::Jet*>* savedJets = new std::vector< xAOD::Jet* >();
  *savedJets = *m_jets;

  ////// Loop over systematic variations of recoil objects /////////////
  for(unsigned int iVar=0; iVar < m_sysName.size(); ++iVar){

    *m_jets = *savedJets;  // Reset the vector of pointers
    //// If MJB, need to reset 4-mom of jets and apply new systematic variations
    if( MJBmode ){

      //Reset each jet pt to calibrated copy (including V+jet if applicable)
      for (unsigned int iJet = 0; iJet < m_jets->size(); ++iJet){
        m_jets->at(iJet)->setJetP4( jets_calibratedP4.at(iJet) );
      }

      // Apply additional jet corrections based on systematic variation.
      // Will apply iterative MJB calibration or JES / JER uncertainties. 
      applyJetSysVariation(m_jets, iVar);

      // Apply tile correction tool last
      applyJetTileCorrectionTool(m_jets);

      reorderJets( m_jets );
    }//If MJB mode


    ////// Do the event selections for each systematic before the recoil object is built //////
    bool hasFailedSyst = false;
    for(unsigned int iS = 0; iS < m_selections.size(); ++iS){
      if( m_selType.at(iS) != SYST )
        continue;
      if( m_selections.at(iS)() ){
        fillCutflow(iS, iVar);
      }else{
        hasFailedSyst = true;
        break;
      }
    }
    /// If it failed a syst selection, continue to the next systematic variation without filling output ///
    if( hasFailedSyst )
      continue;

    ////////// Build the recoil object //////////

    if( MJBmode ){
      //Create recoilJets object from all nonleading, passing jets
      m_recoilTLV.SetPtEtaPhiM(0,0,0,0);
      for (unsigned int iJet = 1; iJet < m_jets->size(); ++iJet){
        TLorentzVector tmpJet;
        tmpJet.SetPtEtaPhiE(m_jets->at(iJet)->pt(), m_jets->at(iJet)->eta(), m_jets->at(iJet)->phi(), m_jets->at(iJet)->e());
        m_recoilTLV += tmpJet;
      }
    }

    ////// Do the event selections for each systematic after the recoiling object is built //////
    bool hasFailed = false;
    for(unsigned int iS = 0; iS < m_selections.size(); ++iS){
      if( m_selType.at(iS) != RECOIL )
        continue;
      if( m_selections.at(iS)() ){
        fillCutflow(iS, iVar);
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
    m_eventInfo->auxdecor< float >("weight_xs") = m_xs * m_acceptance;
    float weight_pileup = 1.;
    if(m_eventInfo->isAvailable< float >("PileupWeight") ){
      weight_pileup = m_eventInfo->auxdecor< float >("PileupWeight");
    }



    if(m_isMC)
      m_eventInfo->auxdecor< float >("weight") = m_mcEventWeight*m_xs*m_acceptance*weight_pileup;
    else
      m_eventInfo->auxdecor< float >("weight") = m_prescale;

    /////////////// Output Plots ////////////////////////////////
    if(m_debug) Info("execute()", "Begin Hist output for %s", m_sysName.at(iVar).c_str() );
    m_jetHists.at(iVar)->execute( m_jets, m_eventInfo);


    if(m_debug) Info("execute()", "Begin TTree output for %s", m_sysName.at(iVar).c_str() );
    ///////////////// Optional MiniTree Output for Nominal Only //////////////////////////
    if( m_writeTree ) {
      if(!m_writeNominalTree ||  m_NominalIndex == (int) iVar) {
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

        int iTree = iVar;
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
//!!      systTool->fillSyst(m_sysName.at(iVar), eventInfo->runNumber(), eventInfo->eventNumber(), recoilJets.Pt()/GeV, (signalJets->at(0)->pt()/recoilJets.Pt()), eventInfo->auxdecor< float >("weight") );
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
//format     147915 2.3793E-01 5.0449E-03 499000
EL::StatusCode MultijetBalanceAlgo::getLumiWeights(const xAOD::EventInfo* eventInfo) {
  if(m_debug) Info("getLumiWeights()", "getLumiWeights");

  if(!m_isMC){
    m_runNumber = eventInfo->runNumber();
    m_xs = 1;
    m_acceptance = 1;
  }else{
    m_mcChannelNumber = eventInfo->mcChannelNumber();
    ifstream fileIn(  PathResolverFindDataFile( m_XSFile ) );
    //ifstream fileIn(  gSystem->ExpandPathName( m_XSFile.c_str() ) );
    std::string runNumStr = std::to_string( m_mcChannelNumber );

    std::string line;
    std::string subStr;
    while (getline(fileIn, line)){
      istringstream iss(line);
      iss >> subStr;
      if (subStr.find(runNumStr) != std::string::npos){
        iss >> subStr;
        sscanf(subStr.c_str(), "%e", &m_xs);
        iss >> subStr;
        sscanf(subStr.c_str(), "%e", &m_acceptance);
        iss >> subStr;
        sscanf(subStr.c_str(), "%i", &m_numAMIEvents);
        Info("getLumiWeights", "Setting xs=%f , acceptance=%f , and numAMIEvents=%i ", m_xs, m_acceptance, m_numAMIEvents);
        continue;
      }
    }
    if( m_numAMIEvents <= 0){
      cerr << "ERROR: Could not find proper file information for file number " << runNumStr << endl;
      return EL::StatusCode::FAILURE;
    }
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

EL::StatusCode MultijetBalanceAlgo :: loadVariations (){
  if(m_debug) Info("loadVariations()", "loadVariations");

  // Add the configuration if All is used //
  if( m_sysVariations.find("All") != std::string::npos){
    if(m_isMC){
      m_sysVariations = "Nominal-localMJB-JER";
    }else{
      m_sysVariations = "Nominal-localMJB-JES";
      //m_sysVariations = "Nominal-localMJB-AllMJB-AllZjet-AllGjet-AllLAr-AllEtaIntercalibration-AllPileup-AllFlavor-AllPunchThrough";
    }
    //m_sysVariations = "Nominal-JetCalibSequence-Special-MJB-AllZjet-AllGjet-AllLAr";
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
      m_sysName.push_back( "Nominal" ); m_sysTool.push_back( -1 ); m_sysDetail.push_back( -1 );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
      m_NominalIndex = m_sysName.size()-1;

    /////////////////// Every Jet Calibration Stage ////////////////
    }else if( varVector.at(iVar).compare("JetCalibSequence") == 0 ){
      if( m_JCSTokens.size() <= 0){
        Error( "loadVariations()", "JetCalibSequence is empty.  This will not be added to the systematics");
      }
      Info( "loadVariations()", "Adding JetCalibSequence");
      for( unsigned int iJCS = 0; iJCS < m_JCSTokens.size(); ++iJCS){
        //Name - JetCalibTool - Variation Number - sign
        m_sysName.push_back("JCS_"+m_JCSTokens.at(iJCS) ); m_sysTool.push_back( 10 ); m_sysDetail.push_back( iJCS );
        m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
      }

    ////////////////////////////////// JES Uncertainties /////////////////////////////////////////
    } else if( varVector.at(iVar).find("JES") != std::string::npos ){
      const CP::SystematicSet recSysts = m_JetUncertaintiesTool_handle->recommendedSystematics();
      std::vector<CP::SystematicSet> JESSysList = HelperFunctions::getListofSystematics( recSysts, "All", 1, msg() ); //All sys at +-1 sigma
      for(unsigned int i=1; i < JESSysList.size(); ++i){
        m_sysName.push_back( JESSysList.at(i).name() );   m_sysTool.push_back( 0 ); m_sysDetail.push_back( i ); m_sysSet.push_back( JESSysList.at(i) );
      }

    //////////////////////////////////////// JER  /////////////////////////////////////////
    } else if( varVector.at(iVar).find("JER") != std::string::npos ){
      const CP::SystematicSet recSysts = m_JERSmearingTool_handle->recommendedSystematics();
      std::vector<CP::SystematicSet> JERSysList = HelperFunctions::getListofSystematics( recSysts, "All", 1, msg() ); //All sys at +-1 sigma
      for(unsigned int i=1; i < JERSysList.size(); ++i){
        m_sysName.push_back( JERSysList.at(i).name() );   m_sysTool.push_back( 1 ); m_sysDetail.push_back( i ); m_sysSet.push_back( JERSysList.at(i) );
      }

    //////////////////////////////////////// MJB  /////////////////////////////////////////
    } else if( varVector.at(iVar).compare("localMJB") == 0 ){
      //Name - MJB Variation - MJB Value - sign

      int systValue[2];
      //Alpha systematics are +-.1  (*100)
      systValue[0] = round(m_alpha*100)-10;
      systValue[1] = round(m_alpha*100)+10;
      m_sysName.push_back("MJB_a"+to_string(systValue[0])+"__1down" );   m_sysTool.push_back( 2 ); m_sysDetail.push_back( systValue[0] );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
      m_sysName.push_back("MJB_a"+to_string(systValue[1])+"__1up" );   m_sysTool.push_back( 2 ); m_sysDetail.push_back( systValue[1] );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );

      //Beta systematics are +-.5 (*10)
      systValue[0] = round(m_beta*10)-5;
      systValue[1] = round(m_beta*10)+5;
      m_sysName.push_back("MJB_b"+to_string(systValue[0])+"__1down" );   m_sysTool.push_back( 3 ); m_sysDetail.push_back( systValue[0] );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
      m_sysName.push_back("MJB_b"+to_string(systValue[1])+"__1up" );   m_sysTool.push_back( 3 ); m_sysDetail.push_back( systValue[1] );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );

      //pt Asymmetry systematics are +-.1 (*100)
      systValue[0] = round(m_ptAsym*100)-10;
      systValue[1] = round(m_ptAsym*100)+10;
      m_sysName.push_back("MJB_pta"+to_string(systValue[0])+"__1down" );   m_sysTool.push_back( 4 ); m_sysDetail.push_back( systValue[0] );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
      m_sysName.push_back("MJB_pta"+to_string(systValue[1])+"__1up" );   m_sysTool.push_back( 4 ); m_sysDetail.push_back( systValue[1] );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );

      //pt threshold systematics are +- 5
      systValue[0] = round(m_ptThresh)-5;
      systValue[1] = round(m_ptThresh)+5;
      m_sysName.push_back("MJB_ptt"+to_string(systValue[0])+"__1down" );   m_sysTool.push_back( 5 ); m_sysDetail.push_back( systValue[0] );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
      m_sysName.push_back("MJB_ptt"+to_string(systValue[1])+"__1up" );   m_sysTool.push_back( 5 ); m_sysDetail.push_back( systValue[1] );
      m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );

//      if (m_MJBIteration > 0 && m_MJBStatsOn){
//        m_sysName.push_back("MJB_stat1__1down"); m_sysTool.push_back( 6 ); m_sysDetail.push_back( 1  );
//        m_sysSet.push_back( CP::SystematicSet() ); m_sysSet.back().insert( CP::SystematicVariation("") );
//      }

    }

  }//for varVector
  return EL::StatusCode::SUCCESS;
}


// initialize and configure the JVT correction tool
EL::StatusCode MultijetBalanceAlgo :: loadJVTTool(){
  if(m_debug) Info("loadJVTTool", "loadJVTTool");

  //// Set up tagger tool to update JVT value after calibration
  if( !m_JVTUpdateTool_handle.isUserConfigured() ){
    ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JVTUpdateTool_handle, JetVertexTaggerTool) );
    ANA_CHECK( m_JVTUpdateTool_handle.setProperty("JVTFileName","JetMomentTools/JVTlikelihood_20140805.root") );
    ANA_CHECK( m_JVTUpdateTool_handle.retrieve() );
  }

  if( !m_JetJVTEfficiencyTool_handle.isUserConfigured() ){
    ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JetJVTEfficiencyTool_handle, CP::JetJvtEfficiency) );
    ANA_CHECK( m_JetJVTEfficiencyTool_handle.setProperty("WorkingPoint", m_JVTWP) );
    ANA_CHECK( m_JetJVTEfficiencyTool_handle.retrieve() );
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetBalanceAlgo :: loadJetCalibrationTool(){

  if(m_debug) Info("loadJetCalibrationTool", "loadJetCalibrationTool");

  if( !m_JetCalibrationTool_handle.isUserConfigured() ){
    ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JetCalibrationTool_handle, JetCalibrationTool) );
    ANA_CHECK( m_JetCalibrationTool_handle.setProperty("JetCollection", m_jetDef) );
    ANA_CHECK( m_JetCalibrationTool_handle.setProperty("ConfigFile", m_jetCalibConfig) );
    ANA_CHECK( m_JetCalibrationTool_handle.setProperty("CalibSequence", m_jetCalibSequence) );
    ANA_CHECK( m_JetCalibrationTool_handle.setProperty("IsData", !m_isMC) );
    ANA_CHECK( m_JetCalibrationTool_handle.retrieve() );
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

EL::StatusCode MultijetBalanceAlgo :: setupJetCalibrationStages() {

  if(m_debug) Info("setupJetCalibrationStages", "setupJetCalibrationStages");
  // Setup calibration stages tools //
  // Create a map from the CalibSequence string components to the xAOD aux data
  std::map <std::string, std::string> JCSMap;
  if (m_inContainerName.find("EMTopo") != std::string::npos || m_inContainerName.find("EMPFlow") != std::string::npos)
    JCSMap["RAW"] = "JetEMScaleMomentum";
  else if( m_inContainerName.find("LCTopo") != std::string::npos )
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

EL::StatusCode MultijetBalanceAlgo :: loadJetCleaningTool(){
  if(m_debug) Info("loadJetCleaningTool", "loadJetCleaningTool");

  if( !m_JetCleaningTool_handle.isUserConfigured() ){
    ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JetCleaningTool_handle, JetCleaningTool) );
    ANA_CHECK( m_JetCleaningTool_handle.setProperty("CutLevel", m_jetCleanCutLevel) );
    if (m_jetCleanUgly){
      ANA_CHECK( m_JetCleaningTool_handle.setProperty("DoUgly", true) );
    }

    ANA_CHECK( m_JetCleaningTool_handle.retrieve() );
  }

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetBalanceAlgo :: loadJetUncertaintyTool(){
  if(m_debug) Info("loadJetUncertaintyTool()", "loadJetUncertaintyTool");

  if( !m_JetUncertaintiesTool_handle.isUserConfigured() ){
    ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JetUncertaintiesTool_handle, JetUncertaintiesTool) );
    ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("JetDefinition", m_jetDef) );
//    ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("ConfigFile", m_jetUncertaintyConfig) );
    if( m_isAFII ){
      ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("MCType", "AFII") );
    }else{
      ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("MCType", "MC15") );
    }

    ANA_CHECK( m_JetUncertaintiesTool_handle.retrieve() );
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetBalanceAlgo :: loadJetResolutionTool(){
  if(m_debug) Info("loadJetResolutionTool", "loadJetResolutionTool");

  // Instantiate the JER Uncertainty tool
  m_JERTool_handle.setTypeAndName("JERTool/JERTool_" + m_name);
  if( !m_JERTool_handle.isUserConfigured() ){
    ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JERTool_handle, JERTool) );
    ANA_CHECK( m_JERTool_handle.setProperty("PlotFileName", m_JERUncertaintyConfig.c_str()) );
    ANA_CHECK( m_JERTool_handle.setProperty("CollectionName", m_jetDef) );
    ANA_CHECK( m_JERTool_handle.retrieve() );
  }

  // Instantiate the JER Smearing tool
  m_JERSmearingTool_handle.setTypeAndName("JERSmearingTool/JERSmearingTool_" + m_name);
  if( !m_JERSmearingTool_handle.isUserConfigured() ){
    ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JERSmearingTool_handle, JERSmearingTool) );
    ANA_CHECK( m_JERSmearingTool_handle.setProperty("JERTool", m_JERTool_handle.getHandle()) );
    ANA_CHECK( m_JERSmearingTool_handle.setProperty("isMC", m_isMC) );
    ANA_CHECK( m_JERSmearingTool_handle.setProperty("ApplyNominalSmearing", m_JERApplySmearing) );
    ANA_CHECK( m_JERSmearingTool_handle.setProperty("SystematicMode", m_JERSystematicMode) );
    ANA_CHECK( m_JERSmearingTool_handle.retrieve() );
  }

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetBalanceAlgo :: loadJetTileCorrectionTool(){
  if(m_debug) Info("loadJetTileCorrectionTool()", "loadJetTileCorrectionTool");

  if( !m_JetTileCorrectionTool_handle.isUserConfigured() ){
    ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JetTileCorrectionTool_handle, CP::JetTileCorrectionTool) );
    //m_JetTileCorrectionTool_handle.setProperty("CorrectionFileName","JetTileCorrection/JetTile_pFile_010216.root");

    ANA_CHECK( m_JetTileCorrectionTool_handle.retrieve() );
  }

  return EL::StatusCode::SUCCESS;
}

//Setup V+jet calibration and systematics files
EL::StatusCode MultijetBalanceAlgo :: loadVjetCalibration(){
  if(m_debug) Info("loadVjetCalibration()", "loadVjetCalibration");

  TFile *VjetFile = TFile::Open( gSystem->ExpandPathName(m_VjetCalibFile.c_str()) , "READ" );

  //Only retrieve Nominal container
  std::string VjetHistName = m_jetDef+"_correction";
  m_VjetHist = (TH1F*) VjetFile->Get( VjetHistName.c_str() );
  if(!m_VjetHist){
    Error("loadVjetCalibration", "Could not load Vjet histogram %s for file %s", VjetHistName.c_str(), gSystem->ExpandPathName(m_VjetCalibFile.c_str()));
    return EL::StatusCode::FAILURE;
  }

  m_VjetHist->SetDirectory(0); //Detach histogram from file to memory
  VjetFile->Close();

  //// Find out the subleading jet pt cut for V+jets ////
  if( m_subjetThreshold.at(0) == -1000 ){
    m_subjetThreshold.at(0) = m_VjetHist->GetXaxis()->GetBinUpEdge( m_VjetHist->GetNbinsX() ) * 1e3;
    //float lastCalibFactor = m_VjetHist->GetBinContent( m_VjetHist->GetNbinsX() );
    //m_subjetThreshold.at(0) = floor( m_subjetThreshold.at(0)*(1./lastCalibFactor) );
    Info("loadVjetCalibration", "Setting first subleading jet pt threshold to %f, taken from the V+jet configuration file", m_subjetThreshold.at(0));
  }

  return EL::StatusCode::SUCCESS;
}

//Setup Previous MJB calibration and systematics files
EL::StatusCode MultijetBalanceAlgo :: loadMJBCalibration(){
  if(m_debug) Info("loadMJBCalibration()", "loadMJBCalibration");

  if(m_MJBIteration == 0 && !m_closureTest)
    return EL::StatusCode::SUCCESS;

  if( m_isMC )
    return EL::StatusCode::SUCCESS;

  // Load the file.  If m_closureTest, then apply the current iteration
  TFile* MJBFile = TFile::Open( gSystem->ExpandPathName( ("$ROOTCOREBIN/data/MultijetBalance/"+m_MJBCorrectionFile).c_str() ), "READ" );
  if(m_debug) Info("loadMJBCalibration", "Loaded MJB input file");
  if (m_closureTest)
    m_ss << m_MJBIteration;
  else
    m_ss << (m_MJBIteration-1);

  std::string mjbIterPrefix = "Iteration"+m_ss.str()+"_";
  std::string histPrefix = "DoubleMJB";

  m_ss.str( std::string() );

  // Get the MJB correction histograms in the input file
  TKey *key;
  TIter next(MJBFile->GetListOfKeys());
  while ((key = (TKey*) next() )){
    std::string dirName = key->GetName();
    if (dirName.find( mjbIterPrefix ) != std::string::npos) { //If it's a Iteration Dir
      TH1D *MJBHist;
      MJBHist = (TH1D*) MJBFile->Get( (dirName+"/"+histPrefix).c_str() );
      //Remove Iteration part of name
      std::string newHistName = dirName.substr(11);
      if( newHistName.find("MCType") == std::string::npos ){   
        MJBHist->SetName( newHistName.c_str() );
        MJBHist->SetDirectory(0);
        m_MJBHists.push_back(MJBHist);
      }
    }
  }

  // Fix the systematics mappings to match the input MJB corrections
  std::vector<std::string> new_sysName;
  std::vector<int> new_sysTool;
  std::vector<int> new_sysDetail;

  int foundCount = 0;
  // Loop over each input MJB systematic histogram
  for(unsigned int i=0; i < m_MJBHists.size(); ++i){
    bool foundMatch = false;
    std::string histName = m_MJBHists.at(i)->GetName();

    //Add the MCType systematic if it's in the input, as this isn't added regularly
    if( histName.find("MCType") != std::string::npos ){
      new_sysName.push_back( histName );
      new_sysTool.push_back( m_sysTool.at(m_NominalIndex) ); //Treat like nominal calibration
      new_sysDetail.push_back( m_sysDetail.at(m_NominalIndex) );
      foundMatch = true;
    } else {
      // Loop over the loaded systematics and find the match
      for(unsigned int iVar=0; iVar < m_sysName.size(); ++iVar){
        if( histName.find( m_sysName.at(iVar) ) != std::string::npos ){
          new_sysName.push_back( histName );
          new_sysTool.push_back( m_sysTool.at(iVar) );
          new_sysDetail.push_back( m_sysDetail.at(iVar) );
          foundMatch = true;
          break;
        }
      }//for m_sysName
    }// if searching m_sysName
    foundCount++;
    if( foundMatch == false){
        Error("loadMJBCalibration()", "Can't find Systematic Variation corresponding to MJB Correction %s. Exiting...", histName.c_str() );
        return EL::StatusCode::FAILURE;
    }
  }//for m_MJBHists

  //Fix the m_NominalIndex
  for(unsigned int i=0; i < m_MJBHists.size(); ++i){
    std::string histName = m_MJBHists.at(i)->GetName();
    if( histName.find("Nominal") != std::string::npos){
      m_NominalIndex = i;
      break;
    }
  }//for m_MJBHists

  //Replace systematic vectors with their new versions
  m_sysName    =  new_sysName;
  m_sysTool   =  new_sysTool;
  m_sysDetail =  new_sysDetail;

  Info("loadMJBCalibration()", "Succesfully loaded MJB calibration file");

  MJBFile->Close();

return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetBalanceAlgo :: applyJetCalibrationTool( std::vector< xAOD::Jet*>* jets ){

  if(m_debug) Info("applyJetCalibrationTool()", "applyJetCalibrationTool");
  for(unsigned int iJet=0; iJet < jets->size(); ++iJet){
    if ( m_JetCalibrationTool_handle->applyCorrection( *(jets->at(iJet)) ) == CP::CorrectionCode::Error ) {
      Error("execute()", "JetCalibrationTool reported a CP::CorrectionCode::Error");
      return StatusCode::FAILURE;
    }
  }// for each jet
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MultijetBalanceAlgo :: applyJetUncertaintyTool( xAOD::Jet* jet , int iVar ){
  if(m_debug) Info("applyJetUncertaintyTool", "Systematic %s", m_sysName.at(iVar).c_str() );


   if( m_sysTool.at(iVar) != 0 ) //If not a JetUncertaintyTool sys variation
    return EL::StatusCode::SUCCESS;

//  std::cout << " for JES " << m_sysName.at(iVar) << " pt of " << jet->pt() << " to ";

  if ( m_JetUncertaintiesTool_handle->applySystematicVariation( m_sysSet.at(iVar) ) != CP::SystematicCode::Ok ) {
    Error("execute()", "Cannot configure JetUncertaintiesTool for systematic %s", m_sysName.at(iVar).c_str());
    return EL::StatusCode::FAILURE;
  }

  if ( m_JetUncertaintiesTool_handle->applyCorrection( *jet ) == CP::CorrectionCode::Error ) {
    Error("execute()", "JetUncertaintiesTool reported a CP::CorrectionCode::Error");
    return EL::StatusCode::FAILURE;
  }
//  std::cout << jet->pt() << std::endl;

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetBalanceAlgo :: applyJetResolutionTool( xAOD::Jet* jet , int iVar ){
  if(m_debug) Info("applyJetResolutionTool", "Systematic %s", m_sysName.at(iVar).c_str());

  if( ( m_sysTool.at(iVar) != 1 ) // If not a JetResolutionTool sys variation
      && !(m_JERApplySmearing && m_isMC)  )  //If not applying smearing to MC
    return EL::StatusCode::SUCCESS;

  // Get systematic set, or nominal for m_JERApplySmearing
  CP::SystematicSet thisSysSet;
  if ( m_sysTool.at(iVar) == 1 )
    thisSysSet = m_sysSet.at(iVar);
  else
    thisSysSet = m_sysSet.at( m_NominalIndex ); //an empty SystematicSet, equal to the nominal JER smearing


//  std::cout << " for JES " << m_sysName.at(iVar) << " pt of " << jet->pt() << " to ";
  if ( m_JERSmearingTool_handle->applySystematicVariation( thisSysSet ) != CP::SystematicCode::Ok ) {
    Error("execute()", "Cannot configure JetResolutionTool for systematic %s", m_sysName.at(iVar).c_str());
    return EL::StatusCode::FAILURE;
  }

  if ( m_JERSmearingTool_handle->applyCorrection( *jet ) == CP::CorrectionCode::Error ) {
    Error("execute()", "JetResolutionTool reported a CP::CorrectionCode::Error");
    return EL::StatusCode::FAILURE;
  }
//  std::cout << jet->pt() << std::endl;

  return EL::StatusCode::SUCCESS;
}


// Apply the Vjet Calibration to jets
EL::StatusCode MultijetBalanceAlgo :: applyVjetCalibration( std::vector< xAOD::Jet*>* jets ){
  if(m_debug) Info("applyVjetCalibration", "applyVjetCalibration ");

  if(m_isMC)   
    return EL::StatusCode::SUCCESS;

  for(unsigned int iJet=0; iJet < jets->size(); ++iJet){
 
    // Do not apply if leading jet or validation mode 
    if( iJet == 0 || m_validation )   continue;
    
    xAOD::Jet* jet = jets->at(iJet); 

    // Do not apply if not in V+jet correction range
    if( jet->pt()/1e3 < m_VjetHist->GetXaxis()->GetBinLowEdge(1)  ||
        jet->pt()/1e3 > m_VjetHist->GetXaxis()->GetBinUpEdge( m_VjetHist->GetNbinsX()+1 ) )
      continue;
 
    //Get nominal V+jet correction
    float thisCalibration = 1.;
    thisCalibration = m_VjetHist->GetBinContent( m_VjetHist->FindBin(jet->pt()/1e3) );
    if( thisCalibration == 0){
      Error("applyVjetCalibration", "Vjet calibration factor is 0!");
      return EL::StatusCode::FAILURE;
    }
    xAOD::JetFourMom_t thisJet;
    thisJet.SetCoordinates( jet->pt(), jet->eta(), jet->phi(), jet->m() );
    thisJet *= (1./thisCalibration);
    jet->setJetP4( thisJet );

  }


  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetBalanceAlgo :: applyMJBCalibration( xAOD::Jet* jet , int iVar ){
  if(m_debug) Info("applyMJBCalibration", "applyMJBCalibration ");

  if(m_isMC)
    return EL::StatusCode::SUCCESS;

  // Get calibration
  float thisCalibration = 1. / m_MJBHists.at(iVar)->GetBinContent( m_MJBHists.at(iVar)->FindBin(jet->pt()/1e3) );

  xAOD::JetFourMom_t thisJet;
  thisJet.SetCoordinates( jet->pt(), jet->eta(), jet->phi(), jet->m() );
  thisJet *= thisCalibration;
  jet->setJetP4( thisJet );

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetBalanceAlgo :: reorderJets( std::vector< xAOD::Jet*>* theseJets ){

  if(m_debug) Info("reorderJets()", "reorderJets ");
  xAOD::Jet* tmpJet = nullptr;
  for(unsigned int iJet = 0; iJet < theseJets->size(); ++iJet){
    for(unsigned int jJet = iJet+1; jJet < theseJets->size(); ++jJet){
      if( theseJets->at(iJet)->pt() < theseJets->at(jJet)->pt() ){
        tmpJet = theseJets->at(iJet);
        theseJets->at(iJet) = theseJets->at(jJet);
        theseJets->at(jJet) = tmpJet;
      }
    }//jJet
  }//iJet

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetBalanceAlgo::applyJetSysVariation(std::vector< xAOD::Jet*>* theseJets, int iSysVar ){
  if(m_debug) Info("applyJetSysVariation()", "applyJetSysVariation ");
  

  // Apply MJB iterative calibration when appropriate
  for(unsigned int iJet = 0; iJet < theseJets->size(); ++iJet){
    xAOD::Jet* jet = theseJets->at(iJet);

    // If validation mode, don't apply MJB calibrations
    if( m_validation )
      continue;

    // If closure test and leading jet
    // or if subleading jets above threshold
    if(  (m_closureTest && iJet==0)
      || (iJet > 0 && jet->pt() > m_subjetThreshold.at(0)) ){
      ANA_CHECK( applyMJBCalibration( jet , iSysVar ) );
    }

  }//MJB calibration
  
  
  //JetUncertainty comes after V+jet Calibration, as these files *should* have been generated using V+jet!
  for(unsigned int iJet = 0; iJet < theseJets->size(); ++iJet){
    xAOD::Jet* jet = theseJets->at(iJet);
      
    if( jet->pt() <= 16.*1e3)  // tool has issues below 16 GeV
      continue;

    // If validation mode
    // or if a subleading jet below threshold
    if(  m_validation 
      || (iJet > 0 && jet->pt() <= m_subjetThreshold.at(0)) ){

      ANA_CHECK( applyJetUncertaintyTool( jet , iSysVar ) );
      ANA_CHECK( applyJetResolutionTool( jet , iSysVar ) );
    }

  }// for each jet


  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetBalanceAlgo :: applyJetTileCorrectionTool( std::vector< xAOD::Jet*>* jets ){
  if(m_debug) Info("applyJetTileCorrectionTool()", "applyJetTileCorrectionTool");

  if( m_isMC )
    return EL::StatusCode::SUCCESS;

  for(unsigned int iJet=0; iJet < jets->size(); ++iJet){
  
    xAOD::Jet* jet = jets->at(iJet);
    xAOD::JetFourMom_t previous_P4;
    previous_P4.SetCoordinates( jet->pt(), jet->eta(), jet->phi(), jet->m() );

    if ( m_JetTileCorrectionTool_handle->applyCorrection( *jet ) == CP::CorrectionCode::Error ) {
      Error("execute()", "JetTileCorrectionTool reported a CP::CorrectionCode::Error");
      return StatusCode::FAILURE;
    }

    jet->auxdata< float >("TileCorrectedPt") = jet->pt();

    if(!m_TileCorrection)
      jet->setJetP4( previous_P4 );
  }//for jet


  return EL::StatusCode::SUCCESS;
}

//////////////////////////////// Selections //////////////////////////
bool MultijetBalanceAlgo:: cut_LeadEta(){
  if(m_debug) Info("execute()", "Lead jet detector eta");


  xAOD::JetFourMom_t jetConstituentP4 = m_jets->at(0)->getAttribute<xAOD::JetFourMom_t>("JetConstitScaleMomentum"); // or JetEMScaleMomentum
  m_jets->at(0)->auxdecor< float >( "detEta") = jetConstituentP4.eta();
  if( fabs(jetConstituentP4.eta()) > 1.2 )
    return false;
  else
    return true;
}

bool MultijetBalanceAlgo:: cut_JetEta(){
  
  for (unsigned int iJet = 1; iJet < m_jets->size(); ++iJet){
    xAOD::JetFourMom_t jetConstituentP4 = m_jets->at(iJet)->getAttribute<xAOD::JetFourMom_t>("JetConstitScaleMomentum"); // or JetEMScaleMomentum
    m_jets->at(iJet)->auxdecor< float >( "detEta") = jetConstituentP4.eta();
    if( fabs(jetConstituentP4.eta() > 2.8 ) ){
      m_jets->erase( m_jets->begin()+iJet );
      --iJet;
    }
  }// for m_jets
  
  if (m_jets->size() >= m_numJets)
    return true;
  else
    return false;

}

bool MultijetBalanceAlgo:: cut_MCCleaning(){
  if(m_debug) Info("execute()", "MC Cleaning ");

  if(m_useMCPileupCheck && m_isMC){
    float pTAvg = ( m_jets->at(0)->pt() + m_jets->at(1)->pt() ) / 2.0;
    if( m_truthJets->size() == 0 || (pTAvg / m_truthJets->at(0)->pt() > 1.4) ){
      return false;
    }
  }

  return true;
}

bool MultijetBalanceAlgo:: cut_SubPt(){

  if( m_jets->at(1)->pt() > m_subjetThreshold.at(m_MJBIteration) )
    return false;
  else
    return true;

}

bool MultijetBalanceAlgo:: cut_JetPtThresh() {
//bool MultijetBalanceAlgo:: cut_JetPtThresh(std::vector< xAOD::Jet*>* m_jets) const {

  if(m_debug) Info("execute()", "Pt threshold ");
  for (unsigned int iJet = 0; iJet < m_jets->size(); ++iJet){
    if( m_jets->at(iJet)->pt()/1e3 < m_ptThresh ){ //Default 25 GeV
      m_jets->erase(m_jets->begin()+iJet);
      --iJet;
    }
  }
  
  if (m_jets->size() < m_numJets)
    std::cout << "Failing pt threshold!" << std::endl;
  
  if (m_jets->size() >= m_numJets)
    return true;
  else
    return false;

}

bool MultijetBalanceAlgo:: cut_JVT() {

  if(m_debug) Info("execute()", "Apply JVT ");
  for(unsigned int iJet = 0; iJet < m_jets->size(); ++iJet){
    m_jets->at(iJet)->auxdata< float >("Jvt") = m_JVTUpdateTool_handle->updateJvt( *(m_jets->at(iJet)) );
    if( !m_JetJVTEfficiencyTool_handle->passesJvtCut( *(m_jets->at(iJet)) ) ){
      m_jets->erase(m_jets->begin()+iJet);  --iJet;
    }
  }

  if (m_jets->size() >= m_numJets)
    return true;
  else
    return false;

}

//// Ignore event if any of the used jets are not clean ////
bool MultijetBalanceAlgo:: cut_CleanJet(){
  if(m_debug) Info("execute()", "Jet Cleaning ");
  for(unsigned int iJet = 0; iJet < m_jets->size(); ++iJet){
    if(! m_JetCleaningTool_handle->keep( *(m_jets->at(iJet))) ){
      return false;
    }//clean jet
  }
 
  return true; 

}

bool MultijetBalanceAlgo:: cut_TriggerEffRecoil(){
  ///// Trigger Efficiency and prescale /////

  m_prescale = 1.;
  bool passedTriggers = false;
  if (m_triggers.size() == 0)
    passedTriggers = true;

  static SG::AuxElement::Decorator< std::vector< std::string > >  passTrigs("passTriggers");
  static SG::AuxElement::Decorator< std::vector< float > >        trigPrescales("triggerPrescales");
  std::vector< std::string > passingTriggers = passTrigs( *m_eventInfo );

  for( unsigned int iT=0; iT < m_triggers.size(); ++iT){
    if( m_recoilTLV.Pt() > m_triggerThresholds.at(iT)){
      auto trigIndex = std::find(passingTriggers.begin(), passingTriggers.end(), m_triggers.at(iT));
      if( trigIndex !=  passingTriggers.end() ){ 
        passedTriggers = true;
        m_prescale = trigPrescales( *m_eventInfo ).at( trigIndex-passingTriggers.begin() );
      }
      break; // only check 1 trigger in correct pt range
    }//recoil Pt
  } // each Trigger

  if( passedTriggers ){
    return true;
  } else{
    return false;
  }

}
bool MultijetBalanceAlgo:: cut_PtAsym(){
  //Remove dijet events, i.e. events where subleading jet dominates the recoil jets
  if(m_debug) Info("execute()", "Pt asym selection ");
  double ptAsym = m_jets->at(1)->pt() / m_recoilTLV.Pt();
  m_eventInfo->auxdecor< float >( "ptAsym" ) = ptAsym;
  if( ptAsym > m_ptAsym ){ //Default 0.8
    return false;
  }

  return true;
}

bool MultijetBalanceAlgo:: cut_Alpha(){
  //Alpha is phi angle between leading jet and recoilJet system
  if(m_debug) Info("execute()", "Alpha Selection ");
  double alpha = fabs(DeltaPhi( m_jets->at(0)->phi(), m_recoilTLV.Phi() )) ;
  m_eventInfo->auxdecor< float >( "alpha" ) = alpha;
  if( (M_PI-alpha) > m_alpha ){  //0.3 by default
    return false;
  }
  return true;
}

bool MultijetBalanceAlgo:: cut_Beta(){
  //Beta is phi angle between leading jet and each other passing jet
  if(m_debug) Info("execute()", "Beta Selection ");
  double smallestBeta=10., avgBeta = 0., thisBeta=0.;
  for(unsigned int iJet=1; iJet < m_jets->size(); ++iJet){
    thisBeta = DeltaPhi(m_jets->at(iJet)->phi(), m_jets->at(0)->phi() );
    if( !m_looseBetaCut && (thisBeta < smallestBeta) ) 
      smallestBeta = thisBeta;
    else if( m_looseBetaCut && (thisBeta < smallestBeta) && (m_jets->at(iJet)->pt() > m_jets->at(0)->pt()*0.1) )
      smallestBeta = thisBeta;
    avgBeta += thisBeta;
    m_jets->at(iJet)->auxdecor< float >( "beta") = thisBeta;
  }
  avgBeta /= (m_jets->size()-1);
  m_eventInfo->auxdecor< float >( "avgBeta" ) = avgBeta;

  if( smallestBeta < m_beta ){ 
    return false;
  }

  return true;
}
