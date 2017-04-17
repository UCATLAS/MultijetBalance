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


#include "AsgTools/AnaToolHandle.h"

#include "JetInterface/IJetSelector.h" //JetCleaningTool
#include "JetCalibTools/IJetCalibrationTool.h"
#include "JetCPInterfaces/ICPJetUncertaintiesTool.h"
#include "JetResolution/IJERTool.h"
#include "JetResolution/IJERSmearingTool.h"
#include "JetInterface/IJetUpdateJvt.h"
#include "JetJvtEfficiency/IJetJvtEfficiency.h"

#include "xAODBTaggingEfficiency/IBTaggingEfficiencyTool.h"
#include "xAODBTaggingEfficiency/IBTaggingSelectionTool.h"

#include "JetCPInterfaces/IJetTileCorrectionTool.h"

static float GeV = 1000.;

class MultijetHists;
class IJetCalibrationTool;
class ICPJetUncertaintiesTool;
class IJERTool;
class IJERSmearingTool;
class IJetSelector;
class IJetUpdateJvt;
class IBTaggingEfficiencyTool;
class IBTaggingSelectionTool;

class SystContainer;
class IJetTileCorrectionTool;

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

    //TODO replace this with name from config
    /** @brief Center of mass energy used to define which file the cross-sections come from*/
    std::string m_comEnergy; //!

////// configuration variables set by user //////
    /** @brief Input container name for jet collection*/
    std::string m_inContainerName;
    /** @brief String consisting of triggers and their recoil \f$p_{T}\f$ thresholds
     *  @note Each trigger / recoil \f$p_{T}\f$ combination is separated by a colon ":",
     *  and each combination is separated by a comma ",".
     * */
    std::string m_triggerAndPt;
    /** @brief The iteration of the current MJB
     * @note MJB is an iterative procedure, with each iteration using outputs from the previous.
     * The procedure starts at a m_MJBIteration value of 0, and is increased by 1 for each iteration.
     * This value corresponds directly to the entries in MultijetBalanceAlgo#m_MJBIterationThreshold.
     * */
    int m_MJBIteration;               // Number of previous MJB iterations
    /** @brief A comma separated list of subleading jet \f$p_{T}\f$ thresholds
     * @note The comma separated list is translated into a vector of values.
     * Each vector entry corresponds to a potential MJB iteration, and the value used is chosen
     * by MultijetBalanceAlgo#m_MJBIteration.
     * */
    std::string m_MJBIterationThreshold;
    /** @brief The location of the file containing previous MJB calibrations
     * @note The file corresponds to previous MJB calibrations and will not be used for a MultijetBalanceAlgo#m_MJBIteration of zero.
     * The naming convention of the calibration histograms should agree with the iteration.
     * */
    std::string m_MJBCorrectionFile;

