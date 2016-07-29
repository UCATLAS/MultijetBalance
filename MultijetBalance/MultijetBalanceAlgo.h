#ifndef MultijetBalance_MultijetBalanceAlgo_H
#define MultijetBalance_MultijetBalanceAlgo_H

/** @file MultijetBalanceAlgo.h
 *  @brief Run the Multijet Balance Selection
 *  @author Jeff Dandoy
 *  @bug No known bugs
 */

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
#include "MultijetBalance/MiniTree.h"
#endif

#include <sstream>

#include "JetMomentTools/JetVertexTaggerTool.h"

#include "xAODBTaggingEfficiency/BTaggingSelectionTool.h"
#include "xAODBTaggingEfficiency/BTaggingEfficiencyTool.h"


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

class SystContainer;

/**
    @brief Event selection of the Multijet Balance
 */

// variables that don't get filled at submission time should be
// protected from being send from the submission node to the worker
// node (done by the //!)
class MultijetBalanceAlgo : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
  public:

    /** @brief TEvent object */
    xAOD::TEvent *m_event;  //!
    /** @brief TStore object for variable storage*/
    xAOD::TStore *m_store;  //!
    /** @brief Count of the current event*/
    int m_eventCounter;     //!
    /** @brief Global name to give to the algorithm*/
    std::string m_name;

