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
#include <MultijetBalanceAlgo/MultijetAlgorithim.h>
#include <xAODAnaHelpers/HelperFunctions.h>
#include <MultijetBalanceAlgo/MultijetHists.h>
#include "xAODCore/ShallowCopy.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"

// external tools include(s):
#include "JetCalibTools/JetCalibrationTool.h"
#include "JetSelectorTools/JetCleaningTool.h"
#include "JetUncertainties/JetUncertaintiesTool.h"
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
//#include "xAODTrigMissingET/TrigMissingETContainer.h"

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

#include "SystTool/SystContainer.h"


using namespace std;

// this is needed to distribute the algorithm to the workers
ClassImp(MultijetAlgorithim)


MultijetAlgorithim :: MultijetAlgorithim ()
{ }

MultijetAlgorithim :: MultijetAlgorithim (std::string name, std::string configName) :
  Algorithm(),
  m_name(name),
  m_configName(configName)
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().

//  !!
}

MultijetAlgorithim ::~MultijetAlgorithim(){
}

EL::StatusCode  MultijetAlgorithim :: configure ()
{
  Info("configure()", "Configuing MultijetAlgorithim Interface. User configuration read from : %s \n", m_configName.c_str());
  m_configName = gSystem->ExpandPathName( m_configName.c_str() );
  RETURN_CHECK_CONFIG("MultijetAlgorithim::configure()", m_configName);
  TEnv* config = new TEnv(m_configName.c_str());

  //  Read Standard Configuration Information
  m_inContainerName   = config->GetValue("InputContainer",   "");
  m_debug             = config->GetValue("Debug" ,           false );
  m_isAFII            = config->GetValue("IsAFII"  ,         false );
  m_isDAOD            = config->GetValue("IsDAOD"  ,         true );
  m_maxEvent          = config->GetValue("MaxEvent"  ,       1e8 );
  m_useCutFlow        = config->GetValue("UseCutFlow",       true);
  m_writeTree         = config->GetValue("WriteTree",        false);
  m_writeNominalTree  = config->GetValue("WriteNominalTree", false);
  m_eventDetailStr    = config->GetValue("EventDetailStr",   "");
  m_jetDetailStr      = config->GetValue("JetDetailStr",     "kinematic");
  m_MJBDetailStr      = config->GetValue("MJBDetailStr",     "");
  m_trigDetailStr            = config->GetValue("TrigDetailStr",         m_trigDetailStr.c_str());
  m_MJBIteration      = config->GetValue("MJBIteration",     0);
  m_MJBCorrectionFile = config->GetValue("MJBCorrectionFile",     "");
  m_MJBCorrectionBinning      = config->GetValue("MJBCorrectionBinning",     "");
  if( m_MJBCorrectionBinning.size() > 0 && m_MJBCorrectionBinning[0] != '_')
    m_MJBCorrectionBinning = "_"+m_MJBCorrectionBinning;
  m_leadJetMJBCorrection  = config->GetValue("LeadJetMJBCorrection",   "false");
  m_varString         = config->GetValue("Variations",       "Nominal");
  m_MCPileupCheckContainer = config->GetValue("MCPileupCheckContainer", "AntiKt4TruthJets");
  m_triggerConfig     = config->GetValue("Triggers", "");
  m_closureTest       = config->GetValue("ClosureTest" ,    false);

  m_PtAsym            = config->GetValue("PtAsym", 0.8);
  m_SubLeadingPt      = config->GetValue("SubLeadingPt", 800.);
  m_noLimitJESPt      = config->GetValue("NoLimitJESPt" ,    false);
  m_reverseSubleading = config->GetValue("ReverseSubleading" ,    false);
  m_leadingInsitu     = config->GetValue("LeadingInsitu", false);
  m_MJBStatsOn        = config->GetValue("MJBStatsOn", false);
  m_allJetBeta        = config->GetValue("AllJetBeta", false);

  m_bootstrap = config->GetValue("BootStrap", false);

  m_numJets = config->GetValue("NumJets", 3);

  if( m_writeNominalTree )
    m_writeTree = true;

  m_comEnergy = "13TeV";
  if( m_MCPileupCheckContainer.compare("None") == 0 ) //If they're identical
    m_useMCPileupCheck = false;


  // Save triggers to use
  std::stringstream ss(m_triggerConfig);
  std::string thisTriggerStr;
  std::string::size_type sz;
  while (std::getline(ss, thisTriggerStr, ',')) {
    m_triggers.push_back( thisTriggerStr.substr(0, thisTriggerStr.find_first_of(':')) );
    m_triggerThresholds.push_back( std::stof(thisTriggerStr.substr(thisTriggerStr.find_first_of(':')+1, thisTriggerStr.size()) , &sz) *GeV );
    cout << m_triggers.at(m_triggers.size()-1) << " " << m_triggerThresholds.at(m_triggers.size()-1) << endl;
  }


  ///// Tool Config parameters /////

  m_JVTCut                  = config->GetValue("JVTCut", 0.64);
  // JetCalibrationTool //
  m_jetDef                  = config->GetValue("JetDefinition", "");
  // JetCleaningTool //
  m_jetCleanCutLevel        = config->GetValue("JetCalibCutLevel", "LooseBad");
  m_jetCleanUgly            = config->GetValue("JetCleanUgly",     false );

  // JetUncertaintiesTool //
  m_jetUncertaintyConfig    = config->GetValue("JetUncertaintyConfig", "$ROOTCOREBIN/data/JetUncertainties/JES_2012/Final/InsituJES2012_AllNuisanceParameters.config");
  m_jetUncertaintyConfig = gSystem->ExpandPathName( m_jetUncertaintyConfig.c_str() );


  // JetCalibrationTool //
  m_jetCalibSequence          = config->GetValue("CalibSequence", "JetArea_Residual_Origin_EtaJES_GSC");
  if(m_isMC && m_isAFII)
    m_jetCalibConfig          = config->GetValue("JetCalibConfig", "JES_Prerecommendation2015_AFII_Apr2015.config");
  else if(m_isMC && !m_isAFII)
    m_jetCalibConfig          = config->GetValue("JetCalibConfig", "JES_MC15Prerecommendation_April2015.config");
    //m_jetCalibConfig          = config->GetValue("JetCalibConfig", "JES_Prerecommendation2015_Feb2015.config");
  else if(!m_isMC){
    m_jetCalibConfig          = config->GetValue("JetCalibConfig",  "JES_Full2012dataset_May2014.config");
  }

  if ( (m_inContainerName.find("LCTopo") == std::string::npos)
      && !m_isMC
      && m_jetCalibSequence.find("Insitu") == std::string::npos)
    m_jetCalibSequence += "_Insitu";

  if( (m_isMC || (m_inContainerName.find("LCTopo") != std::string::npos) )
      && m_jetCalibSequence.find("Insitu") != std::string::npos ){
    Error("initialize()", "Attempting to use an Insitu calibration sequence on MC.  Exiting.");
    return EL::StatusCode::FAILURE;
  }

  config->Print();
  Info("configure()", "MultijetAlgorithim Interface succesfully configured! \n");

  if( m_inContainerName.empty() ) {
    Error("configure()", "InputContainer is empty!");
    return EL::StatusCode::FAILURE;
  }

  std::cout << "Jet calib info: " <<  m_jetCalibConfig << " / " << m_jetCalibSequence << std::endl;

  delete config;

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetAlgorithim :: setupJob (EL::Job& job)
{
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.
  job.useXAOD();
  xAOD::Init( "MultijetAlgorithim" ).ignore(); // call before opening first file

  EL::OutputStream outForTree("tree");
  job.outputAdd (outForTree);

  job.outputAdd(EL::OutputStream("SystToolOutput"));

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MultijetAlgorithim :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.
  Info("histInitialize()", "Calling histInitialize \n");

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MultijetAlgorithim :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MultijetAlgorithim :: changeInput (bool firstFile)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MultijetAlgorithim :: initialize ()
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
  HelperFunctions::retrieve(eventInfo, "EventInfo", m_event, m_store);
  m_isMC = ( eventInfo->eventType( xAOD::EventInfo::IS_SIMULATION ) ) ? true : false;

  //RETURN_CHECK this?? !!
  if ( this->configure() == EL::StatusCode::FAILURE ) {
    Error("initialize()", "Failed to properly configure. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  getLumiWeights(eventInfo);

  //maximum subleading pt for each iteration
  m_maxSub.push_back(m_SubLeadingPt*GeV);  //iteration 0
  //m_maxSub.push_back(800.*GeV);  //iteration 0
  m_maxSub.push_back( 1400.*GeV);  //iteration 1



  // load all variations
  setupJetCalibrationStages();
  loadVariations();

  //Crap for Syst Tool
  // Fix this to be elegent and take any binning
  if( m_bootstrap ){
    systTool_nToys = 100;
    //double theseBins[] = {15. ,20. ,25. ,35. ,45. ,55. ,70. ,85. ,100. ,116. ,134. ,152. ,172. ,194. ,216. ,240. ,264. ,290. ,318. ,346.,376.,408.,442.,478.,516.,556.,598.,642.,688.,736.,786.,838.,894.,952.,1012.,1076.,1162.,1310.,1530.,1992.,2500., 3000., 3500., 4500.};
    double theseBins[] = {125, 150, 175, 200, 225, 250, 275, 300, 325, 350, 375, 400, 425, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000, 1050, 1100, 1150, 1200, 1300, 1500, 2000, 5000.};
    int binSize = sizeof(theseBins)/sizeof(theseBins[0]);
    for( int iBin=0; iBin < binSize; ++iBin){
      systTool_ptBins.push_back(theseBins[iBin]);
    }
    // we have m_sysVar, systTool_ptBins, systTool_nToys
    systTool = new SystContainer(m_sysVar, systTool_ptBins, systTool_nToys);
  }

  if(m_useCutFlow) {

    //std::string newName;
    TFile *file = wk()->getOutputFile ("cutflow");
    TH1D* origCutflowHist = (TH1D*)file->Get("cutflow");
    TH1D* origCutflowHistW = (TH1D*)file->Get("cutflow_weighted");

    m_cutflowFirst = origCutflowHist->GetXaxis()->FindBin("njets");
    origCutflowHistW->GetXaxis()->FindBin("njets");
    origCutflowHist->GetXaxis()->FindBin("trigger2");
    origCutflowHistW->GetXaxis()->FindBin("trigger2");
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
    MultijetHists* thisJetHists = new MultijetHists( ( "Iteration"+m_ss.str()+"_"+m_sysVar.at(iVar) ), (m_jetDetailStr+" "+m_MJBDetailStr).c_str() );
    m_jetHists.push_back(thisJetHists);
    m_jetHists.at(iVar)->initialize(m_MJBCorrectionBinning);
    m_jetHists.at(iVar)->record( wk() );
  }
  m_ss.str("");



  loadTriggerTool();
  loadJVTTool();
  //load Calibration and systematics files
  loadJetCalibrationTool();
  loadJetCleaningTool();
  loadJetUncertaintyTool();
  loadVjetCalibration();

  if (loadMJBCalibration() == EL::StatusCode::FAILURE)
    return EL::StatusCode::FAILURE;


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
      m_treeList.at(iTree)->AddJets(m_jetDetailStr);
      m_treeList.at(iTree)->AddTrigger( m_trigDetailStr );
//      m_treeList.at(iTree)->AddMJB(m_MJBDetailStr);
    }//for iTree

  }//if m_writeTree

  Info("initialize()", "Succesfully initialized output TTree! \n");

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MultijetAlgorithim :: execute ()
{
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.
  if(m_debug) Info("execute()", "Begin Execute");
  ++m_eventCounter;

  if(m_eventCounter >  m_maxEvent){
      wk()->skipEvent();  return EL::StatusCode::SUCCESS;
    }

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
    HelperFunctions::retrieve(eventInfo, "EventInfo", m_event, m_store);
    m_mcEventWeight = (m_isMC ? eventInfo->mcEventWeight() : 1.) ;

    //const xAOD::VertexContainer* vertices = HelperFunctions::getContainer<xAOD::VertexContainer>("PrimaryVertices", m_event, m_store);;
    const xAOD::VertexContainer* vertices = 0;
    HelperFunctions::retrieve(vertices, "PrimaryVertices", m_event, m_store);
    m_pvLocation = HelperFunctions::getPrimaryVertexLocation( vertices );  //Get primary vertex for JVF cut

    //const xAOD::JetContainer* inJets = HelperFunctions::getContainer<xAOD::JetContainer>(m_inContainerName, m_event, m_store);
    const xAOD::JetContainer* inJets = 0;
    HelperFunctions::retrieve(inJets, m_inContainerName, m_event, m_store);

    if(inJets->size() < m_numJets){
      wk()->skipEvent();  return EL::StatusCode::SUCCESS;
  }
  passCutAll(); //njets

  //Create an editable shallow copy and a removable container
  std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > originalSignalJetsSC = xAOD::shallowCopyContainer( *inJets );

  std::vector< xAOD::Jet*>* originalSignalJets = new std::vector< xAOD::Jet* >();
  for( auto thisJet : *(originalSignalJetsSC.first) ) {
     originalSignalJets->push_back( thisJet );
   }

  const xAOD::JetContainer* truthJets = 0;
  if(m_useMCPileupCheck && m_isMC){
    //truthJets = HelperFunctions::getContainer<xAOD::JetContainer>(m_MCPileupCheckContainer, m_event, m_store);
    HelperFunctions::retrieve(truthJets, m_MCPileupCheckContainer, m_event, m_store);
  }

  /////////////////////////// Begin Selections and Creation of Variables ///////////////////////////////
  if(m_debug) Info("execute()", "Begin Selections ");
  //Standard values that may be varied
  float alphaCut, betaCut, ptAsymCut, ptThresholdCut;

  /////////////////Selections from note ///////////////////
  //triger: prescaled for 300 < pt_Recoil < 600 GeV, and unprescaled for pt_recoil > 600

  //at least 2 tracks with vertex - Done in baseEventSelection
  //!!data quality to remove fake jets from noise bursts in caloriters, non-collision background, and from cosmic rays

  if(m_debug) Info("execute()", "Get Raw Kinematics ");
  vector<TLorentzVector> rawJetKinematics;
  for (unsigned int iJet = 0; iJet < originalSignalJets->size(); ++iJet){
    TLorentzVector thisJet;
    thisJet.SetPtEtaPhiE(originalSignalJets->at(iJet)->pt(), originalSignalJets->at(iJet)->eta(), originalSignalJets->at(iJet)->phi(), originalSignalJets->at(iJet)->e());
    rawJetKinematics.push_back(thisJet);
  }

  if(m_debug) Info("execute()", "Apply Jet Calibration Tool ");
  for(unsigned int iJet=0; iJet < originalSignalJets->size(); ++iJet){
//cout << "original Pt is " << originalSignalJets->at(iJet)->pt() << endl;
    applyJetCalibrationTool( originalSignalJets->at(iJet) );
    originalSignalJets->at(iJet)->auxdecor< float >( "jetCorr") = originalSignalJets->at(iJet)->pt() / rawJetKinematics.at(iJet).Pt() ;
  }
  reorderJets( originalSignalJets );
//start


  ////  trigger ////
  if(m_debug) Info("execute()", "Trigger ");
  float prescale = 1.;
  bool passedTriggers = false;
  if (m_triggers.size() == 0)
    passedTriggers = true;

  for( unsigned int iT=0; iT < m_triggers.size(); ++iT){

    auto triggerChainGroup = m_trigDecTools.at(iT)->getChainGroup(m_triggers.at(iT));
    if(originalSignalJets->at(0)->pt() > m_triggerThresholds.at(iT)){
      if( triggerChainGroup->isPassed() ){
        passedTriggers = true;
        prescale = m_trigDecTools.at(iT)->getPrescale(m_triggers.at(iT));
      }
      break;
    // need to break after the first isPassed, even if trigger Thresholds is low!

//      std::string l1string = "";
//      if (m_triggers.at(iT).find("HLT_j360") != std::string::npos){
//        l1string = "L1_J100";
//      }else if(m_triggers.at(iT).find("HLT_j260") != std::string::npos){
//        l1string = "L1_J75";
//      }else if(m_triggers.at(iT).find("HLT_j150") != std::string::npos){
//        l1string = "L1_J40";
//      }
//
//      prescaleCut = triggerChainGroup->getPrescale();
//cout << m_triggers.at(iT) << " : " << l1string << endl;
//cout << m_trigDecTools.at(iT)->getPrescale(m_triggers.at(iT)) << ":" << m_trigDecTools.at(iT)->getPrescale(m_triggers.at(iT),TrigDefs::requireDecision) << " : " << m_trigDecTools.at(iT)->getPrescale(l1string.c_str()) << endl;
//cout << "Final? " << m_trigDecTools.at(iT)->getPrescale(m_triggers.at(iT)) << std::endl;
//cout << "Prescale cut " << prescaleCut << endl;
//
//
//      //Check that it's also below another trigger region ??
////      cout << "fixed " << TrigConf::PrescaleSet::getPrescaleFromCut(prescale) << endl;
//
////      HLT_j360 (unp) = L1_J100 (unp)
////      HLT_j260 (p) = L1_J75 (unp)
////      HLT_j150 (p) = L1_J40 (p)
//      break;
    }
  }

  if( !passedTriggers ){
    delete originalSignalJetsSC.first; delete originalSignalJetsSC.second; delete originalSignalJets;
    wk()->skipEvent();  return EL::StatusCode::SUCCESS;
  }
  passCutAll(); //trigger2

  //Assign detEta for jets .  Will this be changed by calibrations?
  if(m_debug) Info("execute()", "DetEta ");
  for(unsigned int iJet=0; iJet < originalSignalJets->size(); ++iJet){
    xAOD::JetFourMom_t jetConstituentP4 = originalSignalJets->at(iJet)->getAttribute<xAOD::JetFourMom_t>("JetEMScaleMomentum");
    //xAOD::JetFourMom_t jetConstituentP4 = originalSignalJets->at(iJet)->getAttribute<xAOD::JetFourMom_t>("JetConstitScaleMomentum");
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


  //Save original pt of all jets
  //!! to pointer?
  vector<TLorentzVector> originalJetKinematics;
  for (unsigned int iJet = 0; iJet < originalSignalJets->size(); ++iJet){
    TLorentzVector thisJet;
    thisJet.SetPtEtaPhiE(originalSignalJets->at(iJet)->pt(), originalSignalJets->at(iJet)->eta(), originalSignalJets->at(iJet)->phi(), originalSignalJets->at(iJet)->e());
    originalJetKinematics.push_back(thisJet);
  }

  int m_cutflowFirst_SystLoop = m_iCutflow; //Get cutflow position for systematic looping
  vector< xAOD::Jet*>* signalJets = new std::vector< xAOD::Jet* >();

  for(unsigned int iVar=0; iVar < m_sysVar.size(); ++iVar){

    if(m_debug) Info("execute()", "Starting variation %i %s", iVar, m_sysVar.at(iVar).c_str());
    //Reset standard values
    *signalJets = *originalSignalJets;
    m_iCutflow = m_cutflowFirst_SystLoop;
    alphaCut = 0.3; //0.3
    betaCut = 1.0; //1
//    ptAsymCut = 0.8; //0.8
    ptAsymCut = m_PtAsym;
    ptThresholdCut = 25.*GeV;

    //Set relevant variations for this iVar
    if( m_sysTool.at(iVar) == 2 ){ //alpha
//      std::string thisVarStr = m_sysVar.at(iVar).substr( m_sysVar.at(iVar).find("MJB_a")+5, 2 );
//      alphaCut = std::stod(thisVarStr)  / 100;
      alphaCut = (double) m_sysToolIndex.at(iVar)  / 100.;
    }else if( m_sysTool.at(iVar) == 3 ){ //beta
//      std::string thisVarStr = m_sysVar.at(iVar).substr( m_sysVar.at(iVar).find("MJB_b")+5, 2 );
//      betaCut = std::stod(thisVarStr) / 10.;
      betaCut = (double) m_sysToolIndex.at(iVar)  / 10.;
    }else if( m_sysTool.at(iVar) == 4 ){ //pta
//      std::string thisVarStr = m_sysVar.at(iVar).substr( m_sysVar.at(iVar).find("MJB_pta")+7, 2 );
//      ptAsymCut = std::stod(thisVarStr) / 100.;
      ptAsymCut = (double) m_sysToolIndex.at(iVar)  / 100.;
    }else if( m_sysTool.at(iVar) == 5 ){ //ptt
//      std::string thisVarStr = m_sysVar.at(iVar).substr( m_sysVar.at(iVar).find("MJB_ptt")+7, 2 );
//      ptThresholdCut = std::stod(thisVarStr) * GeV;
      ptThresholdCut = (double) m_sysToolIndex.at(iVar) * GeV;
    }

    //?? eta uncertainties should be applied before V+jet nominal?? This makes more sense to me, but is not what Gagik did. Ask Bogdan!
    if(m_debug) Info("execute()", "Apply other calibrations ");
    for (unsigned int iJet = 0; iJet < signalJets->size(); ++iJet){

      if(m_sysTool.at(iVar) == 1){
        int iCalibStage = m_sysToolIndex.at(iVar);
        xAOD::JetFourMom_t jetCalibStageCopy = signalJets->at(iJet)->getAttribute<xAOD::JetFourMom_t>( m_JCSStrings.at(iCalibStage).c_str() );
        signalJets->at(iJet)->auxdata< float >("pt") = jetCalibStageCopy.Pt();
        signalJets->at(iJet)->auxdata< float >("eta") = jetCalibStageCopy.Eta();
        signalJets->at(iJet)->auxdata< float >("phi") = jetCalibStageCopy.Phi();
        signalJets->at(iJet)->auxdata< float >("e") = jetCalibStageCopy.E();
      } else {

        if( iJet !=0 || m_leadingInsitu ){ //Use Insitu Correction
          signalJets->at(iJet)->auxdata< float >("pt") = originalJetKinematics.at(iJet).Pt();
          signalJets->at(iJet)->auxdata< float >("eta") = originalJetKinematics.at(iJet).Eta();
          signalJets->at(iJet)->auxdata< float >("phi") = originalJetKinematics.at(iJet).Phi();
          signalJets->at(iJet)->auxdata< float >("e") = originalJetKinematics.at(iJet).E();
        } else { //Use GSC Correction for lead jet
          xAOD::JetFourMom_t jetCalibGSCCopy = signalJets->at(iJet)->getAttribute<xAOD::JetFourMom_t>("JetGSCScaleMomentum");
//cout << "GSC Pt is " << jetCalibGSCCopy.Pt() << endl;
          signalJets->at(iJet)->auxdata< float >("pt") = jetCalibGSCCopy.Pt();
          signalJets->at(iJet)->auxdata< float >("eta") = jetCalibGSCCopy.Eta();
          signalJets->at(iJet)->auxdata< float >("phi") = jetCalibGSCCopy.Phi();
          signalJets->at(iJet)->auxdata< float >("e") = jetCalibGSCCopy.E();
        }
      }

      if(iJet > 0){  //Don't apply systematic corrections to lead jet
        applyJetUncertaintyTool( signalJets->at(iJet) , iVar );
        // Vjet currently removed!! What's the best way to apply these uncertainties?!
        applyMJBCalibration( signalJets->at(iJet) , iVar );
      } else if( m_closureTest || m_leadingInsitu ){ //Apply MJB to lead jet
        //apply previous correction for closure test??
        applyMJBCalibration( signalJets->at(iJet), iVar, true );
      }

//prev      if(iJet > 0){  //Don't apply systematic corrections to lead jet
//prev        applyJetUncertaintyTool( signalJets->at(iJet) , iVar );
//prev        applyVjetCalibration( signalJets->at(iJet) , iVar );
//prev        applyMJBCalibration( signalJets->at(iJet) , iVar );
//prev      } else if( m_closureTest){ //Apply MJB to lead jet
//prev        applyMJBCalibration( signalJets->at(iJet), iVar, true );
//prev      }

    }
    reorderJets( signalJets );

    //Z/y - jet balance is only valid up to 800 GeV
    if(m_debug) Info("execute()", "Subleading pt selection ");
    if( !m_reverseSubleading && (signalJets->at(1)->pt() > m_maxSub.at(m_MJBIteration)) ){
        continue;
    }else if( m_reverseSubleading && (signalJets->at(1)->pt() <= m_maxSub.at(m_MJBIteration)) ){
        continue;
    }
    passCut(iVar); //ptSub

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

//    if(m_debug) Info("execute()", "Apply JVF ");
//    for(unsigned int iJet = 0; iJet < signalJets->size(); ++iJet){
//      if( signalJets->at(iJet)->pt() < 50.*GeV && fabs(signalJets->at(iJet)->auxdecor< float >("detEta")) < 2.4 ){
//        if( signalJets->at(iJet)->getAttribute< std::vector<float> >( "JVF" ).at( m_pvLocation ) < 0.25 ) {
//          signalJets->erase(signalJets->begin()+iJet);  --iJet;
//        }
//      }
//    }
    if(m_debug) Info("execute()", "Apply JVT ");
    for(unsigned int iJet = 0; iJet < signalJets->size(); ++iJet){
      signalJets->at(iJet)->auxdata< float >("Jvt") = m_JVTToolHandle->updateJvt( *(signalJets->at(iJet)) );
      if( signalJets->at(iJet)->pt() < 50.*GeV && fabs(signalJets->at(iJet)->auxdecor< float >("detEta")) < 2.4 ){
        if( signalJets->at(iJet)->getAttribute<float>( "Jvt" ) < m_JVTCut ) { //loose 0.14, medium 0.64, tight 0.92
//          cout << "Removing jet with pt/eta/jvt " << signalJets->at(iJet)->pt() << "/" << signalJets->at(iJet)->auxdecor< float >("detEta") << "/" << signalJets->at(iJet)->getAttribute<float>( "Jvt" ) << endl;
          signalJets->erase(signalJets->begin()+iJet);  --iJet;
        }
      }
    }
    if (signalJets->size() < m_numJets)
      continue;
    passCut(iVar); //JVF


    if(m_debug) Info("execute()", "Jet Cleaning ");
    //// Specialized jet Cleaning: ignore event if any of the used jets are not clean ////
    for(unsigned int iJet = 0; iJet < signalJets->size(); ++iJet){
      if(! m_JetCleaningTool->accept( *(signalJets->at(iJet))) ){
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
      // !! thisBeta = fabs(TVector2::Phi_mpi_pi( signalJets->at(iJet)->phi() - signalJets->at(0)->phi() ));
      thisBeta = DeltaPhi(signalJets->at(iJet)->phi(), signalJets->at(0)->phi() );
      //std::cout << thisBeta << " " << signalJets->at(iJet)->pt() << std::endl;
      if( m_allJetBeta )
        smallestBeta = thisBeta;
      else if( (thisBeta < smallestBeta) && (signalJets->at(iJet)->pt() > signalJets->at(0)->pt()*0.25) )
        smallestBeta = thisBeta;
      avgBeta += thisBeta;
      signalJets->at(iJet)->auxdecor< float >( "beta") = thisBeta;
    }
    avgBeta /= (signalJets->size()-1);
    eventInfo->auxdecor< float >( "avgBeta" ) = avgBeta;

    if( smallestBeta < betaCut ){ //1.0
        continue;
    }
    passCut(iVar); //beta




  //from his definitions beta_corr < 1 && (M_PI-alpha_corr) < 0.02

  ////plots:
  //            //for 3 jet events only
  //            sin23_to_Sin2 = sin( dphi2+dphi3)/sing(dphi2)
  //            similar for sing23_to_sin3
  //            pt2R = jets[1].Perp()*sing23_to_sin3;
  //            pt3R = jets[2].Perp()*sin23_to_sin2;

    //%%%%%%%%%%%%%%%%%%%%%%%%%%% End Selections %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%5

    ////////////////////// Add Extra Variables //////////////////////////////
    eventInfo->auxdecor< int >("njet") = signalJets->size();
    eventInfo->auxdecor< float >( "recoilPt" ) = recoilJets.Pt();
    eventInfo->auxdecor< float >( "recoilEta" ) = recoilJets.Eta();
    eventInfo->auxdecor< float >( "recoilPhi" ) = recoilJets.Phi();
    eventInfo->auxdecor< float >( "recoilM" ) = recoilJets.M();
    eventInfo->auxdecor< float >( "recoilE" ) = recoilJets.E();
    eventInfo->auxdecor< float >( "ptBal" ) = signalJets->at(0)->pt() / recoilJets.Pt();
    eventInfo->auxdecor< float >( "ptBal2" ) = 0.5 * (signalJets->at(0)->pt() + recoilJets.Pt()) / recoilJets.Pt();

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
    if(m_debug) Info("execute()", "Begin output for %s", m_sysVar.at(iVar).c_str() );
    m_jetHists.at(iVar)->execute( signalJets, eventInfo, m_pvLocation );


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
        if(signalJets)  m_treeList.at(iTree)->FillJets(  plottingJets, m_pvLocation  );
        m_treeList.at(iTree)->FillTrigger( eventInfo );
        m_treeList.at(iTree)->Fill();
//        m_treeList.at(iTree)->ClearMJB();

        //if(eventInfo)   m_nominalTree->FillEvent( eventInfo    );
        //if(signalJets)  m_nominalTree->FillJets(  *plottingJets  );
        //m_nominalTree->Fill();
        //m_nominalTree->ClearUser();
        delete plottingJets;
        delete plottingJetsAux;
      }//If it's not m_writeNominalTree or else we're on the nominal sample
    }//if m_writeTree


    /////////////////////////////////////// SystTool ////////////////////////////////////////
    if( m_bootstrap ){
      systTool->fillSyst(m_sysVar.at(iVar), eventInfo->runNumber(), eventInfo->eventNumber(), recoilJets.Pt()/GeV, (signalJets->at(0)->pt()/recoilJets.Pt()), eventInfo->auxdecor< float >("weight") );
    }

  }//For each iVar

//!! Other ideas
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


EL::StatusCode MultijetAlgorithim :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MultijetAlgorithim :: finalize ()
{
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

//  if( m_writeTree){
//    TFile * file = wk()->getOutputFile ("tree");
//    for(unsigned int iTree=0; iTree < m_treeList.size(); ++iTree){
//      if(!m_treeList.at(iTree)->writeTo( file )) {
//        Error("finalize()", "Failed to write tree to ouput file!");
//        return EL::StatusCode::FAILURE;
//      }
//    }
//  }//if m_writeTree


  if( m_bootstrap ){
    //systTool->writeToFile("SystToolOutput.root");
    systTool->writeToFile(wk()->getOutputFile("SystToolOutput"));
    delete systTool;
  }

  delete m_JetCalibrationTool;
  delete m_JetCleaningTool;
  delete m_JetUncertaintiesTool;

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
  } //m_useCutflow


  //Add one cutflow histogram to output for number of initial events
  std::string thisName;
  if(m_isMC)
    m_ss << m_mcChannelNumber;
  else
    m_ss << m_runNumber;

  if( m_useCutFlow) {

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
  }

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



EL::StatusCode MultijetAlgorithim :: histFinalize ()
{
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


EL::StatusCode MultijetAlgorithim::passCut(int iVar){
  if(m_debug) Info("passCut()", "Passing Cut %i", iVar);
  m_cutflowHist.at(iVar)->Fill(m_iCutflow, 1);
  m_cutflowHistW.at(iVar)->Fill(m_iCutflow, m_mcEventWeight);
  m_iCutflow++;

return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetAlgorithim::passCutAll(){
  for(unsigned int iVar=0; iVar < m_sysVar.size(); ++iVar){
    m_cutflowHist.at(iVar)->Fill(m_iCutflow, 1);
    m_cutflowHistW.at(iVar)->Fill(m_iCutflow, m_mcEventWeight);
  }
  m_iCutflow++;

return EL::StatusCode::SUCCESS;
}

//This grabs luminosity, acceptace, and eventNumber information from the respective text file
//format     147915 2.3793E-01 5.0449E-03 499000
EL::StatusCode MultijetAlgorithim::getLumiWeights(const xAOD::EventInfo* eventInfo) {

  if(!m_isMC){
    m_runNumber = eventInfo->runNumber();
    m_xs = 1;
    m_acceptance = 1;
  }else{
    m_mcChannelNumber = eventInfo->mcChannelNumber();
//cout << m_mcChannelNumber << endl;
    ifstream fileIn(  gSystem->ExpandPathName( ("$ROOTCOREBIN/data/MultijetBalanceAlgo/XsAcc_"+m_comEnergy+".txt").c_str() ) );
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
double MultijetAlgorithim::DeltaPhi(double phi1, double phi2){
  phi1=TVector2::Phi_0_2pi(phi1);
  phi2=TVector2::Phi_0_2pi(phi2);
  return fabs(TVector2::Phi_mpi_pi(phi1-phi2));
}

//Calculate DeltaR
double MultijetAlgorithim::DeltaR(double eta1, double phi1,double eta2, double phi2){
  phi1=TVector2::Phi_0_2pi(phi1);
  phi2=TVector2::Phi_0_2pi(phi2);
  double dphi=TVector2::Phi_0_2pi(phi1-phi2);
  dphi = TMath::Min(dphi,(2.0*M_PI)-dphi);
  double deta = eta1-eta2;
  return sqrt(deta*deta+dphi*dphi);
}

EL::StatusCode MultijetAlgorithim :: loadVariations (){

  // Add the configuration if AllSystematics is used //
  if( m_varString.find("AllSystematics") != std::string::npos){
    m_varString = "Nominal-JetCalibSequence-Special-MJB-AllZjet-AllGjet-AllLAr";
  }

  m_NominalIndex = -1; //The index of the nominal

  // Turn into a vector of all systematics names
  std::vector< std::string> varVector;
  size_t pos = 0;
  while ((pos = m_varString.find("-")) != std::string::npos){
    varVector.push_back( m_varString.substr(0, pos) );
    m_varString.erase(0, pos+1);
  }
  varVector.push_back( m_varString ); //append final one

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

    /////////////////////////////// Special ///////////////////////////////
    }else if( varVector.at(iVar).compare("Special") == 0 ){

      ifstream fileIn( gSystem->ExpandPathName( m_jetUncertaintyConfig.c_str() ) );
      std::string line;
      std::string subStr;
      while (getline(fileIn, line)){
        if (line.find(".Name:") != std::string::npos){
          //get JES number
          istringstream iss(line);
          iss >> subStr;
          std::string thisJESNumberStr = subStr.substr(subStr.find_first_of('.')+1, subStr.find_last_of('.')-subStr.find_first_of('.')-1 );
          int thisJESNumber = atoi( thisJESNumberStr.c_str() );
          if( thisJESNumber >= 57 && thisJESNumber <= 68){
            //next get JES name
            iss >> subStr;

            //Name - JES Tool - JES Number - sign
            m_sysVar.push_back( subStr ); m_sysTool.push_back( 0 ); m_sysToolIndex.push_back( thisJESNumber ); m_sysSign.push_back( 1 );
            m_sysVar.push_back( subStr ); m_sysTool.push_back( 0 ); m_sysToolIndex.push_back( thisJESNumber ); m_sysSign.push_back( 0 );

          } //if a Special JES
        }//if the relevant Name line
      }//for each line in JES config


    ////////////////////////////////// GJ, ZJ, or LAr /////////////////////////////////////////
    } else if( varVector.at(iVar).find("All") != std::string::npos ){

      //Get JES systematic type from name
      std::string sysType = varVector.at(iVar).substr(3, varVector.at(iVar).size() );

      ifstream fileIn( gSystem->ExpandPathName( m_jetUncertaintyConfig.c_str() ) );
      std::string line;
      std::string subStr;
      while (getline(fileIn, line)){
        if (line.find(".Name:") != std::string::npos){

          //get JES number
          istringstream iss(line);
          iss >> subStr;
          std::string thisJESNumberStr = subStr.substr(subStr.find_first_of('.')+1, subStr.find_last_of('.')-subStr.find_first_of('.')-1 );
          int thisJESNumber = atoi( thisJESNumberStr.c_str() );

          //next get JES Name
          iss >> subStr;
          std::string thisJESName = subStr;
          if( thisJESName.find( sysType ) != std::string::npos ){

            //Name - JES Tool - JES Number - sign
            m_sysVar.push_back( thisJESName+"_pos" ); m_sysTool.push_back( 0 ); m_sysToolIndex.push_back( thisJESNumber ); m_sysSign.push_back( 1 );
            m_sysVar.push_back( thisJESName+"_neg" ); m_sysTool.push_back( 0 ); m_sysToolIndex.push_back( thisJESNumber ); m_sysSign.push_back( 0 );

          } //if the current JES type
        }//if the relevant Name line
      }//for each line in JES config

    //////////////////////////////////////// MJB  /////////////////////////////////////////
    } else if( varVector.at(iVar).compare("MJB") == 0 ){
      //Name - MJB Variation - MJB Value - sign
      m_sysVar.push_back("MJB_a40_pos");   m_sysTool.push_back( 2 ); m_sysToolIndex.push_back( 40 ); m_sysSign.push_back(1);
      m_sysVar.push_back("MJB_a20_neg");   m_sysTool.push_back( 2 ); m_sysToolIndex.push_back( 20 ); m_sysSign.push_back(0);
      m_sysVar.push_back("MJB_b15_pos");   m_sysTool.push_back( 3 ); m_sysToolIndex.push_back( 15 ); m_sysSign.push_back(1);
      m_sysVar.push_back("MJB_b05_neg");   m_sysTool.push_back( 3 ); m_sysToolIndex.push_back( 5  ); m_sysSign.push_back(0);
      m_sysVar.push_back("MJB_pta90_pos"); m_sysTool.push_back( 4 ); m_sysToolIndex.push_back( 90 ); m_sysSign.push_back(1);
      if(m_PtAsym < 0.9){ // don't do it for 1.0 ...
        m_sysVar.push_back("MJB_pta70_neg"); m_sysTool.push_back( 4 ); m_sysToolIndex.push_back( 70 ); m_sysSign.push_back(0);
      }
      m_sysVar.push_back("MJB_ptt30_pos"); m_sysTool.push_back( 5 ); m_sysToolIndex.push_back( 30 ); m_sysSign.push_back(1);
      m_sysVar.push_back("MJB_ptt20_neg"); m_sysTool.push_back( 5 ); m_sysToolIndex.push_back( 20 ); m_sysSign.push_back(0);
      if (m_MJBIteration > 0 && m_MJBStatsOn){
        m_sysVar.push_back("MJB_stat0_pos"); m_sysTool.push_back( 6 ); m_sysToolIndex.push_back( 0  ); m_sysSign.push_back(1);
        m_sysVar.push_back("MJB_stat0_neg"); m_sysTool.push_back( 6 ); m_sysToolIndex.push_back( 0  ); m_sysSign.push_back(0);
        m_sysVar.push_back("MJB_stat1_pos"); m_sysTool.push_back( 6 ); m_sysToolIndex.push_back( 1  ); m_sysSign.push_back(1);
        m_sysVar.push_back("MJB_stat1_neg"); m_sysTool.push_back( 6 ); m_sysToolIndex.push_back( 1  ); m_sysSign.push_back(0);
      }

      //////// These are not implemented ///////
     // m_sysVar.push_back("MJB_sherpa");  m_sysSign.push_back(1);
     // m_sysVar.push_back("MJB_powheg");  m_sysSign.push_back(1);
     // m_sysVar.push_back("MJB_pythia");  m_sysSign.push_back(1);
     // m_sysVar.push_back("MJB_herwig");  m_sysSign.push_back(1);

    }

//!!//// These are for applying falvor response to one jet at a time, for first 3 jets. Not used now?
//!!////??  m_sysVar.push_back("EIC_flvcomp2_pos");  m_sysSign.push_back(1);
//!!////??  m_sysVar.push_back("EIC_flvresp2_pos");  m_sysSign.push_back(1);
//!!////??  m_sysVar.push_back("EIC_flvcomp3_pos");  m_sysSign.push_back(1);
//!!////??  m_sysVar.push_back("EIC_flvresp3_pos");  m_sysSign.push_back(1);
//!!////??  m_sysVar.push_back("EIC_flvcomp4_pos");  m_sysSign.push_back(1);
//!!////??  m_sysVar.push_back("EIC_flvresp4_pos");  m_sysSign.push_back(1);
//!!////??  m_sysVar.push_back("EIC_flvcomp2_neg");  m_sysSign.push_back(0);
//!!////??  m_sysVar.push_back("EIC_flvresp2_neg");  m_sysSign.push_back(0);
//!!////??  m_sysVar.push_back("EIC_flvcomp3_neg");  m_sysSign.push_back(0);
//!!////??  m_sysVar.push_back("EIC_flvresp3_neg");  m_sysSign.push_back(0);
//!!////??  m_sysVar.push_back("EIC_flvcomp4_neg");  m_sysSign.push_back(0);
//!!////??  m_sysVar.push_back("EIC_flvresp4_neg");  m_sysSign.push_back(0);
//!!
  }//for varVector
  cout << "Done loading systematics " << endl;
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetAlgorithim :: loadVariationsOld (){
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetAlgorithim :: loadTriggerTool(){

  for(unsigned int iT=0; iT < m_triggers.size(); ++iT){
    TrigConf::xAODConfigTool* tmpTrigConfTool = new TrigConf::xAODConfigTool( ("xAODConfigTool_"+m_triggers.at(iT)).c_str() );
    tmpTrigConfTool->initialize();
    ToolHandle< TrigConf::ITrigConfigTool > configHandle( tmpTrigConfTool );

    Trig::TrigDecisionTool* tmpTrigDecTool = new Trig::TrigDecisionTool( ("TrigDecisionTool_"+m_triggers.at(iT)).c_str() );
    tmpTrigDecTool->setProperty( "ConfigTool", configHandle );
    tmpTrigDecTool->setProperty( "TrigDecisionKey", "xTrigDecision" );
    tmpTrigDecTool->setProperty( "OutputLevel", MSG::INFO);
    tmpTrigDecTool->initialize();

    m_trigConfTools.push_back( tmpTrigConfTool );
    m_trigDecTools.push_back( tmpTrigDecTool );

  }

  return EL::StatusCode::SUCCESS;
}

// initialize and configure the JVT correction tool
EL::StatusCode MultijetAlgorithim :: loadJVTTool(){
  m_JVTTool = new JetVertexTaggerTool("jvtag");
  m_JVTToolHandle = ToolHandle<IJetUpdateJvt>("jvtag");
  RETURN_CHECK("loadJVTTool", m_JVTTool->setProperty("JVTFileName","JetMomentTools/JVTlikelihood_20140805.root"), "");
  RETURN_CHECK("loadJVTTool", m_JVTTool->initialize(), "");

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetAlgorithim :: loadJetCalibrationTool(){

  m_JetCalibrationTool = new JetCalibrationTool("JetCorrectionTool", m_jetDef, m_jetCalibConfig, m_jetCalibSequence, !m_isMC);
  m_JetCalibrationTool->msg().setLevel( MSG::ERROR); // VERBOSE, INFO, DEBUG

  RETURN_CHECK( "loadJetCalibrationTool", m_JetCalibrationTool->initializeTool("JetCorrectionTool"), "");

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetAlgorithim :: setupJetCalibrationStages() {

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

EL::StatusCode MultijetAlgorithim :: loadJetCleaningTool(){

  m_JetCleaningTool = new JetCleaningTool("JetCleaning");
  RETURN_CHECK( "loadJetCleaningTool", m_JetCleaningTool->setProperty( "CutLevel", m_jetCleanCutLevel), "");
  if (m_jetCleanUgly){
    RETURN_CHECK( "JetCalibrator::initialize()", m_JetCleaningTool->setProperty( "DoUgly", true), "");
  }

  RETURN_CHECK( "loadJetCleaningTool", m_JetCleaningTool->initialize(), "JetCleaning Interface succesfully initialized!");

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetAlgorithim :: loadJetUncertaintyTool(){

  m_JetUncertaintiesTool = new JetUncertaintiesTool("JESProvider");
  std::string thisJetDef = "";
  if( m_inContainerName.find("AntiKt4EMTopo") != std::string::npos)
    thisJetDef = "AntiKt4EMTopo";
  else if( m_inContainerName.find("AntiKt6EMTopo") != std::string::npos )
    thisJetDef = "AntiKt6EMTopo";
  else if( m_inContainerName.find("AntiKt4LCTopo") != std::string::npos )
    thisJetDef = "AntiKt4LCTopo";
  else if( m_inContainerName.find("AntiKt6LCTopo") != std::string::npos )
    thisJetDef = "AntiKt6LCTopo";

  RETURN_CHECK( "loadJetUncertaintyTool", m_JetUncertaintiesTool->setProperty("JetDefinition", thisJetDef), "" );
  if(m_isAFII)
    RETURN_CHECK( "loadJetUncertaintyTool", m_JetUncertaintiesTool->setProperty("MCType", "AFII"), "" );
  else
    RETURN_CHECK( "loadJetUncertaintyTool", m_JetUncertaintiesTool->setProperty("MCType", "MC15"), "" );
  RETURN_CHECK( "loadJetUncertaintyTool", m_JetUncertaintiesTool->setProperty("ConfigFile",m_jetUncertaintyConfig), "" );
  RETURN_CHECK( "loadJetUncertaintyTool", m_JetUncertaintiesTool->initialize(), "" );

  m_JetUncertaintiesTool->msg().setLevel( MSG::ERROR ); // VERBOSE, INFO, DEBUG


//  //Setup integer mapping of JetUncertaintiesTool systematics to use
//  std::string jetSystNames[4] = {"EtaIntercalibration_Modelling", "EtaIntercalibration_TotalStat", "Flavor_Composition", "Flavor_Response"};
//   int jetSystNums[4] = {56, 57, 64, 65}; // for JES_2012/Final/InsituJES2012_AllNuisanceParameters.config
//  //int jetSystNums[4] = {3, 4, 11, 12}; // for JES_2012/Final/InsituJES2012_3NP_Scenario1.config
//
//  for(unsigned int iJESVar=0; iJESVar < 4; ++iJESVar){
//    std::string thisJetSystName = jetSystNames[iJESVar];
//    int thisJetSystNum = jetSystNums[iJESVar];
//    for(unsigned int iVar=0; iVar < m_sysVar.size(); ++iVar){
//      if( m_sysVar.at(iVar).find( thisJetSystName ) != std::string::npos )
//        m_JESMap[iVar] = thisJetSystNum;
//    }//iVar
//  }//iJESVar

  return EL::StatusCode::SUCCESS;
}

//Setup V+jet calibration and systematics files
EL::StatusCode MultijetAlgorithim :: loadVjetCalibration(){

  std::string containerType;
  if( m_inContainerName.find("AntiKt4EMTopo") != std::string::npos)
    containerType = "EMJES_R4";
  else if( m_inContainerName.find("AntiKt6EMTopo") != std::string::npos )
    containerType = "EMJES_R6";
  else if( m_inContainerName.find("AntiKt4LCTopo") != std::string::npos )
    containerType = "LCJES_R4";
  else if( m_inContainerName.find("AntiKt6LCTopo") != std::string::npos )
    containerType = "LCJES_R6";

  TFile *VjetFile = TFile::Open( gSystem->ExpandPathName("$ROOTCOREBIN/data/MultijetBalanceAlgo/Vjet/Vjet_Systematics.root") , "READ" );
  TIter next(VjetFile->GetListOfKeys());
  TKey* key;
  std::string thisKeyName;

  while ( (key = (TKey*) next()) ){
    thisKeyName = key->GetName();
    if( thisKeyName.find(containerType) != std::string::npos ){

      /////Save histogram to memory in m_VjetHists
      TH1F *h = (TH1F*) VjetFile->Get(thisKeyName.c_str());
//      TH1F *h = (TH1F*) key->Clone(); //This didn't work!
      h->SetDirectory(0); //Detach histogram from file to memory
      m_VjetHists.push_back(h);

      ///// Integer mapping of systematics file with m_sysVar variation
      //Map this latest histogram index to it's algorithim index
      thisKeyName = thisKeyName.substr( thisKeyName.find("SystError_")+10 , thisKeyName.size() );
      for(unsigned int iVar=0; iVar < m_sysVar.size(); ++iVar){
        if( m_sysVar.at(iVar).find(thisKeyName) != std::string::npos){
          m_VjetMap[iVar] = m_VjetHists.size()-1;
        }
      }//iVar

    }
  }//while key

  VjetFile->Close();

  return EL::StatusCode::SUCCESS;
}

//Setup Previous MJB calibration and systematics files
EL::StatusCode MultijetAlgorithim :: loadMJBCalibration(){

  if(m_MJBIteration == 0 && !m_closureTest)
    return EL::StatusCode::SUCCESS;

  TFile* MJBFile = TFile::Open( gSystem->ExpandPathName( ("$ROOTCOREBIN/data/MultijetBalanceAlgo/"+m_MJBCorrectionFile).c_str() ), "READ" );
  if (m_closureTest)
    m_ss << m_MJBIteration;
  else
    m_ss << (m_MJBIteration-1);

  std::string mjbIterPrefix = "Iteration"+m_ss.str()+"_";
  std::string histPrefix = "DoubleMJB";
  if (m_leadJetMJBCorrection)
    histPrefix += "_leadJet";


  for(unsigned int iVar=0; iVar < m_sysVar.size(); ++iVar){
    TH1D *MJBHist;
    if( m_sysVar.at(iVar).find("JCS") == std::string::npos && m_sysVar.at(iVar).find("MJB_stat") == std::string::npos ){
      MJBHist = (TH1D*) MJBFile->Get( (mjbIterPrefix+m_sysVar.at(iVar)+"/"+histPrefix+m_MJBCorrectionBinning).c_str() );
      if (! MJBHist){ //if it doesn't exists !!
        Error("loadMJBCalibration()", "Can't find Systematic Variation %s%s. Exiting...", mjbIterPrefix.c_str(), m_sysVar.at(iVar).c_str() );
        return EL::StatusCode::FAILURE;
      }
      MJBHist->SetDirectory(0); //Detach historam from file to memory
    }
    m_MJBHists.push_back(MJBHist);
  }

  m_ss.str( std::string() );
  MJBFile->Close();

return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetAlgorithim :: applyJetCalibrationTool( xAOD::Jet* jet){
  if ( m_JetCalibrationTool->applyCorrection( *jet ) == CP::CorrectionCode::Error ) {
    Error("execute()", "JetCalibrationTool reported a CP::CorrectionCode::Error");
    Error("execute()", "%s", m_name.c_str());
    return StatusCode::FAILURE;
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetAlgorithim :: applyJetUncertaintyTool( xAOD::Jet* jet , int iVar ){
  if(m_debug) Info("execute()", "applyJetUncertaintyTool ");

  if( ( m_isMC ) //JetUncertaintyTool doesn't apply to MC
    || ( m_sysTool.at(iVar) != 0 ) ) //If not JetUncertaintyTool
    return EL::StatusCode::SUCCESS;

  if( !m_noLimitJESPt && (jet->pt() > m_maxSub.at(0)) ){  //Can't be above 800 GeV
    return EL::StatusCode::SUCCESS;
  }

  // These should be taken care of by the tool? !!
//  if( (m_JESMap[iVar] == 64 || m_JESMap[iVar] == 65) && jet->pt() < 20.*GeV)  //Flavor pt limit
//    return EL::StatusCode::SUCCESS;
//  else if( (m_JESMap[iVar] == 56 || m_JESMap[iVar] == 57) && jet->pt() < 15.*GeV)  //EtaIntercalibration pt limit
//    return EL::StatusCode::SUCCESS;

  float thisUncertainty = 1.;
  if( m_sysSign.at(iVar) == 1)
    thisUncertainty += m_JetUncertaintiesTool->getUncertainty(m_sysToolIndex.at(iVar), *jet);
  else
    thisUncertainty -= m_JetUncertaintiesTool->getUncertainty(m_sysToolIndex.at(iVar), *jet);

  TLorentzVector thisJet;
  thisJet.SetPtEtaPhiE( jet->pt(), jet->eta(), jet->phi(), jet->e() );
  thisJet *= thisUncertainty;
  jet->auxdata< float >("pt") = thisJet.Pt();
  jet->auxdata< float >("eta") = thisJet.Eta();
  jet->auxdata< float >("phi") = thisJet.Phi();
  jet->auxdata< float >("e") = thisJet.E();

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetAlgorithim :: applyVjetCalibration( xAOD::Jet* jet , int iVar ){
  if(m_debug) Info("execute()", "applyVjetCalibration ");

  if(m_isMC)
    return EL::StatusCode::SUCCESS;

  if( (m_sysTool.at(iVar) == 1) || jet->pt() < 17.*GeV ) //If NoCorr or not in V+jet correction range
    return EL::StatusCode::SUCCESS;
  if( !m_noLimitJESPt && jet->pt() > m_maxSub.at(0) )
    return EL::StatusCode::SUCCESS;


  float thisCalibration = 1.;
  //Get nominal V+jet correction
  thisCalibration = m_VjetHists.at(0)->GetBinContent( m_VjetHists.at(0)->FindBin(jet->pt()/GeV) );

  //If this systematic variation is for V+jet
  if( m_VjetMap.find(iVar) != m_VjetMap.end() ){
    //cout << "Content is " <<  m_VjetHists.at(m_VjetMap[iVar])->GetBinContent( m_VjetHists.at(m_VjetMap[iVar])->FindBin(jet->pt()/GeV) ) << " for bin " << m_VjetHists.at(m_VjetMap[iVar])->FindBin(jet->pt()/GeV) << " for pt " << jet->pt()/GeV << endl;
    //cout << "nominal correction is " << thisCalibration << endl;
    if( m_sysSign.at(iVar) == 1){
      thisCalibration += m_VjetHists.at(m_VjetMap[iVar])->GetBinContent( m_VjetHists.at(m_VjetMap[iVar])->FindBin(jet->pt()/GeV) );
    }else{
      thisCalibration -= m_VjetHists.at(m_VjetMap[iVar])->GetBinContent( m_VjetHists.at(m_VjetMap[iVar])->FindBin(jet->pt()/GeV) );
    }
    //cout << "final correction for " << m_sysVar.at(iVar) << " is " << thisCalibration << endl;
  }

  TLorentzVector thisJet;
  thisJet.SetPtEtaPhiE(jet->pt(), jet->eta(), jet->phi(), jet->e());
  thisJet *= (1./thisCalibration); //modify TLV
  //(**jet) *= (1./thisCalibration); //following Gagik...
  jet->auxdata< float >("pt") = thisJet.Pt();
  jet->auxdata< float >("eta") = thisJet.Eta();
  jet->auxdata< float >("phi") = thisJet.Phi();
  jet->auxdata< float >("e") = thisJet.E();

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode MultijetAlgorithim :: applyMJBCalibration( xAOD::Jet* jet , int iVar, bool isLead /*=false*/ ){

  if(m_isMC)
    return EL::StatusCode::SUCCESS;

  //No correction for first iteration
  if (m_MJBIteration == 0 && !m_closureTest)
    return EL::StatusCode::SUCCESS;

  //If it's lead jet but not the closure test
  if( isLead && !m_closureTest)
    return EL::StatusCode::SUCCESS;

  // if it's a subleading jet but below the 800 GeV limit
  if( !isLead && jet->pt() <= m_maxSub.at(0))
    return EL::StatusCode::SUCCESS;

  // If it's for a subcalibration of JCS, don't apply this calibration
  if(m_sysTool.at(iVar) == 1)
    return EL::StatusCode::SUCCESS;


//cout << "Applying MJB calibration!!!!" << endl;
  float thisCalibration = 1. / m_MJBHists.at(iVar)->GetBinContent( m_MJBHists.at(iVar)->FindBin(jet->pt()/GeV) );

  // MJB Statistical Systematic //
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

  TLorentzVector thisJet;
  thisJet.SetPtEtaPhiE(jet->pt(), jet->eta(), jet->phi(), jet->e());
  thisJet *= thisCalibration; //modify TLV
  //(**jet) *= (thisCalibration); //following Gagik...
//cout << "pt " << jet->pt() << " -> " << thisJet.Pt() << endl;
  jet->auxdata< float >("pt") = thisJet.Pt();
  jet->auxdata< float >("eta") = thisJet.Eta();
  jet->auxdata< float >("phi") = thisJet.Phi();
  jet->auxdata< float >("e") = thisJet.E();


  return EL::StatusCode::SUCCESS;
}


EL::StatusCode MultijetAlgorithim :: reorderJets( std::vector< xAOD::Jet*>* theseJets ){

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