// TODO Removed    std::string m_MJBBinningName;
//
    /** @brief Dash "-" separated list of systematic variations to use
     * @note Systematic variations are chosen according to their name in the MJB or their respective calibration tools.
     * For example including "EtaIntercalibration" will grab the JetUncertainties \f$\eta\f$-intercalibration systematics.
     * Short-cut strings are also accepted, including:
     *   - Nominal : Include the nominal result
     *   - AllSystematic : Include every defined systematic
     *   - MJB : Include all MJB systematics (defined in MultijetBalanceAlgo#loadVariations)
     *   - AllZjet : Include all Z+jet \a in-situ calibration systematics
     *   - AllGjet : Include all \f$\gamma\f$+jet \a in-situ calibration systematics
     *   - AllXXX  : Include all JetUncertainties systematics include the substring "XXX"
     *   - Special : Include all JetUncertainties systematics including EtaIntercalibration, Pileup, Flavor, or PunchThrough
     *   - JetCalibSequence : Include each stage of the jet calibration procedure (origin, etaJES, GSC, etc.)
     * */
    std::string m_sysVariations;
    /** @brief Apply previous MJB calibration to the leading jet, performing a closure test*/
    bool m_closureTest;
    /** @brief Run in validation mode, applying full insitu calibration to all jets (See Instructions)*/
    bool m_validation;
    /** @brief Run in Bootstrap mode (See Instructions)*/
    bool m_bootstrap;
    /** @brief Use MJB statistical uncertainties
     * @note This is untested, do not use!*/
    bool m_MJBStatsOn;
    /** @brief Selection for the minimum number of jets in the event (Default of 3)*/
    unsigned int m_numJets;
    /** @brief Selection for the \f$p_{T}\f$ asymmetry of the event (Default 0.8)*/
    float m_ptAsym;
    /** @brief Selection for the \f$\alpha\f$ of the event (Default 0.3)*/
    float m_alpha;
    /** @brief Selection for the \f$\beta\f$ of any jet in the event (Default 1.0)*/
    float m_beta;
    /** @brief \f$p_{T}\f$ threshold for each jet to be included (default 25 GeV)*/
    float m_ptThresh;
    /** @brief Boolean requiring each jet to pass the MultijetBalanceAlgo#m_beta selection only for
     * jets with \f$p_{T}\f$ > 25% of the leading jet \f$p_{T}\f$ */
    bool m_looseBetaCut;
    /** @brief Apply the GSC-stage calibration to the leading jet when calibrating.  This should only be used
     * if a special eta-intercalibration stage insitu file is not available, and is a close approximate.  
     * @note It is an exact approximation if the leading jet detEta cut is changed from 1.2 to 0.8 */
    bool m_leadingGSC;
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
    /** @brief Name of the truth xAOD container for MC Pileup Check, set to "None" if not used */
    std::string m_MCPileupCheckContainer;
    /** @brief Setting for ATLAS Fastsim production samples */
    bool m_isAFII;
    /** @brief Option to output the cutflow histograms */
    bool m_useCutFlow;
    /** @brief File containing the MC sample cross-section values */
    std::string m_XSFile;
    /** @brief Apply the Tile Correction to data */
    bool m_TileCorrection;

    /** @brief Number of toys used by the bootstrapping procedure.  Recommendation is 100*/
    int m_systTool_nToys;
    /** @brief Comma separated list of bin edges for recoil \f$p_{T}\f$*/
    std::string m_binning;
    /** @brief Root file containing intermediate V+jet \a in-situ calibrations, set to empty if no such calibrations are applied */
    std::string m_VjetCalibFile;

    /** @brief Boolean for performing b-tagging on jets*/
    bool m_bTag;
    /** @brief Comma separated list of of b-tag working point efficiency percentages*/
    std::string m_bTagWPsString;
    /** @brief Path to b-tagging CDI file*/
    std::string m_bTagFileName;
    /** @brief Variable for b-taggign (i.e. MV2c20) */
    std::string m_bTagVar;
    /** @brief Propagated option from b-tagging tool*/
    bool m_useDevelopmentFile;
    /** @brief Propagated option from b-tagging tool*/
    bool m_useConeFlavourLabel;

////// configuration variables set automatically //////
    /** @brief If input is MC, as automatically determined from xAOD::EventInfo::IS_SIMULATION*/
    bool m_isMC;
    /** @brief Vector of subleading jet \f$p_{T}\f$ selection thresholds, filled automatically from MultijetBalanceAlgo#m_MJBIterationThreshold*/
    std::vector<float> m_subleadingPtThreshold;
    /** @brief Vector of b-tag working point efficiency percentages, filled automatically from MultijetBalanceAlgo#m_bTag*/
    std::vector<std::string> m_bTagWPs;
    /** @brief Set to true automatically if MultijetBalanceAlgo#m_MCPileupCheckContainer is not "None"*/
    bool m_useMCPileupCheck;
    /** @brief Set to true for iterations beyond the first, as bootstrap mode propagation is handled differently */
    bool m_iterateBootstrap;
    /** @brief Vector of triggers, filled automatically from MultijetBalanceAlgo#m_triggerAndPt*/
    std::vector<std::string> m_triggers;
    /** @brief Vector of trigger thresholds, filled automatically from MultijetBalanceAlgo#m_triggerAndPt*/
    std::vector<float> m_triggerThresholds;
    /** @brief Vector of bins, filled automatically from MultijetBalanceAlgo#m_binning*/
    std::vector<double> m_bins;
    /** @brief Whether to use V+jet intermediate calibrations, set to true if MultijetBalanceAlgo#m_VjetCalibFile.size()*/
    bool m_VjetCalib;

