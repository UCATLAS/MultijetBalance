#ifndef MultijetBalanceAlgo_MultijetAlgorithim_H
#define MultijetBalanceAlgo_MultijetAlgorithim_H

#include <EventLoop/StatusCode.h>
#include <EventLoop/Algorithm.h>

// Infrastructure include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"

// ROOT include(s):
#include "TH1D.h"

// inlude the parent class header for tree
#ifndef __MAKECINT__
#include "MultijetBalanceAlgo/MiniTree.h"
#endif

#include <sstream>

#include "JetMomentTools/JetVertexTaggerTool.h"


static float GeV = 1000.;

class MultijetHists;
class JetCalibrationTool;
class JetCleaningTool;
class JetUncertaintiesTool;
namespace TrigConf {
  class xAODConfigTool;
}
namespace Trig {
class TrigDecisionTool;
}

class MultijetAlgorithim : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
  public:
    // float cutValue;
    xAOD::TEvent *m_event;  //!
    xAOD::TStore *m_store;  //!
    int m_eventCounter;     //!

    std::string m_name;
    std::string m_configName;
    std::vector<TH1D*> m_cutflowHist;    //!
    std::vector<TH1D*> m_cutflowHistW;   //!
    unsigned int m_cutflowFirst;     //!
    unsigned int m_iCutflow;     //!
    float m_mcEventWeight; //!
    std::string m_comEnergy; //!

    //configuration variables
    std::string m_inContainerName;        // input container name
    bool m_debug;                     // set verbose mode
    bool m_isMC;                      // Is MC
    bool m_isAFII;                      // Is AFII
    bool m_isDAOD;                    // Is DAOD, not original AOD
    bool m_is8TeV;                    // Is 8TeV, not 13TeV
    int m_maxEvent;                   // Maximum number of events to run on
    bool m_useCutFlow;                // true will write out cutflow histograms
    bool m_writeTree;                 // true will write out a TTree
    bool m_writeNominalTree;          // true will write out only the Nominal TTree
    std::string m_eventDetailStr;            // Will print out extra histograms
    std::string m_jetDetailStr;              // Will print out extra histograms
    std::string m_MJBDetailStr;              // Will print out extra histograms
    int m_MJBIteration;               // Number of previous MJB iterations
    std::string m_MJBCorrectionFile;
    std::string m_MJBCorrectionBinning;
    std::string m_varString;                  // Systematic Variations to use
    std::string m_MCPileupCheckContainer; // Name of truth container for MC Pileup Check
    bool m_useMCPileupCheck;
    bool m_leadJetMJBCorrection;
    bool m_closureTest;
    float m_triggerThreshold;
    float m_JVTCut;

    //config for Jet Tools
    std::string m_jetDef;
    std::string m_jetCalibSequence;
    std::string m_jetCalibConfig;
    std::string m_jetCleanCutLevel;
    bool m_jetCleanUgly;
    std::string m_jetUncertaintyConfig;

  private:



    #ifndef __CINT__
    JetCalibrationTool   * m_JetCalibrationTool;    //!
    JetCleaningTool      * m_JetCleaningTool;       //!
    JetUncertaintiesTool * m_JetUncertaintiesTool;  //!
    #endif // not __CINT__

    TrigConf::xAODConfigTool       * m_trigConfTool;          //!
    Trig::TrigDecisionTool     * m_trigDecTool;           //!

    //JVTTool
    JetVertexTaggerTool      * m_JVTTool;        //!
    ToolHandle<IJetUpdateJvt>  m_JVTToolHandle;  //!

    int m_pvLocation; //!

    float m_xs; //!
    float m_acceptance; //!
    int m_numAMIEvents; //!
    int m_mcChannelNumber; //!
    int m_runNumber; //!
    std::stringstream m_ss; //!

    std::vector<int> m_maxSub; //!
    std::vector<std::string> m_algVar; //!
    int m_NominalIndex; //!
    int m_NoCorrIndex; //!
    std::vector<int> m_algVarPos; //!
    std::vector< TH1F* > m_VjetHists; //!
    std::vector< TH1D* > m_MJBHists; //!
    std::map<int, int> m_VjetMap; //!
    std::map<int, int> m_JESMap; //!
    std::vector<int> m_JCSIndex; //!
    std::vector<std::string> m_JCSTokens; //!
    std::vector<std::string> m_JCSStrings; //!

    EL::StatusCode passCutAll();
    EL::StatusCode passCut(int iVar);

    double DeltaPhi(double phi1, double phi2);
    double DeltaR(double eta1, double phi1,double eta2, double phi2);



    #ifndef __MAKECINT__

    //MiniTree* m_nominalTree; //!
    std::vector<MiniTree*> m_treeList; //!
    EL::StatusCode getLumiWeights(const xAOD::EventInfo* eventInfo);
    std::vector<MultijetHists*> m_jetHists; //!

    #endif

  EL::StatusCode loadVariations();
  EL::StatusCode loadTriggerTool();
  EL::StatusCode loadJVTTool();
  EL::StatusCode loadJetCalibrationTool();
  EL::StatusCode setupJetCalibrationStages();
  EL::StatusCode loadJetCleaningTool();
  EL::StatusCode loadJetUncertaintyTool();
  EL::StatusCode loadVjetCalibration();
  EL::StatusCode loadMJBCalibration();

    #ifndef __MAKECINT__
     EL::StatusCode applyJetCalibrationTool( xAOD::Jet* jet);
     EL::StatusCode applyJetCleaningTool();
     EL::StatusCode applyJetUncertaintyTool( xAOD::Jet* jet , int iVar );
     EL::StatusCode applyVjetCalibration( xAOD::Jet* jet , int iVar );
     EL::StatusCode applyMJBCalibration( xAOD::Jet* jet , int iVar, bool isLead = false );
     EL::StatusCode reorderJets(std::vector< xAOD::Jet*>* signalJets);

    #endif

  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:
  // Tree *myTree; //!
  // TH1 *myHist; //!




  // this is a standard constructor
  MultijetAlgorithim ();
  MultijetAlgorithim (std::string name, std::string configName);
  ~MultijetAlgorithim ();

  // these are the functions inherited from Algorithm
  virtual EL::StatusCode setupJob (EL::Job& job);
  virtual EL::StatusCode fileExecute ();
  virtual EL::StatusCode histInitialize ();
  virtual EL::StatusCode changeInput (bool firstFile);
  virtual EL::StatusCode initialize ();
  virtual EL::StatusCode execute ();
  virtual EL::StatusCode postExecute ();
  virtual EL::StatusCode finalize ();
  virtual EL::StatusCode histFinalize ();

  // these are the functions not inherited from Algorithm
  virtual EL::StatusCode configure ();

  // this is needed to distribute the algorithm to the workers
  ClassDef(MultijetAlgorithim, 1);
};

#endif
