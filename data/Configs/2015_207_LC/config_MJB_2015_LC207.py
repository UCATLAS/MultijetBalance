import ROOT
from xAH_config import xAH_config

c = xAH_config()

c.setalg("BasicEventSelection",    { "m_applyGRLCut"                 : True,
                                     "m_GRLxml"                      : "$ROOTCOREBIN/data/MultijetBalance/data15_13TeV.periodAllYear_DetStatus-v79-repro20-02_DQDefects-00-02-02_PHYS_StandardGRL_All_Good_25ns.xml",
                                     "m_derivationName"              : "EXOT2",
                                     "m_useMetaData"                 : False,
                                     "m_storePassHLT"                : True,
                                     "m_applyTriggerCut"             : True,
                                     "m_triggerSelection"            : "HLT_j360|HLT_j260|HLT_j200",
                                     "m_checkDuplicatesData"         : False,
                                     "m_applyEventCleaningCut"       : True,
                                     "m_applyPrimaryVertexCut"       : True
#                                     "m_MCPileupCheckRecoContainer"  : "AntiKt4EMTopoJets",
#                                     "m_MCPileupCheckTruthContainer" : "AntiKt4TruthJets"
                                     } )

c.setalg("MultijetBalanceAlgo",   { "m_name"                : "MultijetAlgo",

### MJB Configuration  ###
  "m_inContainerName"     : "AntiKt4LCTopoJets",
  "m_triggerAndPt" : "HLT_j360:480,HLT_j260:360,HLT_j200:300",
  "m_MJBIteration" : 1,
  ## The pt thresholds on the subleading jets for event inclusion:
  "m_MJBIterationThreshold" : "935,1480",  #Nominal and Config2 for Vjet
  ## For higher iterations:
  "m_MJBCorrectionFile" : "Bootstrap_Iteration0_LC_hist.combined.Pythia.DoubleMJB_initial.root",
  #"m_MJBCorrectionFile" : "Iteration0_LC_hist.combined.Pythia.Fit_DoubleMJB_initial.root",

  "m_binning"  : "300,360,420,480,540,600,660,720,780,840,900,960,1020,1140,1260,1480,2000",
#!!  "m_MJBCorrectionBinning" : "Fine",


 ## Use dedicated V+jet calibrations, requires JetCalibTools to only run on eta-intercalibratino! ##
 "m_VjetCalibFile"  : "$ROOTCOREBIN/data/MultijetBalance/Vjet_2015_207_LC_Config2/Vjet_Systematics.root",

  ## Systematic Variations to use:
#  "m_sysVariations" : "Nominal",
  "m_sysVariations" : "AllSystematics",
#  "m_sysVariations" : "Nominal-MJB",

  ## (Deprecated Option) Add statistical systematic for MJB:
#  "m_MJBStatsOn" : True,

#------ Event Selections ------#
#  "m_numJets"  : 3,
#  "m_ptAsym" : 0.8,
#  "m_alpha" : 0.3,
#  "m_beta" : 1.0,
#  "m_ptThresh" : 25,  #in GeV
  ## Force removal of all jets within beta:
#  "m_allJetBeta" : True,

#------ Bootstrap Mode ------#
  "m_bootstrap" : True,
  "m_systTool_nToys" : 100,

#------ Validation Mode ------#
  ## Apply the jet calibrations to the leading jet:
#  "m_leadingInsitu" : True,
  # Allow calibrations of subleading jets beyond JetCalibTool limit:
#  "m_noLimitJESPt" : True,
#  "m_numJets"  : 2,
#  "m_ptAsym" : 1.0,

#------ B-tag Mode : Not yet implemented ------#
  "m_bTagWPsString" : "",
#  "m_bTagFileName" : "$ROOTCOREBIN/data/xAODAnaHelpers/2016-Winter-13TeV-MC15-CDI-February5_prov.root",
#  "m_bTagVar"  : "MV2c20",
#  "m_bTagLeadJetWP" : "",
#  "m_leadingInsitu" : True,
#  "m_noLimitJESPt" :  True,

#------ Not Used ------#
  ##Apply MJB correction to the leading jet:
#  "m_closureTest" : True,

  ## (Deprecated Option) Bin corrections against leading jet pt, not against reference jet pt:
#!!  "m_leadJetMJBCorrection" : True,

  ## Deprecated Option) Force subleading jet to be greater than
#!!  "m_reverseSubleading" : True,


### Plotting Options ###
#  "m_writeTree" : True,
  "m_writeNominalTree" : True,
#  "m_MJBDetailStr" : "bTag85 bTag77",
#  "m_MJBDetailStr" : "extraMJB",
  "m_eventDetailStr" : "pileup",
  "m_jetDetailStr" : "kinematic flavorTag",# truth",#truth_details",
#  "m_jetDetailStr" : "kinematic truth truth_details sfFTagVL sfFTagL sfFTagM sfFTagT",
  "m_trigDetailStr" : "basic passTriggers",

### Extra Options ###
#  "m_debug"      :  True,
#  "m_maxEvent" : 2000,
  ## Remove problematic Pileup events from low pt MC slices:
  "m_MCPileupCheckContainer" : "AntiKt4TruthJets",

  #!! "m_isAFII" : True,
  #!!  "m_isDAOD" : True,
#!!  "m_useCutFlow" : False,


### Tool Configurations ###

  #-- JetCalibTool --#
  "m_jetDef"            : "AntiKt4LCTopo",
  "m_jetCalibSequence"  : "JetArea_Residual_Origin_EtaJES_GSC",
## Final 20.1 calibration for validation
#  "m_jetCalibConfig"    : "JES_2015dataset_recommendation_Feb2016.config",
## Initial 20.7 eta-intercalibration for results
  "m_jetCalibConfig"    : "JES_20_7_Recommendation_May2016_EtaIntercalOnly.config",

  #-- JetCleaning --#
  "m_jetCleanCutLevel"  : "LooseBad",
#  "m_jetCleanUgly"      : True,

  #-- JVT --#
  "m_JVTCut" : 0.59, # 2016

  #-- JetUncertaintiesTool --#
  "m_jetUncertaintyConfig" : "$ROOTCOREBIN/data/JetUncertainties/JES_2015/ICHEP2016/JESNPsForMJB_2015_207_LC.config"


  } )