////// config for Jet Tools //////
    /** @brief Jet definition (e.g. AntiKt4EMTopo). Propagated option for Jet Tools */
    std::string m_jetDef;
    /** @brief Calibration sequence for JetCalibTools */
    std::string m_jetCalibSequence;
    /** @brief Configuration file for JetCalibTools*/
    std::string m_jetCalibConfig;
    /** @brief Jet cleaning level (i.e. LooseBad), propagated to JetCleaningTool*/
    std::string m_jetCleanCutLevel;
    /** @brief True to clean ugly jets, propagated to JetCleaningTool*/
    bool m_jetCleanUgly;
    /** @brief JVT working point to apply*/
    std::string m_JVTWP;
    /** @brief Configuration file for JetUncertainties */
    std::string m_jetUncertaintyConfig;
    /** @brief Configuration file for Jet Resolution Tool */
    std::string m_JERUncertaintyConfig;
    /** @brief Apply JER smearing */
    bool m_JERApplySmearing;
    /** @brief JER Systematic mode (Full or Simple)*/
    std::string m_JERSystematicMode;

    /** @brief SystTool object for Boostrap mode*/
    SystContainer  * systTool; //!


  private:



    /** @brief JetCalibrationTool handle*/
    asg::AnaToolHandle<IJetCalibrationTool> m_JetCalibrationTool_handle; //!
    /** @brief JetUncertaintiesTool instance*/
    asg::AnaToolHandle<ICPJetUncertaintiesTool> m_JetUncertaintiesTool_handle; //!

    /** @brief JERTool handle */
    asg::AnaToolHandle<IJERTool> m_JERTool_handle;    //!
    /** @brief JER Smearing Tool handle */
    asg::AnaToolHandle<IJERSmearingTool> m_JERSmearingTool_handle;    //!

    /** @brief JetCleaningTool handle*/
    asg::AnaToolHandle<IJetSelector> m_JetCleaningTool_handle; //!

    /** @brief JetUpdateJvt handle */
    asg::AnaToolHandle<IJetUpdateJvt> m_JVTUpdateTool_handle; //!
    /** @brief JetJvtEfficiency handle*/
    asg::AnaToolHandle<CP::IJetJvtEfficiency> m_JetJVTEfficiencyTool_handle; //!


    /** @brief Vector of BTaggingSelectionTool handles*/
    std::vector< asg::AnaToolHandle<IBTaggingSelectionTool> >  m_AllBTaggingSelectionTool_handles; //!
    /** @brief Vector of BTaggingEfficiencyTool handles*/
    std::vector< asg::AnaToolHandle<IBTaggingEfficiencyTool> > m_AllBTaggingEfficiencyTool_handles; //!


    asg::AnaToolHandle<CP::IJetTileCorrectionTool>  m_JetTileCorrectionTool_handle; //!

    /** @brief Location of primary vertex within xAOD::VertexContainer*/
    int m_pvLocation; //!

    /** @brief Event weight from the MC generation*/
    float m_mcEventWeight; //!
    /** @brief AMI cross-section weight, grabbed from file TODO*/
    float m_xs; //!
    /** @brief AMI acceptance weight, grabbed from file TODO*/
    float m_acceptance; //!
    /** @brief AMI number of events, grabbed from file TODO*/
    int m_numAMIEvents; //!
    /** @brief Channel number assigned to the MC sample*/
    int m_mcChannelNumber; //!
    /** @brief Run number assigned to the data*/
    int m_runNumber; //!
    /** @brief A stringstream object for various uses*/
    std::stringstream m_ss; //!

    /** @brief Vector of each individual systematic variation name*/
    std::vector<std::string> m_sysName; //!
    /** @brief Integer corresponding to the type of systematic used.
     * @note Value is  -1 for Nominal, 
     * 0 for JetUncertainties, 1 for JER, 10 for JetCalibSequence, 2 for MJB alpha, 3 for MJB beta, 4 for MJB \f$p_{T}\f$ asymmetry,
     * 5 for MJB \f$p_{T}\f$ threshold, 6 for MJB statistical uncertainty. */
    std::vector<int> m_sysTool; //!
    /** @brief Detail related to the systematic uncertainty.
     * @note For example, this would be the selection value for MJB systematics, or the index of a JES systematic uncertainty in the JetUncertainties tool*/
    std::vector<int> m_sysDetail; //!
    /** @brief CP::SystematicSet for each systematic */ 
    std::vector<CP::SystematicSet> m_sysSet; //!
    /** @brief Position of the Nominal result within m_sysVar */
    int m_NominalIndex; //!

    /** @brief V+jet \a in-situ nominal calibration histogram taken from MultijetBalanceAlgo#m_VjetCalibFile*/
    TH1F* m_VjetHist; //!
    /** @brief MJB \a in-situ calibration histograms from previous iterations taken from MultijetBalanceAlgo#m_MJBCorrectionFile*/
    std::vector< TH1D* > m_MJBHists; //!