//No longer used:    std::string m_configName;
    /** @brief */
    float m_mcEventWeight; //!
    /** @brief */
    std::string m_comEnergy; //!

    // configuration variables set by user
    /** @brief */
    std::string m_inContainerName;        // input container name
    /** @brief */
    std::string m_triggerAndPt;
    /** @brief */
    int m_MJBIteration;               // Number of previous MJB iterations
    /** @brief */
    std::string m_MJBIterationThreshold;
    /** @brief */
    std::string m_MJBCorrectionFile;
    /** @brief */
    std::string m_MJBBinningName;
    /** @brief */
    std::string m_sysVariations;                  // Systematic Variations to use
    /** @brief */
    bool m_MJBStatsOn;
    /** @brief */
    unsigned int m_numJets;
    /** @brief */
    float m_ptAsym;
    /** @brief */
    float m_alpha;
    /** @brief */
    float m_beta;
    /** @brief */
    float m_ptThresh;
    /** @brief */
    bool m_allJetBeta;
    /** @brief */
    bool m_bootstrap;
    /** @brief */
    bool m_leadingInsitu;
    /** @brief */
    bool m_noLimitJESPt;
    /** @brief */
    bool m_closureTest;
    /** @brief */
    bool m_leadJetMJBCorrection;
    /** @brief */
    bool m_reverseSubleading;
    /** @brief True value will allow TTree output */
    bool m_writeTree;
    /** @brief True value will write out only the Nominal TTree */
    bool m_writeNominalTree;
    /** @brief Control substrings for creating MJB histograms */
    std::string m_MJBDetailStr;
    /** @brief Control substrings for creating event-level histograms */
    std::string m_eventDetailStr;
    /** @brief Control substrings for creating jet-level histograms */
    std::string m_jetDetailStr;
    /** @brief Control substrings for creating trigger-level histograms */
    std::string m_trigDetailStr;
    /** @brief Set verbose mode */
    bool m_debug;
    /** @brief Maximum number of events to run over */
    int m_maxEvent;
    /** @brief Name of the truth xAOD container for MC Pileup Check */
    std::string m_MCPileupCheckContainer;
    /** @brief Setting for ATLAS Fastsim production samples */
    bool m_isAFII;
    /** @brief Setting for Derived xAODs, rather than original xAODs */
    bool m_isDAOD;
    /** @brief Option to output the cutflow histograms */
    bool m_useCutFlow;

    /** @brief */
    int m_systTool_nToys;
    /** @brief */
    std::string m_binning;
    /** @brief */
    std::string m_VjetCalibFile;

    /** @brief */
    bool m_bTag;
    /** @brief */
    std::string m_bTagWPsString;
    /** @brief */
    std::vector<std::string> m_bTagWPs;
    /** @brief */
    std::string m_bTagFileName;
    /** @brief */
    std::string m_bTagVar;
    /** @brief */
    bool m_useDevelopmentFile;
    /** @brief */
    bool m_useConeFlavourLabel;

    // configuration variables set automatically
    /** @brief */
    bool m_isMC;                      // Is MC
    /** @brief */
    std::vector<float> m_subLeadingPtThreshold;
    /** @brief */
    bool m_useMCPileupCheck;
    /** @brief */
    bool m_iterateBootstrap;
    /** @brief */
    std::vector<std::string> m_triggers;
    /** @brief */
    std::vector<float> m_triggerThresholds;
    /** @brief */
    std::vector<double> m_bins;
    /** @brief */
    bool m_VjetCalib;

    //config for Jet Tools
    /** @brief */
    std::string m_jetDef;
    /** @brief */
    std::string m_jetCalibSequence;
    /** @brief */
    std::string m_jetCalibConfig;
    /** @brief */
    std::string m_jetCleanCutLevel;
    /** @brief */
    bool m_jetCleanUgly;
    /** @brief */
    float m_JVTCut;
    /** @brief */
    std::string m_jetUncertaintyConfig;

    // for SystTool
    /** @brief */
    SystContainer  * systTool; //!
    /** @brief */
    std::vector<double> systTool_ptBins; //!

  private:



    #ifndef __CINT__
    /** @brief */
    JetCalibrationTool   * m_JetCalibrationTool;    //!
    /** @brief */
    JetCleaningTool      * m_JetCleaningTool;       //!
    /** @brief */
    JetUncertaintiesTool * m_JetUncertaintiesTool;  //!
    #endif // not __CINT__

    /** @brief */
    std::vector< BTaggingSelectionTool* >  m_BJetSelectTools; //!
    /** @brief */
    std::vector< BTaggingEfficiencyTool* > m_BJetEffSFTools; //!

    /** @brief */
    std::vector< TrigConf::xAODConfigTool* > m_trigConfTools; //!
    /** @brief */
    std::vector< Trig::TrigDecisionTool* > m_trigDecTools;    //!

    //JVTTool
    /** @brief */
    JetVertexTaggerTool      * m_JVTTool;        //!
    /** @brief */
    ToolHandle<IJetUpdateJvt>  m_JVTToolHandle;  //!

    /** @brief */
    int m_pvLocation; //!

    /** @brief */
    float m_xs; //!
    /** @brief */
    float m_acceptance; //!
    /** @brief */
    int m_numAMIEvents; //!
    /** @brief */
    int m_mcChannelNumber; //!
    /** @brief */
    int m_runNumber; //!
    /** @brief */
    std::stringstream m_ss; //!

    /** @brief */
    std::vector<std::string> m_sysVar; //!
    /** @brief */
    std::vector<int> m_sysTool; //!
    /** @brief */
    std::vector<int> m_sysToolIndex; //!
    /** @brief */
    std::vector<int> m_sysSign; //!
    /** @brief */
    int m_NominalIndex; //!

    /** @brief */
    std::vector< TH1F* > m_VjetHists; //!
    /** @brief */
    std::vector< TH1D* > m_MJBHists; //!
    /** @brief */
    std::map<int, int> m_VjetMap; //!
    /** @brief */
    std::map<int, int> m_JESMap; //!
    /** @brief */
    std::vector<std::string> m_JCSTokens; //!
    /** @brief */
    std::vector<std::string> m_JCSStrings; //!

    /** @brief Update every cutflow for this selection
        @note The current selection is determined by the variable m_iCutflow, which automatically updates with each use
    */
    EL::StatusCode passCutAll();
    /** @brief Update cutflow iVar for this selection
        @note The current selection is determined by the variable m_iCutflow, which automatically updates with each use
        @param iVar  The index of the current systematic variation whose cutflow is to be filled
    */
    EL::StatusCode passCut(int iVar);

  public:
    /** @brief Vector of cutflows, for each systematic variation, showing the integer number of events passing each selection */
    std::vector<TH1D*> m_cutflowHist;    //!
    /** @brief Vector of weighted cutflows, for each systematic variation, showing the weighted number of events passing each selection */
    std::vector<TH1D*> m_cutflowHistW;   //!
    /** @brief  Bin index corresponding to the first new selection
     * @note  The cutflow histograms are grabbed from previous xAH algorithms and already include entries.
     * The first location of the new MJB entr is stored here.
     * */
    unsigned int m_cutflowFirst;     //!
    /** @brief Automatically updating index of the current selection, used by passCutAll() and passCut() */
    unsigned int m_iCutflow;     //!

  private:
    /** @brief */
    double DeltaPhi(double phi1, double phi2);
    /** @brief */
    double DeltaR(double eta1, double phi1,double eta2, double phi2);



    #ifndef __MAKECINT__

    /** @brief */
    std::vector<MiniTree*> m_treeList; //!
    /** @brief */
    EL::StatusCode getLumiWeights(const xAOD::EventInfo* eventInfo);
    /** @brief */
    std::vector<MultijetHists*> m_jetHists; //!

    #endif

    /** @brief */
    EL::StatusCode loadVariations();
    /** @brief */
    EL::StatusCode loadVariationsOld();
    /** @brief */
    EL::StatusCode loadTriggerTool();
    /** @brief */
    EL::StatusCode loadJVTTool();
    /** @brief */
    EL::StatusCode loadJetCalibrationTool();
    /** @brief */
    EL::StatusCode setupJetCalibrationStages();
    /** @brief */
    EL::StatusCode loadJetCleaningTool();
    /** @brief */
    EL::StatusCode loadJetUncertaintyTool();
    /** @brief */
    EL::StatusCode loadVjetCalibration();
    /** @brief */
    EL::StatusCode loadMJBCalibration();
    /** @brief */
    EL::StatusCode loadBTagTools();

    #ifndef __MAKECINT__
    /** @brief */
     EL::StatusCode applyJetCalibrationTool( xAOD::Jet* jet);
    /** @brief */
     EL::StatusCode applyJetCleaningTool();
    /** @brief */
     EL::StatusCode applyJetUncertaintyTool( xAOD::Jet* jet , int iVar );
    /** @brief */
     EL::StatusCode applyVjetCalibration( xAOD::Jet* jet , int iVar );
    /** @brief */
     EL::StatusCode applyMJBCalibration( xAOD::Jet* jet , int iVar, bool isLead = false );
    /** @brief */
     EL::StatusCode reorderJets(std::vector< xAOD::Jet*>* signalJets);

    #endif

