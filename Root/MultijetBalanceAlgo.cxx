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


// ROOT include(s):
#include "TFile.h"
#include "TEnv.h"
#include "TSystem.h"
#include "TKey.h"

// c++ includes(s):
#include <iostream>
#include <fstream>

// package include(s):
#include <xAODAnaHelpers/tools/ReturnCheck.h>
#include <xAODAnaHelpers/tools/ReturnCheckConfig.h>
#include <MultijetBalance/MultijetBalanceAlgo.h>
#include <xAODAnaHelpers/HelperFunctions.h>
#include <MultijetBalance/MultijetHists.h>

// external tools include(s):
#include "JetCalibTools/JetCalibrationTool.h"
#include "JetUncertainties/JetUncertaintiesTool.h"
#include "JetSelectorTools/JetCleaningTool.h"
#include "JetMomentTools/JetVertexTaggerTool.h"
#include "JetJvtEfficiency/JetJvtEfficiency.h"

#include "xAODBTaggingEfficiency/BTaggingSelectionTool.h"
#include "xAODBTaggingEfficiency/BTaggingEfficiencyTool.h"

#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

#include "SystTool/SystContainer.h"


using namespace std;

// this is needed to distribute the algorithm to the workers
ClassImp(MultijetBalanceAlgo)


MultijetBalanceAlgo :: MultijetBalanceAlgo (std::string name) :
  Algorithm(),
  m_name(name),
  m_JetCalibrationTool_handle("JetCalibrationTool/JetCalibrationTool_"+name),
  m_JetUncertaintiesTool_handle("JetUncertaintiesTool/JetUncertaintiesTool_"+name),
  m_JetCleaningTool_handle("JetCleaningTool/JetCleaningTool_"+name),
  m_JVTUpdateTool_handle("JetVertexTaggerTool/JVTUpdateTool_"+name),
  m_JetJVTEfficiencyTool_handle("CP::JetJVTEfficiency/JBTEfficiencyTool_"+name)
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
  m_beta = 0.6;
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
  m_bTagWPsString = "77,85";


  //config for Jet Tools
  m_jetDef = "";
  m_jetCalibSequence = "";
  m_jetCalibConfig = "";
  m_jetCleanCutLevel = "";
  m_jetCleanUgly = false;
  m_JVTWP = "Medium";
  m_jetUncertaintyConfig = "";

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
    m_triggerThresholds.push_back( std::stof(thisSubStr.substr(thisSubStr.find_first_of(':')+1, thisSubStr.size()) , &sz) *GeV );
  }

  // Save subleading pt thresholds to use
  std::stringstream ss2(m_MJBIterationThreshold);
  while( std::getline(ss2, thisSubStr, ',')) {
    m_subleadingPtThreshold.push_back( std::stof(thisSubStr, &sz) * GeV );
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
    m_subleadingPtThreshold.at(0) = 999999.;
  }// if m_validation

  // Setup bootstrap depending upon iteration
  m_iterateBootstrap = false;
  if( m_bootstrap && m_MJBIteration > 0){
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
  if( !m_VjetCalib && (m_subleadingPtThreshold.at(0) == -1000) ){
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



EL::StatusCode MultijetBalanceAlgo :: changeInput (bool firstFile)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



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
  RETURN_CHECK("init", HelperFunctions::retrieve(eventInfo, "EventInfo", m_event, m_store), "Failed to retrieve EventInfo");
  m_isMC = ( eventInfo->eventType( xAOD::EventInfo::IS_SIMULATION ) ) ? true : false;

  EL_RETURN_CHECK("init:configure", this->configure() );

  EL_RETURN_CHECK("init", getLumiWeights(eventInfo) );

  // load all variations
  setupJetCalibrationStages();
  EL_RETURN_CHECK("init", loadJetUncertaintyTool());
  EL_RETURN_CHECK("init", loadVariations());

  EL_RETURN_CHECK("init", loadTriggerTool());
  //load Calibration and systematics files
  EL_RETURN_CHECK("init", loadJetCalibrationTool());
  EL_RETURN_CHECK("init", loadJetCleaningTool());
  EL_RETURN_CHECK("init", loadJVTTool());
  EL_RETURN_CHECK("init", loadBTagTools());
  if (m_VjetCalib)
    EL_RETURN_CHECK("init", loadVjetCalibration());

  EL_RETURN_CHECK("init", loadMJBCalibration());

  if( m_bootstrap ){
    systTool = new SystContainer(m_sysVar, m_bins, m_systTool_nToys);
  }

  for(unsigned int iVar=0; iVar < m_sysVar.size(); ++iVar){
    cout << iVar << " var is " << m_sysVar.at(iVar) << " at index " << m_sysToolIndex.at(iVar) << endl;
  }

  if(m_useCutFlow) {
    Info("initialize()", "Setting Cutflow");

    //std::string newName;
    TFile *file = wk()->getOutputFile ("cutflow");
    TH1D* origCutflowHist = (TH1D*)file->Get("cutflow");
    TH1D* origCutflowHistW = (TH1D*)file->Get("cutflow_weighted");

    m_cutflowFirst = origCutflowHist->GetXaxis()->FindBin("njets");
    origCutflowHistW->GetXaxis()->FindBin("njets");
    origCutflowHist->GetXaxis()->FindBin( "centralLead");
    origCutflowHistW->GetXaxis()->FindBin("centralLead");
    origCutflowHist->GetXaxis()->FindBin( "detEta");
    origCutflowHistW->GetXaxis()->FindBin("detEta");
    origCutflowHist->GetXaxis()->FindBin( "mcCleaning");
    origCutflowHistW->GetXaxis()->FindBin("mcCleaning");
    origCutflowHist->GetXaxis()->FindBin( "ptSub");
    origCutflowHistW->GetXaxis()->FindBin("ptSub");
    origCutflowHist->GetXaxis()->FindBin( "ptThreshold");
    origCutflowHistW->GetXaxis()->FindBin("ptThreshold");
    origCutflowHist->GetXaxis()->FindBin( "JVT");
    origCutflowHistW->GetXaxis()->FindBin("JVT");
    origCutflowHist->GetXaxis()->FindBin( "cleanJet");
    origCutflowHistW->GetXaxis()->FindBin("cleanJet");
    origCutflowHist->GetXaxis()->FindBin( "TriggerEff");
    origCutflowHistW->GetXaxis()->FindBin("TriggerEff");
    origCutflowHist->GetXaxis()->FindBin( "ptAsym");
    origCutflowHistW->GetXaxis()->FindBin("ptAsym");
    origCutflowHist->GetXaxis()->FindBin( "alpha");
    origCutflowHistW->GetXaxis()->FindBin("alpha");
    origCutflowHist->GetXaxis()->FindBin( "beta");
    origCutflowHistW->GetXaxis()->FindBin("beta");

    //Add a cutflow for each variation
    for(unsigned int iVar=0; iVar < m_sysVar.size(); ++iVar){
      m_cutflowHist.push_back( (TH1D*) origCutflowHist->Clone() );
      m_cutflowHistW.push_back( (TH1D*) origCutflowHistW->Clone() );
      m_cutflowHist.at(iVar)->SetName( ("cutflow_"+m_sysVar.at(iVar)).c_str() );
      m_cutflowHistW.at(iVar)->SetName( ("cutflow_weighted_"+m_sysVar.at(iVar)).c_str() );
      m_cutflowHist.at(iVar)->SetTitle( ("cutflow_"+m_sysVar.at(iVar)).c_str() );
      m_cutflowHistW.at(iVar)->SetTitle( ("cutflow_weighted_"+m_sysVar.at(iVar)).c_str() );
      m_cutflowHist.at(iVar)->SetDirectory( file );
      m_cutflowHistW.at(iVar)->SetDirectory( file );

      //Need to retroactively fill original bins of these histograms
      for(unsigned int iBin=1; iBin < m_cutflowFirst; ++iBin){
        m_cutflowHist.at(iVar)->SetBinContent(iBin, origCutflowHist->GetBinContent(iBin) );
        m_cutflowHistW.at(iVar)->SetBinContent(iBin, origCutflowHistW->GetBinContent(iBin) );
      }//for iBin
    }//for each m_sysVar
  } //m_useCutflow


  //Add output hists for each variation
  m_ss << m_MJBIteration;
  for(unsigned int iVar=0; iVar < m_sysVar.size(); ++iVar){
    std::string histOutputName = "Iteration"+m_ss.str()+"_"+m_sysVar.at(iVar);

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
    for(int unsigned iVar=0; iVar < m_sysVar.size(); ++iVar){
      //cout << "iVar/Nominal " << iVar << " " << m_NominalIndex << endl;
      if (m_writeNominalTree && (int) iVar != m_NominalIndex)
        continue;

      TTree * outTree = new TTree( ("outTree_"+m_sysVar.at(iVar)).c_str(), ("outTree_"+m_sysVar.at(iVar) ).c_str());
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
//      m_treeList.at(iTree)->AddMJB(m_MJBDetailStr);
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

  //const xAOD::EventInfo* eventInfo = HelperFunctions::getContainer<xAOD::EventInfo>("EventInfo", m_event, m_store);
  const xAOD::EventInfo* eventInfo = 0;
  RETURN_CHECK("exec", HelperFunctions::retrieve(eventInfo, "EventInfo", m_event, m_store), "Failed to retrieve EventInfo");
  m_mcEventWeight = (m_isMC ? eventInfo->mcEventWeight() : 1.) ;

  //const xAOD::VertexContainer* vertices = HelperFunctions::getContainer<xAOD::VertexContainer>("PrimaryVertices", m_event, m_store);;
  const xAOD::VertexContainer* vertices = 0;
  RETURN_CHECK("exec", HelperFunctions::retrieve(vertices, "PrimaryVertices", m_event, m_store), "Failed to retrieve PrimaryVertices");
  m_pvLocation = HelperFunctions::getPrimaryVertexLocation( vertices );  //Get primary vertex for JVF cut

  //const xAOD::JetContainer* inJets = HelperFunctions::getContainer<xAOD::JetContainer>(m_inContainerName, m_event, m_store);
  const xAOD::JetContainer* inJets = 0;
  RETURN_CHECK("exec", HelperFunctions::retrieve(inJets, m_inContainerName, m_event, m_store), "Failed to retrieve JetContainer");

  const xAOD::JetContainer* truthJets = 0;
  if(m_useMCPileupCheck && m_isMC){
    //truthJets = HelperFunctions::getContainer<xAOD::JetContainer>(m_MCPileupCheckContainer, m_event, m_store);
    RETURN_CHECK("exec", HelperFunctions::retrieve(truthJets, m_MCPileupCheckContainer, m_event, m_store), "Failed to retrieve truthJets");
  }

  //// Pass njets ////
  if(inJets->size() < m_numJets){
    wk()->skipEvent();  return EL::StatusCode::SUCCESS;
  }
  passCutAll(); //njets

  /////Create an editable shallow copy and a removable container //////
  std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > originalSignalJetsSC = xAOD::shallowCopyContainer( *inJets );


  std::vector< xAOD::Jet*>* originalSignalJets = new std::vector< xAOD::Jet* >();
  for( auto thisJet : *(originalSignalJetsSC.first) ) {
     originalSignalJets->push_back( thisJet );
   }


  ////////////////////////////////// Apply Calibrations, store jetCorr ////////////////////////////////////////
  if(m_debug) Info("execute()", "Get Raw Kinematics ");
  vector<TLorentzVector> rawJetKinematics;
  for (unsigned int iJet = 0; iJet < originalSignalJets->size(); ++iJet){
    TLorentzVector thisJet;
    thisJet.SetPtEtaPhiE(originalSignalJets->at(iJet)->pt(), originalSignalJets->at(iJet)->eta(), originalSignalJets->at(iJet)->phi(), originalSignalJets->at(iJet)->e());
    rawJetKinematics.push_back(thisJet);
  }

  if(m_debug) Info("execute()", "Apply Jet Calibration Tool ");
  for(unsigned int iJet=0; iJet < originalSignalJets->size(); ++iJet){
//    EL_RETURN_CHECK("execute", applyJetCalibrationTool( originalSignalJets->at(iJet) ) );
    m_JetCalibrationTool_handle->applyCorrection( *(originalSignalJets->at(iJet)) );
    originalSignalJets->at(iJet)->auxdecor< float >( "jetCorr") = originalSignalJets->at(iJet)->pt() / rawJetKinematics.at(iJet).Pt() ;
  }
  reorderJets( originalSignalJets );

  /////////////////////////// Begin Selections and Creation of Variables ///////////////////////////////
  if(m_debug) Info("execute()", "Begin Selections");

  if(m_debug) Info("execute()", "Detector Eta checks");
  for(unsigned int iJet=0; iJet < originalSignalJets->size(); ++iJet){
    xAOD::JetFourMom_t jetConstituentP4 = originalSignalJets->at(iJet)->getAttribute<xAOD::JetFourMom_t>("JetConstitScaleMomentum"); // or JetEMScaleMomentum
    originalSignalJets->at(iJet)->auxdecor< float >( "detEta") = jetConstituentP4.eta();
  }

  if( fabs(originalSignalJets->at(0)->auxdecor< float >("detEta")) > 1.2 ) {
    delete originalSignalJetsSC.first; delete originalSignalJetsSC.second; delete originalSignalJets;
    wk()->skipEvent();  return EL::StatusCode::SUCCESS;
  }
  passCutAll(); //centralLead

  for(unsigned int iJet=1; iJet < originalSignalJets->size(); ++iJet){
    if( fabs(originalSignalJets->at(iJet)->auxdecor< float >("detEta")) > 2.8){
      originalSignalJets->erase(originalSignalJets->begin()+iJet);
      --iJet;
    }
  }
  if (originalSignalJets->size() < m_numJets){
    delete originalSignalJetsSC.first; delete originalSignalJetsSC.second; delete originalSignalJets;
    wk()->skipEvent();  return EL::StatusCode::SUCCESS;
  }
  passCutAll(); //detEta

  if(m_debug) Info("execute()", "MC Cleaning ");
  //// mcCleaning ////  We will likely remove this one!
  if(m_useMCPileupCheck && m_isMC){
    float pTAvg = ( originalSignalJets->at(0)->pt() + originalSignalJets->at(1)->pt() ) /2.0;
    if( truthJets->size() == 0 || (pTAvg / truthJets->at(0)->pt() > 1.4) ){
      delete originalSignalJetsSC.first; delete originalSignalJetsSC.second; delete originalSignalJets;
      wk()->skipEvent();  return EL::StatusCode::SUCCESS;
    }
  }
  passCutAll(); //mcCleaning

  if(m_debug) Info("execute()", "Subleading jet pt cut ");
  //The insitu calibrations are derived on calibrated pts, and assumed to be ~ uncalibrated pt, so that correction is applied to uncalibrated pt.
  //The subleading cut must be applied to uncalibrated pt, otherwise we get behavior like 1200->1250 pt (good and keep), 1201->1201 pt (bad but keep).
  //We want to delete events with 1201 pt jets in this situation
  //This also work for when calibration makes pt go down.
  //
  //Technically this causes down and up syst. variations to have different pt selections on the final objects.  
  //i.e. on 950 uncalibrated, up variation may cut on 990 calibrated while down variation cuts on 980 calibrated. 
  //No easy way to avoid this, and disjoint-ness in systematic variations is more palatable
  //
  //TODO : This needs to be changed in case m_leadingGSC is chosen!!
  if( originalSignalJets->at(1)->pt() > m_subleadingPtThreshold.at(m_MJBIteration) ){
    delete originalSignalJetsSC.first; delete originalSignalJetsSC.second; delete originalSignalJets;
    wk()->skipEvent();  return EL::StatusCode::SUCCESS;
  }

  // The following code will manually edit jet 4-vectors in various ways.
  // Save the original kinematics of all jets to be reapplied after each loop
  vector<TLorentzVector> originalJetKinematics;
  for (unsigned int iJet = 0; iJet < originalSignalJets->size(); ++iJet){
    TLorentzVector thisJet;
    thisJet.SetPtEtaPhiE(originalSignalJets->at(iJet)->pt(), originalSignalJets->at(iJet)->eta(), originalSignalJets->at(iJet)->phi(), originalSignalJets->at(iJet)->e());
    originalJetKinematics.push_back(thisJet);
  }
  passCutAll(); //ptSub

  int m_cutflowFirst_SystLoop = m_iCutflow; //Get cutflow position for systematic looping
  //A vector of temporary jet objects whose 4-vectors can be edited.
  //This will be reset after each systematic loop.
  vector< xAOD::Jet*>* signalJets = new std::vector< xAOD::Jet* >();

  float alphaCut, betaCut, ptAsymCut, ptThresholdCut;
  for(unsigned int iVar=0; iVar < m_sysVar.size(); ++iVar){

    if(m_debug) Info("execute()", "Starting variation %i %s", iVar, m_sysVar.at(iVar).c_str());
    
    //Reset standard values
    *signalJets = *originalSignalJets;
    m_iCutflow = m_cutflowFirst_SystLoop;
    alphaCut = m_alpha; //0.3
    betaCut = m_beta; 
    ptAsymCut = m_ptAsym;
    ptThresholdCut = m_ptThresh*GeV;

    //Set relevant MJB variations for this iVar
    if( m_sysTool.at(iVar) == 2 ){ //alpha
      alphaCut = (double) m_sysToolIndex.at(iVar)  / 100.;
    }else if( m_sysTool.at(iVar) == 3 ){ //beta
      betaCut = (double) m_sysToolIndex.at(iVar)  / 10.;
    }else if( m_sysTool.at(iVar) == 4 ){ //pta
      ptAsymCut = (double) m_sysToolIndex.at(iVar)  / 100.;
    }else if( m_sysTool.at(iVar) == 5 ){ //ptt
      ptThresholdCut = (double) m_sysToolIndex.at(iVar) * GeV;
    }
    
    if(m_debug) Info("execute()", "Set calibration level for all jets");
    // Must reset jet kinematics for each loop of systematic variations
    for (unsigned int iJet = 0; iJet < signalJets->size(); ++iJet){

      /////////////////// Get calibration stage of the jet /////////////////////////
      if(m_sysTool.at(iVar) == 1){
        // A systematic variation that lets us use any jet calibration stage for all jets
        int iCalibStage = m_sysToolIndex.at(iVar);
        xAOD::JetFourMom_t jetCalibStageCopy = signalJets->at(iJet)->getAttribute<xAOD::JetFourMom_t>( m_JCSStrings.at(iCalibStage).c_str() );
        signalJets->at(iJet)->auxdata< float >("pt") = jetCalibStageCopy.Pt();
        signalJets->at(iJet)->auxdata< float >("eta") = jetCalibStageCopy.Eta();
        signalJets->at(iJet)->auxdata< float >("phi") = jetCalibStageCopy.Phi();
        signalJets->at(iJet)->auxdata< float >("e") = jetCalibStageCopy.E();
        continue; //Don't apply any further calibrations or uncertainties for this jet, skip to the next jet
      } else {

        //Use final insitu correction if not leading jet or we want to fully calibrate leading jet 
        if ( m_leadingGSC && iJet == 0){ // Set GSC-stage for leading jet
          xAOD::JetFourMom_t jetCalibGSCCopy = signalJets->at(iJet)->getAttribute<xAOD::JetFourMom_t>("JetGSCScaleMomentum");
          signalJets->at(iJet)->auxdata< float >("pt") = jetCalibGSCCopy.Pt();
          signalJets->at(iJet)->auxdata< float >("eta") = jetCalibGSCCopy.Eta();
          signalJets->at(iJet)->auxdata< float >("phi") = jetCalibGSCCopy.Phi();
          signalJets->at(iJet)->auxdata< float >("e") = jetCalibGSCCopy.E();
        } else { // Use in-situ correction. (This should be after eta-intercalibration if using V+jets
          signalJets->at(iJet)->auxdata< float >("pt") = originalJetKinematics.at(iJet).Pt();
          signalJets->at(iJet)->auxdata< float >("eta") = originalJetKinematics.at(iJet).Eta();
          signalJets->at(iJet)->auxdata< float >("phi") = originalJetKinematics.at(iJet).Phi();
          signalJets->at(iJet)->auxdata< float >("e") = originalJetKinematics.at(iJet).E();
        }
      }

      //////// Now apply futher V+jet & MJB calibrations as well as systematic variations ////////

      if( m_validation ){
        //All jets get treated the same in validation mode, and no MJB-specific calibrations are applied
        calibrateBelowThreshold( signalJets->at(iJet) , iVar );
      } else if( m_closureTest ){
        //Leading jet with MJB-derived calibration only, other jets as normally calibrated
        if (iJet == 0 || (signalJets->at(iJet)->pt() > m_subleadingPtThreshold.at(0)) ){
          applyMJBCalibration( signalJets->at(iJet) , iVar );
        }else{
          calibrateBelowThreshold( signalJets->at(iJet) , iVar );
        }
      } else {
        //Regular mode, apply extra calibrations to subleading jets
        if( iJet > 0 ){
          if( signalJets->at(iJet)->pt() > m_subleadingPtThreshold.at(0) ){
            applyMJBCalibration( signalJets->at(iJet) , iVar );
          }else{
            calibrateBelowThreshold( signalJets->at(iJet) , iVar );
          }
        }
      }

    }//for jets
    reorderJets( signalJets );

    if(m_debug) Info("execute()", "Pt threshold ");
    for (unsigned int iJet = 0; iJet < signalJets->size(); ++iJet){
      if( signalJets->at(iJet)->pt() < ptThresholdCut ){ //Default 25 GeV
        signalJets->erase(signalJets->begin()+iJet);
        --iJet;
      }
    }
    if (signalJets->size() < m_numJets)
      continue;
    passCut(iVar); //ptThreshold

    if(m_debug) Info("execute()", "Apply JVT ");
    for(unsigned int iJet = 0; iJet < signalJets->size(); ++iJet){
      signalJets->at(iJet)->auxdata< float >("Jvt") = m_JVTUpdateTool_handle->updateJvt( *(signalJets->at(iJet)) );
      if( !m_JetJVTEfficiencyTool_handle->passesJvtCut( *(signalJets->at(iJet)) ) ){
        signalJets->erase(signalJets->begin()+iJet);  --iJet;
      }
    }
    if (signalJets->size() < m_numJets)
      continue;
    passCut(iVar); //JVF

    //Old, manual JVT method
    //if( signalJets->at(iJet)->pt() < 60.*GeV && fabs(signalJets->at(iJet)->auxdecor< float >("detEta")) < 2.4 ){
    //  if( signalJets->at(iJet)->getAttribute<float>( "Jvt" ) < m_JVTCut ) { 

    if(m_debug) Info("execute()", "Jet Cleaning ");
    //// Ignore event if any of the used jets are not clean ////
    for(unsigned int iJet = 0; iJet < signalJets->size(); ++iJet){
      if(! m_JetCleaningTool_handle->keep( *(signalJets->at(iJet))) ){
        wk()->skipEvent();  return EL::StatusCode::SUCCESS;
      }//clean jet
    }
    passCut(iVar); //cleanJet

    //Create recoilJets object from all nonleading, passing jets
    TLorentzVector recoilJets;
    for (unsigned int iJet = 1; iJet < signalJets->size(); ++iJet){
      TLorentzVector tmpJet;
      tmpJet.SetPtEtaPhiE(signalJets->at(iJet)->pt(), signalJets->at(iJet)->eta(), signalJets->at(iJet)->phi(), signalJets->at(iJet)->e());
      recoilJets += tmpJet;
    }

    ///// Trigger Efficiency /////
    float prescale = 1.;
    bool passedTriggers = false;
    if (m_triggers.size() == 0)
      passedTriggers = true;

    for( unsigned int iT=0; iT < m_triggers.size(); ++iT){
      auto triggerChainGroup = m_AllTrigDecisionTool_handles.at(iT)->getChainGroup(m_triggers.at(iT));
      if(recoilJets.Pt() > m_triggerThresholds.at(iT)){
        if( triggerChainGroup->isPassed() ){
          passedTriggers = true;
          prescale = m_AllTrigDecisionTool_handles.at(iT)->getPrescale(m_triggers.at(iT));
        }
        break;
      }//recoil Pt
    } // each Trigger
    if( !passedTriggers ){
      continue;
    }
    passCut(iVar); //TriggerEff


    //Remove dijet events, i.e. events where subleading jet dominates the recoil jets
    if(m_debug) Info("execute()", "Pt asym selection ");
    double ptAsym = signalJets->at(1)->pt() / recoilJets.Pt();
    eventInfo->auxdecor< float >( "ptAsym" ) = ptAsym;
    if( ptAsym > ptAsymCut ){ //Default 0.8
      continue;
    }
    passCut(iVar); //ptAsym

    //Alpha is phi angle between leading jet and recoilJet system
    if(m_debug) Info("execute()", "Alpha Selection ");
    double alpha = fabs(DeltaPhi( signalJets->at(0)->phi(), recoilJets.Phi() )) ;
    eventInfo->auxdecor< float >( "alpha" ) = alpha;
    if( (M_PI-alpha) > alphaCut ){  //0.3 by default
      continue;
    }
    passCut(iVar); //alpha

    //Beta is phi angle between leading jet and each other passing jet
    if(m_debug) Info("execute()", "Beta Selection ");
    double smallestBeta=10., avgBeta = 0., thisBeta=0.;
    for(unsigned int iJet=1; iJet < signalJets->size(); ++iJet){
      thisBeta = DeltaPhi(signalJets->at(iJet)->phi(), signalJets->at(0)->phi() );
      if( !m_looseBetaCut && (thisBeta < smallestBeta) ) 
        smallestBeta = thisBeta;
      //RUN1 else if( m_looseBetaCut && (thisBeta < smallestBeta) && (signalJets->at(iJet)->pt() > signalJets->at(0)->pt()*0.25) )
      else if( m_looseBetaCut && (thisBeta < smallestBeta) && (signalJets->at(iJet)->pt() > signalJets->at(0)->pt()*0.1) )
        smallestBeta = thisBeta;
      avgBeta += thisBeta;
      signalJets->at(iJet)->auxdecor< float >( "beta") = thisBeta;
    }
    avgBeta /= (signalJets->size()-1);
    eventInfo->auxdecor< float >( "avgBeta" ) = avgBeta;

    if( smallestBeta < betaCut ){ 
        continue;
    }
    passCut(iVar); //beta


    //////////// B-tagging ///////////////
    for(unsigned int iB=0; iB < m_bTagWPs.size(); ++iB){ 
      for(unsigned int iJet=0; iJet < signalJets->size(); ++iJet){
        //m_MJBDetailStr  is bTag85
        SG::AuxElement::Decorator< int > isBTag( ("BTag_"+m_bTagWPs.at(iB)+"Fixed").c_str() );
        if( m_AllBTaggingSelectionTool_handles.at(iB)->accept( signalJets->at(iJet) ) ) {
          isBTag( *signalJets->at(iJet) ) = 1;
        }else{
          isBTag( *signalJets->at(iJet) ) = 0;
        }
  
  
        float thisSF(1.0);
        SG::AuxElement::Decorator< float > bTagSF( ("BTagSF_"+m_bTagWPs.at(iB)+"Fixed").c_str() );
        if( m_isMC && fabs(signalJets->at(iJet)->eta()) < 2.5 ){
          CP::CorrectionCode BJetEffCode;
          if( isBTag( *signalJets->at(iJet) ) == 1 ){
            BJetEffCode = m_AllBTaggingEfficiencyTool_handles.at(iB)->getScaleFactor( *signalJets->at(iJet), thisSF );
          }else{
            BJetEffCode = m_AllBTaggingEfficiencyTool_handles.at(iB)->getInefficiencyScaleFactor( *signalJets->at(iJet), thisSF );
          }
          if (BJetEffCode == CP::CorrectionCode::Error){
            Warning( "execute()", "Error in m_BJetEFFSFTool's getEfficiencyScaleFactor, setting Scale Factor to -2");
            thisSF = -2;
            //return EL::StatusCode::FAILURE;
          }
  
        } // if m_isMC, get SF
  
        bTagSF( *signalJets->at(iJet) ) = thisSF;
        //cout << iJet << " " << " WP:" << m_bTagWPs.at(iB) << " gives " << isBTag(*signalJets->at(iJet)) << " of " << thisSF << endl;
  
      }//signalJets
    }//bTagWPs

    //%%%%%%%%%%%%%%%%%%%%%%%%%%% End Selections %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%5

    ////////////////////// Add Extra Variables //////////////////////////////
    eventInfo->auxdecor< int >("njet") = signalJets->size();
    eventInfo->auxdecor< float >( "recoilPt" ) = recoilJets.Pt();
    eventInfo->auxdecor< float >( "recoilEta" ) = recoilJets.Eta();
    eventInfo->auxdecor< float >( "recoilPhi" ) = recoilJets.Phi();
    eventInfo->auxdecor< float >( "recoilM" ) = recoilJets.M();
    eventInfo->auxdecor< float >( "recoilE" ) = recoilJets.E();
    eventInfo->auxdecor< float >( "ptBal" ) = signalJets->at(0)->pt() / recoilJets.Pt();

    for(unsigned int iJet=0; iJet < signalJets->size(); ++iJet){

      vector<float> thisEPerSamp = signalJets->at(iJet)->auxdata< vector< float> >("EnergyPerSampling");
      float TotalE = 0., TileE = 0.;
      for( int iLayer=0; iLayer < 24; ++iLayer){
        TotalE += thisEPerSamp.at(iLayer);
      }

      TileE += thisEPerSamp.at(12);
      TileE += thisEPerSamp.at(13);
      TileE += thisEPerSamp.at(14);

      signalJets->at(iJet)->auxdecor< float >( "TileFrac" ) = TileE / TotalE;

    }


    eventInfo->auxdecor< float >("weight_mcEventWeight") = m_mcEventWeight;
    eventInfo->auxdecor< float >("weight_prescale") = prescale;
    eventInfo->auxdecor< float >("weight_xs") = m_xs * m_acceptance;
    if(m_isMC)
      eventInfo->auxdecor< float >("weight") = m_mcEventWeight*m_xs*m_acceptance;
    else
      eventInfo->auxdecor< float >("weight") = prescale;


    /////////////// Output Plots ////////////////////////////////
    if(m_debug) Info("execute()", "Begin Hist output for %s", m_sysVar.at(iVar).c_str() );
    m_jetHists.at(iVar)->execute( signalJets, eventInfo);


    if(m_debug) Info("execute()", "Begin TTree output for %s", m_sysVar.at(iVar).c_str() );
  ///////////////// Optional MiniTree Output for Nominal Only //////////////////////////
    if( m_writeTree ) {
      if(!m_writeNominalTree ||  m_NominalIndex == (int) iVar) {
      //!! The following is a bit slow!
        std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > originalSignalJetsSC = xAOD::shallowCopyContainer( *inJets );
        xAOD::JetContainer* plottingJets = new xAOD::JetContainer();
        xAOD::JetAuxContainer* plottingJetsAux = new xAOD::JetAuxContainer();
        plottingJets->setStore( plottingJetsAux );
        for(unsigned int iJet=0; iJet < signalJets->size(); ++iJet){
          xAOD::Jet* newJet = new xAOD::Jet();
          newJet->makePrivateStore( *(signalJets->at(iJet)) );
          plottingJets->push_back( newJet );
        }

        int iTree = iVar;
        if( m_writeNominalTree)
          iTree = 0;
        if(eventInfo)   m_treeList.at(iTree)->FillEvent( eventInfo    );
        if(signalJets)  m_treeList.at(iTree)->FillJets(  plottingJets);
        m_treeList.at(iTree)->FillTrigger( eventInfo );
        m_treeList.at(iTree)->Fill();

        delete plottingJets;
        delete plottingJetsAux;
      }//If it's not m_writeNominalTree or else we're on the nominal sample
    }//if m_writeTree


    /////////////////////////////////////// SystTool ////////////////////////////////////////
    if( m_bootstrap ){
      systTool->fillSyst(m_sysVar.at(iVar), eventInfo->runNumber(), eventInfo->eventNumber(), recoilJets.Pt()/GeV, (signalJets->at(0)->pt()/recoilJets.Pt()), eventInfo->auxdecor< float >("weight") );
    }

  }//For each iVar

//!! Other ideas for quicker plotting
/*
    std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > originalSignalJetsSC = xAOD::shallowCopyContainer( *inJets );
    xAOD::JetContainer* plottingJets = new xAOD::JetContainer();
    xAOD::JetAuxContainer* plottingJetsAux = new xAOD::JetAuxContainer();
    plottingJets->setStore( plottingJetsAux );
    for(unsigned int iJet=0; iJet < signalJets->size(); ++iJet){
      //plottingJets->push_back( signalJets->at(iJet) );
      xAOD::Jet* newJet = new xAOD::Jet();
      newJet->makePrivateStore( *(signalJets->at(iJet)) );
      plottingJets->push_back( newJet );
    }
    m_jetHists.at(iVar)->execute( inJets, m_mcEventWeight );


    ///////////////////////////// fill the tree ////////////////////////////////////////////
    if( m_writeTree ) {
      //Create  ConstDataVector or JetContainer for plotting purposes

      if(eventInfo)   m_treeList.at(iVar)->FillEvent( eventInfo    );
      if(signalJets)  m_treeList.at(iVar)->FillJets(  *(plottingJets)  );
      m_treeList.at(iVar)->Fill();

    }//if m_writeTree
    delete plottingJets;
*/



  delete signalJets;
  delete originalSignalJetsSC.first; delete originalSignalJetsSC.second; delete originalSignalJets;

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
  for(unsigned int iVar=0; iVar < m_sysVar.size(); ++iVar){
    m_jetHists.at(iVar)->finalize();
  }


  if( m_bootstrap ){
    systTool->writeToFile(wk()->getOutputFile("SystToolOutput"));
    delete systTool;  systTool = nullptr;
  }


  //Need to retroactively fill original bins of these histograms
  if(m_useCutFlow) {
    TFile *file = wk()->getOutputFile ("cutflow");
    TH1D* origCutflowHist = (TH1D*)file->Get("cutflow");
    TH1D* origCutflowHistW = (TH1D*)file->Get("cutflow_weighted");

    for(unsigned int iVar=0; iVar < m_sysVar.size(); ++iVar){
      for(unsigned int iBin=1; iBin < m_cutflowFirst; ++iBin){
        m_cutflowHist.at(iVar)->SetBinContent(iBin, origCutflowHist->GetBinContent(iBin) );
        m_cutflowHistW.at(iVar)->SetBinContent(iBin, origCutflowHistW->GetBinContent(iBin) );
      }//for iBin
    }//for each m_sysVar

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


EL::StatusCode MultijetBalanceAlgo::passCut(int iVar){
  if(m_useCutFlow) {
    if(m_debug) Info("passCut()", "Passing Cut %i", iVar);
    m_cutflowHist.at(iVar)->Fill(m_iCutflow, 1);
    m_cutflowHistW.at(iVar)->Fill(m_iCutflow, m_mcEventWeight);
    m_iCutflow++;
  }

return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetBalanceAlgo::passCutAll(){
  if(m_useCutFlow) {
    if(m_debug) Info("passCutAll()", "Passing Cut");
    for(unsigned int iVar=0; iVar < m_sysVar.size(); ++iVar){
      m_cutflowHist.at(iVar)->Fill(m_iCutflow, 1);
      m_cutflowHistW.at(iVar)->Fill(m_iCutflow, m_mcEventWeight);
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
    ifstream fileIn(  gSystem->ExpandPathName( m_XSFile.c_str() ) );
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
  double dphi=TVector2::Phi_0_2pi(phi1-phi2);
  dphi = TMath::Min(dphi,(2.0*M_PI)-dphi);
  double deta = eta1-eta2;
  return sqrt(deta*deta+dphi*dphi);
}

EL::StatusCode MultijetBalanceAlgo :: loadVariations (){
  if(m_debug) Info("loadVariations()", "loadVariations");

  // Add the configuration if AllSystematics is used //
  if( m_sysVariations.find("AllSystematics") != std::string::npos){
    if(m_isMC){
      m_sysVariations = "Nominal-localMJB";
    }else{
      m_sysVariations = "Nominal-localMJB-AllZjet-AllGjet-AllLAr-AllEtaIntercalibration-AllPileup-AllFlavor-AllPunchThrough";
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
      m_sysVar.push_back( "Nominal" ); m_sysTool.push_back( -1 ); m_sysToolIndex.push_back( -1 ); m_sysSign.push_back(0);
      m_NominalIndex = m_sysVar.size()-1;

    /////////////////// Every Jet Calibration Stage ////////////////
    }else if( varVector.at(iVar).compare("JetCalibSequence") == 0 ){
      if( m_JCSTokens.size() <= 0){
        Error( "loadVariations()", "JetCalibSequence is empty.  This will not be added to the systematics");
      }
      Info( "loadVariations()", "Adding JetCalibSequence");
      for( unsigned int iJCS = 0; iJCS < m_JCSTokens.size(); ++iJCS){
        //Name - JetCalibTool - Variation Number - sign
        m_sysVar.push_back("JCS_"+m_JCSTokens.at(iJCS) ); m_sysTool.push_back( 1 ); m_sysToolIndex.push_back( iJCS ); m_sysSign.push_back(0);
      }

    ////////////////////////////////// JES Uncertainties /////////////////////////////////////////
    } else if( varVector.at(iVar).find("All") != std::string::npos ){

      //Get JES systematic type from name
      std::string sysType = varVector.at(iVar).substr(3, varVector.at(iVar).size() );

      ifstream fileIn( gSystem->ExpandPathName( m_jetUncertaintyConfig.c_str() ) );
      std::string line;
      std::string subStr;
      while (getline(fileIn, line)){
        //Only get relevant lines, and nothing from propagated MJB (prop)!
        if( (line.find(".Name:") != std::string::npos) && (line.find("_prop") == std::string::npos) && (line.find("_orig") == std::string::npos)){

          istringstream iss(line);
          iss >> subStr;
          ////old method to get JES number
          //std::string thisJESNumberStr = subStr.substr(subStr.find_first_of('.')+1, subStr.find_last_of('.')-subStr.find_first_of('.')-1 );
          //int thisJESNumber = atoi( thisJESNumberStr.c_str() );

          //next get JES Name
          iss >> subStr;
          std::string thisJESName = subStr;

          if( thisJESName.find( sysType ) != std::string::npos ){
            std::string tmpJESName = thisJESName;
            if( tmpJESName.find("MCTYPE") != std::string::npos){
              tmpJESName.replace( tmpJESName.find("MCTYPE"), 6, "MC15");
            }
            unsigned int JESSpot = m_JetUncertaintiesTool_handle->getComponentIndex( "JET_"+tmpJESName );

            if( JESSpot >= m_JetUncertaintiesTool_handle->getNumComponents() ){
              Error( "loadVar", " Could not find component JET_%s, exiting.", tmpJESName.c_str());
              return EL::StatusCode::FAILURE;
            }
            if( m_debug )  Info("loadVar", "JES %s is at %i", tmpJESName.c_str(), JESSpot);
            //Name - JES Tool - JES Number - sign
            m_sysVar.push_back( thisJESName+"_pos" ); m_sysTool.push_back( 0 ); m_sysToolIndex.push_back( JESSpot ); m_sysSign.push_back( 1 );
            m_sysVar.push_back( thisJESName+"_neg" ); m_sysTool.push_back( 0 ); m_sysToolIndex.push_back( JESSpot ); m_sysSign.push_back( 0 );

          } //if the current JES type
        }//if the relevant Name line
      }//for each line in JES config

    //////////////////////////////////////// MJB  /////////////////////////////////////////
    } else if( varVector.at(iVar).compare("localMJB") == 0 ){
      //Name - MJB Variation - MJB Value - sign

      int systValue[2];
      //Alpha systematics are +-.1  (*100)
      systValue[0] = round(m_alpha*100)-10;
      systValue[1] = round(m_alpha*100)+10;
      m_sysVar.push_back("MJB_a"+to_string(systValue[0])+"_neg" );   m_sysTool.push_back( 2 ); m_sysToolIndex.push_back( systValue[0] ); m_sysSign.push_back(0);
      m_sysVar.push_back("MJB_a"+to_string(systValue[1])+"_pos" );   m_sysTool.push_back( 2 ); m_sysToolIndex.push_back( systValue[1] ); m_sysSign.push_back(1);

      //Beta systematics are +-.4 (*10)
      systValue[0] = round(m_beta*10)-4;
      systValue[1] = round(m_beta*10)+4;
      m_sysVar.push_back("MJB_b"+to_string(systValue[0])+"_neg" );   m_sysTool.push_back( 3 ); m_sysToolIndex.push_back( systValue[0] ); m_sysSign.push_back(0);
      m_sysVar.push_back("MJB_b"+to_string(systValue[1])+"_pos" );   m_sysTool.push_back( 3 ); m_sysToolIndex.push_back( systValue[1] ); m_sysSign.push_back(1);

      //pt Asymmetry systematics are +-.1 (*100)
      systValue[0] = round(m_ptAsym*100)-10;
      systValue[1] = round(m_ptAsym*100)+10;
      m_sysVar.push_back("MJB_pta"+to_string(systValue[0])+"_neg" );   m_sysTool.push_back( 4 ); m_sysToolIndex.push_back( systValue[0] ); m_sysSign.push_back(0);
      m_sysVar.push_back("MJB_pta"+to_string(systValue[1])+"_pos" );   m_sysTool.push_back( 4 ); m_sysToolIndex.push_back( systValue[1] ); m_sysSign.push_back(1);

      //pt threshold systematics are +- 5
      systValue[0] = round(m_ptThresh)-5;
      systValue[1] = round(m_ptThresh)+5;
      m_sysVar.push_back("MJB_ptt"+to_string(systValue[0])+"_neg" );   m_sysTool.push_back( 5 ); m_sysToolIndex.push_back( systValue[0] ); m_sysSign.push_back(0);
      m_sysVar.push_back("MJB_ptt"+to_string(systValue[1])+"_pos" );   m_sysTool.push_back( 5 ); m_sysToolIndex.push_back( systValue[1] ); m_sysSign.push_back(1);

      if (m_MJBIteration > 0 && m_MJBStatsOn){
        m_sysVar.push_back("MJB_stat0_pos"); m_sysTool.push_back( 6 ); m_sysToolIndex.push_back( 0  ); m_sysSign.push_back(1);
        m_sysVar.push_back("MJB_stat0_neg"); m_sysTool.push_back( 6 ); m_sysToolIndex.push_back( 0  ); m_sysSign.push_back(0);
        m_sysVar.push_back("MJB_stat1_pos"); m_sysTool.push_back( 6 ); m_sysToolIndex.push_back( 1  ); m_sysSign.push_back(1);
        m_sysVar.push_back("MJB_stat1_neg"); m_sysTool.push_back( 6 ); m_sysToolIndex.push_back( 1  ); m_sysSign.push_back(0);
      }

    }

  }//for varVector
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetBalanceAlgo :: loadTriggerTool(){

  if(m_debug) Info("loadTriggerTool", "loadTriggerTool");
  for(unsigned int iT=0; iT < m_triggers.size(); ++iT){

    asg::AnaToolHandle<Trig::TrigDecisionTool> this_TrigDecisionTool_handle;    
    this_TrigDecisionTool_handle.setTypeAndName("Trig::TrigDecisionTool/TrigDecisionTool_"+m_triggers.at(iT)+"_"+m_name);
    asg::AnaToolHandle<TrigConf::ITrigConfigTool> this_TrigConfigTool_handle;
    this_TrigConfigTool_handle.setTypeAndName("TrigConf::xAODConfigTool/TrigConfigTool_"+m_triggers.at(iT)+"_"+m_name);

    if( !this_TrigDecisionTool_handle.isUserConfigured() ){

      ANA_CHECK( ASG_MAKE_ANA_TOOL( this_TrigConfigTool_handle, TrigConf::xAODConfigTool ) ); 
      ANA_CHECK( this_TrigConfigTool_handle.retrieve() );

      ANA_CHECK( ASG_MAKE_ANA_TOOL( this_TrigDecisionTool_handle, Trig::TrigDecisionTool ) );
      ANA_CHECK( this_TrigDecisionTool_handle.setProperty( "ConfigTool", this_TrigConfigTool_handle.getHandle() ) );
      ANA_CHECK( this_TrigDecisionTool_handle.setProperty( "TrigDecisionKey", "xTrigDecision" ) );
      ANA_CHECK( this_TrigDecisionTool_handle.retrieve() );



    }
    m_AllTrigDecisionTool_handles.push_back( this_TrigDecisionTool_handle );
    m_AllTrigConfigTool_handles.push_back( this_TrigConfigTool_handle );

  }

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

  std::string thisJetDef = "";
  if( m_inContainerName.find("AntiKt4EMTopo") != std::string::npos )
    thisJetDef = "AntiKt4EMTopo";
  else if( m_inContainerName.find("AntiKt6EMTopo") != std::string::npos )
    thisJetDef = "AntiKt6EMTopo";
  else if( m_inContainerName.find("AntiKt4LCTopo") != std::string::npos )
    thisJetDef = "AntiKt4LCTopo";
  else if( m_inContainerName.find("AntiKt6LCTopo") != std::string::npos )
    thisJetDef = "AntiKt6LCTopo";

  if( !m_JetUncertaintiesTool_handle.isUserConfigured() ){
    ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JetUncertaintiesTool_handle, JetUncertaintiesTool) );
    ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("JetDefinition", thisJetDef) );
    ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("ConfigFile", m_jetUncertaintyConfig) );
    if( m_isAFII ){
      ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("MCType", "AFII") );
    }else{
      ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("MCType", "MC15") );
    }

    ANA_CHECK( m_JetUncertaintiesTool_handle.retrieve() );
  }

  return EL::StatusCode::SUCCESS;
}

//Setup V+jet calibration and systematics files
EL::StatusCode MultijetBalanceAlgo :: loadVjetCalibration(){
  if(m_debug) Info("loadVjetCalibration()", "loadVjetCalibration");

  std::string containerType;
  if( m_inContainerName.find("AntiKt4EMTopo") != std::string::npos)
    containerType = "EMJES_R4";
  else if( m_inContainerName.find("AntiKt6EMTopo") != std::string::npos )
    containerType = "EMJES_R6";
  else if( m_inContainerName.find("AntiKt4LCTopo") != std::string::npos )
    containerType = "LCJES_R4";
  else if( m_inContainerName.find("AntiKt6LCTopo") != std::string::npos )
    containerType = "LCJES_R6";

  TFile *VjetFile = TFile::Open( gSystem->ExpandPathName(m_VjetCalibFile.c_str()) , "READ" );

  //Only retrieve Nominal container
  std::string correctionName = containerType+"_correction";
  m_VjetHist = (TH1F*) VjetFile->Get( correctionName.c_str() );
  m_VjetHist->SetDirectory(0); //Detach histogram from file to memory
  VjetFile->Close();

  //// Find out the subleading jet pt cut for V+jets ////
  if( m_subleadingPtThreshold.at(0) == -1000 ){
    m_subleadingPtThreshold.at(0) = m_VjetHist->GetXaxis()->GetBinUpEdge( m_VjetHist->GetNbinsX() );
    //float lastCalibFactor = m_VjetHist->GetBinContent( m_VjetHist->GetNbinsX() );
    //m_subleadingPtThreshold.at(0) = floor( m_subleadingPtThreshold.at(0)*(1./lastCalibFactor) );
    Info("loadVjetCalibration", "Setting first subleading jet pt threshold to %f, taken from the V+jet configuration file", m_subleadingPtThreshold.at(0));
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
  if(m_debug)  std::cout << "Loaded MJB File" << std::endl;
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
  std::vector<std::string> new_sysVar;
  std::vector<int> new_sysTool;
  std::vector<int> new_sysToolIndex;
  std::vector<int> new_sysSign;

  int foundCount = 0;
  // Loop over each input MJB systematic histogram
  for(unsigned int i=0; i < m_MJBHists.size(); ++i){
    bool foundMatch = false;
    std::string histName = m_MJBHists.at(i)->GetName();

    //Add the MCType systematic if it's in the input, as this isn't added regularly
    if( histName.find("MCType") != std::string::npos ){
      new_sysVar.push_back( histName );
      new_sysTool.push_back( m_sysTool.at(m_NominalIndex) ); //Treat like nominal calibration
      new_sysToolIndex.push_back( m_sysToolIndex.at(m_NominalIndex) );
      new_sysSign.push_back( m_sysSign.at(m_NominalIndex) );
      foundMatch = true;
    } else {
      // Loop over the loaded systematics and find the match
      for(unsigned int iVar=0; iVar < m_sysVar.size(); ++iVar){
        if( histName.find( m_sysVar.at(iVar) ) != std::string::npos ){
          new_sysVar.push_back( histName );
          new_sysTool.push_back( m_sysTool.at(iVar) );
          new_sysToolIndex.push_back( m_sysToolIndex.at(iVar) );
          new_sysSign.push_back( m_sysSign.at(iVar) );
          foundMatch = true;
          break;
        }
      }//for m_sysVar
    }// if searching m_sysVar
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
  m_sysVar =       new_sysVar;
  m_sysTool =      new_sysTool;
  m_sysToolIndex = new_sysToolIndex;
  m_sysSign =      new_sysSign;

  Info("loadMJBCalibration()", "Succesfully loaded MJB calibration file");

//    for(unsigned int i=0; i < m_MJBHists.size(); ++i){
//      cout << m_MJBHists.at(i)->GetName() << endl;
//    }

  MJBFile->Close();

return EL::StatusCode::SUCCESS;
}

//EL::StatusCode MultijetBalanceAlgo :: applyJetCalibrationTool( xAOD::Jet* jet){
//  if(m_debug) Info("applyJetCalibrationTool()", "applyJetCalibrationTool");
//  if ( m_JetCalibrationTool_handle->applyCorrection( *jet ) == CP::CorrectionCode::Error ) {
//    Error("execute()", "JetCalibrationTool reported a CP::CorrectionCode::Error");
//    Error("execute()", "%s", m_name.c_str());
//    return StatusCode::FAILURE;
//  }
//  return EL::StatusCode::SUCCESS;
//}

EL::StatusCode MultijetBalanceAlgo :: applyJetUncertaintyTool( xAOD::Jet* jet , int iVar ){
  if(m_debug) Info("applyJetUncertaintyTool()", "applyJetUncertaintyTool");

  if( m_isMC //JetUncertaintyTool doesn't apply to MC
    || ( m_sysTool.at(iVar) != 0 ) ) //If not a JetUncertaintyTool sys variation
    return EL::StatusCode::SUCCESS;

  //Get the calibration
  float thisUncertainty = 1.;
  if( m_sysSign.at(iVar) == 1)
    thisUncertainty += m_JetUncertaintiesTool_handle->getUncertainty(m_sysToolIndex.at(iVar), *jet);
  else
    thisUncertainty -= m_JetUncertaintiesTool_handle->getUncertainty(m_sysToolIndex.at(iVar), *jet);

  //Apply the calibration to the TLorentzVector
  TLorentzVector thisJet;
  thisJet.SetPtEtaPhiE( jet->pt(), jet->eta(), jet->phi(), jet->e() );
  thisJet *= thisUncertainty;
  jet->auxdata< float >("pt") = thisJet.Pt();
  jet->auxdata< float >("eta") = thisJet.Eta();
  jet->auxdata< float >("phi") = thisJet.Phi();
  jet->auxdata< float >("e") = thisJet.E();

  return EL::StatusCode::SUCCESS;
}


// Apply the Vjet Calibration to jets below the first subleadingPtThreshold
EL::StatusCode MultijetBalanceAlgo :: applyVjetCalibration( xAOD::Jet* jet , int iVar ){
  if(m_debug) Info("applyVjetCalibration", "applyVjetCalibration ");

  if(m_isMC)
    return EL::StatusCode::SUCCESS;

  //if( (m_sysTool.at(iVar) == 1) || jet->pt() < 20.*GeV ){ //If NoCorr or not in V+jet correction range

  //Get nominal V+jet correction
  float thisCalibration = 1.;
  thisCalibration = m_VjetHist->GetBinContent( m_VjetHist->FindBin(jet->pt()/GeV) );


  //Modify the TLorentzVector
  TLorentzVector thisJet;
  thisJet.SetPtEtaPhiE(jet->pt(), jet->eta(), jet->phi(), jet->e());
  thisJet *= (1./thisCalibration);
  jet->auxdata< float >("pt") = thisJet.Pt();
  jet->auxdata< float >("eta") = thisJet.Eta();
  jet->auxdata< float >("phi") = thisJet.Phi();
  jet->auxdata< float >("e") = thisJet.E();

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetBalanceAlgo :: applyMJBCalibration( xAOD::Jet* jet , int iVar, bool isLead /*=false*/ ){
  if(m_debug) Info("applyMJBCalibration", "applyMJBCalibration ");

  if(m_isMC)
    return EL::StatusCode::SUCCESS;

  // Get calibration
  float thisCalibration = 1. / m_MJBHists.at(iVar)->GetBinContent( m_MJBHists.at(iVar)->FindBin(jet->pt()/GeV) );

  // MJB Statistical Systematic //
  // This is not well tested!! TODO
  // Is the error from the first iteration applied for the first iteration? If so, this needs to be done later in the plotting code
  if (m_sysTool.at(iVar) == 6){
    int reverseIndex = m_sysToolIndex.at(iVar);
    int numBins = m_MJBHists.at(iVar)->GetNbinsX();
    int index = numBins - reverseIndex;
    float errY = m_MJBHists.at(iVar)->GetBinError( index );
    if( m_sysSign.at(iVar) == 1) // then it's negative
      errY = -errY;
    thisCalibration = 1./ (m_MJBHists.at(iVar)->GetBinContent( m_MJBHists.at(iVar)->FindBin(jet->pt()/GeV) ) * (1+errY) );

  }

  // Apply calibration to TLorentzVector
  TLorentzVector thisJet;
  thisJet.SetPtEtaPhiE(jet->pt(), jet->eta(), jet->phi(), jet->e());
  thisJet *= thisCalibration; //modify TLV
  jet->auxdata< float >("pt") = thisJet.Pt();
  jet->auxdata< float >("eta") = thisJet.Eta();
  jet->auxdata< float >("phi") = thisJet.Phi();
  jet->auxdata< float >("e") = thisJet.E();

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetBalanceAlgo :: reorderJets( std::vector< xAOD::Jet*>* theseJets ){

  if(m_debug) Info("reorderJets()", "reorderJets ");
  xAOD::Jet* tmpJet;
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

EL::StatusCode MultijetBalanceAlgo::calibrateBelowThreshold(xAOD::Jet* jet, int iSysVar ){
  if(m_debug) Info("calibrateBelowThreshold()", "calibrateBelowThreshold ");

  //First calibrate for V+jet - this will check the proper pt automatically!
  if (m_VjetCalib)
    applyVjetCalibration( jet , iSysVar );
  //JetUncertainty comes after V+jet Calibration, as these files *should* have been generated using V+jet!
  //This will only use the same pt range as the Vjet
  applyJetUncertaintyTool( jet , iSysVar );

  return EL::StatusCode::SUCCESS;
}