//    std::map<int, int> m_VjetMap; //!
//    std::map<int, int> m_JESMap; //!
    /** @brief Substring name given to each JES calibration stage in MultijetBalanceAlgo#m_jetCalibSequence,
     * used for JetCalibSequence option of MultijetBalanceAlgo#m_sysVariations */
    std::vector<std::string> m_JCSTokens; //!
    /** @brief Full name of each JES calibration stage (i.e. JetGSCScaleMomentum). 
     * Has a direct correspondence with MultijetBalanceAlgo#m_JCSTokens*/
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
    /** @brief Calculate the \f$\Delta\phi\f$ between two objects*/
    double DeltaPhi(double phi1, double phi2);
    /** @brief Calculate the \f$\Delta R\f$ between two objects*/
    double DeltaR(double eta1, double phi1,double eta2, double phi2);



    #ifndef __MAKECINT__

    /** @brief Vector of MiniTree objects to output, each corresponding to a different systematic*/
    std::vector<MiniTree*> m_treeList; //!
    /** @brief Retrieve event info from the xAOD::EventInfo object and the file TODO
     * @note: Fills in values for MultijetBalanceAlgo#m_runNumber, MultijetBalanceAlgo#m_mcChannelNumber, MultijetBalanceAlgo#m_xs, MultijetBalanceAlgo#m_acceptance */
    EL::StatusCode getLumiWeights(const xAOD::EventInfo* eventInfo);
    /** @brief Vector of MultijetHists objects to output, each corresponding to a different systematic*/
    std::vector<MultijetHists*> m_jetHists; //!

    #endif

    /** @brief Load all the systematic variations from MultijetBalanceAlgo#m_sysVariations */
    EL::StatusCode loadVariations();
    /** @brief Load the JVT correction Tool*/
    EL::StatusCode loadJVTTool();
    /** @brief Load the JetCalibTool*/
    EL::StatusCode loadJetCalibrationTool();
    /** @brief Load the Jet Resolution Tool*/
    EL::StatusCode loadJetResolutionTool();
    /** @brief Find and connect the jet calibration stages for MultijetBalanceAlgo#m_JCSTokens and MultijetBalanceAlgo#m_JCSStrings*/
    EL::StatusCode setupJetCalibrationStages();
    /** @brief Load the JetCleaningTool*/
    EL::StatusCode loadJetCleaningTool();
    /** @brief Load the JetUncertainties*/
    EL::StatusCode loadJetUncertaintyTool();
    /** @brief Load the JetTileCorrectionTool*/
    EL::StatusCode loadJetTileCorrectionTool();
    /** @brief Load the intermediate V+jet \a in-situ calibration from  MultijetBalanceAlgo#m_VjetCalibFile*/
    EL::StatusCode loadVjetCalibration();
    /** @brief Load the previous MJB calibrations from MultijetBalanceAlgo#m_MJBCorrectionFile*/
    EL::StatusCode loadMJBCalibration();
    /** @brief Load the b-tagging tools*/
    EL::StatusCode loadBTagTools();

    #ifndef __MAKECINT__
    /** @brief Apply the JetCalibTool*/
     EL::StatusCode applyJetCalibrationTool( xAOD::Jet* jet);
    /** @brief Apply the JetTileCorrectionTool*/
     EL::StatusCode applyJetTileCorrectionTool( xAOD::Jet* jet);
    /** @brief Apply the JetCleaningTool*/
     EL::StatusCode applyJetCleaningTool();
    /** @brief Apply the Jet Uncertainty Tool*/
     EL::StatusCode applyJetUncertaintyTool( xAOD::Jet* jet , int iVar );
    /** @brief Apply the Jet Resolution Tool*/
     EL::StatusCode applyJetResolutionTool( xAOD::Jet* jet , int iVar );
    /** @brief Apply the intermediate V+jet \a in-situ calibrations*/
     EL::StatusCode applyVjetCalibration( xAOD::Jet* jet , int iVar );
    /** @brief Apply the MJB calibration from previous iterations */
     EL::StatusCode applyMJBCalibration( xAOD::Jet* jet , int iVar, bool isLead = false );
    /** @brief Order the jets in a collection to descend in \f$p_{T}\f$*/
     EL::StatusCode reorderJets(std::vector< xAOD::Jet*>* signalJets);
    /** @brief For jets below subleading pt threshold, call (optionally) applyVjetCalibration and applyJetUncertaintyTool */ 
    EL::StatusCode calibrateBelowThreshold(xAOD::Jet* jet, int iSysVar );

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