public:




    /** @brief Standard constructor*/
    MultijetBalanceAlgo (std::string name = "MultijetBalanceAlgo");
    /** @brief Standard destructor*/
    ~MultijetBalanceAlgo ();

  // these are the functions inherited from Algorithm
    /** @brief Setup the job (inherits from Algorithm)*/
    virtual EL::StatusCode setupJob (EL::Job& job);
    /** @brief Execute the file (inherits from Algorithm)*/
    virtual EL::StatusCode fileExecute ();
    /** @brief Initialize the output histograms before any input file is attached (inherits from Algorithm)*/
    virtual EL::StatusCode histInitialize ();
    /** @brief Change to the next input file (inherits from Algorithm)*/
    virtual EL::StatusCode changeInput (bool firstFile);
    /** @brief Initialize the input file (inherits from Algorithm)*/
    virtual EL::StatusCode initialize ();
    /** @brief Execute each event of the input file (inherits from Algorithm)*/
    virtual EL::StatusCode execute ();
    /** @brief End the event execution (inherits from Algorithm)*/
    virtual EL::StatusCode postExecute ();
    /** @brief Finalize the input file (inherits from Algorithm)*/
    virtual EL::StatusCode finalize ();
    /** @brief Finalize the histograms after all files (inherits from Algorithm)*/
    virtual EL::StatusCode histFinalize ();

  // these are the functions not inherited from Algorithm
    /** @brief Configure the variables once before running*/
    virtual EL::StatusCode configure ();

    /** @brief Used to distribute the algorithm to the workers*/
    ClassDef(MultijetBalanceAlgo, 1);
};

#endif
