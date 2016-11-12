import ROOT
from xAH_config import xAH_config

c = xAH_config()

c.setalg("BasicEventSelection",    { "m_applyGRLCut"                 : True,
                                     "m_GRLxml"                      : "$ROOTCOREBIN/data/MultijetBalance/data16_13TeV.periodAllYear_DetStatus-v83-pro20-14_DQDefects-00-02-04_PHYS_StandardGRL_All_Good_25ns.xml",
                                     "m_derivationName"              : "EXOT2",
                                     "m_useMetaData"                 : False,
                                     "m_storePassHLT"                : True,
                                     "m_storeTrigDecisions"          : True,
                                     "m_applyTriggerCut"             : True,
                                     "m_triggerSelection"            : "HLT_j380|HLT_j260|HLT_j175",
                                     #"m_triggerSelection"            : "HLT_j380|HLT_j260|HLT_j175|HLT_j110",
                                     "m_checkDuplicatesData"         : False,
                                     "m_applyEventCleaningCut"       : True,
                                     "m_applyPrimaryVertexCut"       : True
                                     } )

c.setalg("MultijetBalanceAlgo",   { "m_name"                : "MultijetAlgo",

### MJB Configuration  ###
  "m_inContainerName"     : "AntiKt4EMTopoJets",
  "m_triggerAndPt" : "HLT_j380:500,HLT_j260:350,HLT_j175:300",
#  min binning 2016 from study on tight beta cut: "HLT_j380:466,HLT_j260:320, HLT_j175:180",
#  min binning 2016 from study on loose beta cut: ,
  #"m_triggerAndPt" : "HLT_j380:500,HLT_j260:350,HLT_j175:250,HLT_j110:200",  #Used for validation
#  "m_triggerAndPt" : "HLT_j360:420,HLT_j260:325,HLT_j200:250,HLT_j175:225,HLT_j150:200,HLT_j110:175,HLT_j85:125", #original
  "m_MJBIteration" : 0,
  ## The pt thresholds on the subleading jets for event inclusion. -1 gets taken from the V+jet file.
  ## This is set automatically in m_validation to a large number!
  "m_MJBIterationThreshold" : "900,1700",
  #"m_MJBIterationThreshold" : "-1,1700",
  ## For higher iterations:
#  "m_MJBCorrectionFile" : "Iteration0_EM_hist.combined.Pythia.Fit_DoubleMJB_initial.root",

## 2016 initial validation binning
  "m_binning"   : "300,350,400,450,500,550,600,650,700,750,800,850,900,950,1000,1050,1100,1150,1200,1300,1500,1700,2000,2300", #23bins
#  "m_binning"   : "300,325,350,375,400,425,450,475,500,525,550,575,600,625,650,675,700,725,750,775,800,825,850,875,900,925,950,975,1000,1050,1100,1150,1200,1300,1500,1700,2000,2300", #37 bins
#  "m_binning"  : "300,350,400,450,500,550,600,650,700,750,800,850,900,950,1000,1100,1200,1300,1400,1600,1800,2000,2500,3000,3500",


 ## Use dedicated V+jet calibrations, requires JetCalibTools to only run on eta-intercalibration! ##
# "m_VjetCalibFile"  : "$ROOTCOREBIN/data/MultijetBalance/PreviousConfigs/2015_207_EM/Vjet_2015_207/Vjet_Nominal.root",
 "m_VjetCalibFile"  : "",

 ## Use GSC value, not insitu, for leading jet.
#"m_leadingGSC" : True,

  ## Systematic Variations to use:
#  "m_sysVariations" : "Nominal",
  "m_sysVariations" : "AllSystematics",
#  "m_sysVariations" : "Nominal-MJB",

  ## (Deprecated Option) Add statistical systematic for MJB:
#  "m_MJBStatsOn" : True,

#------ Event Selections ------#
  "m_numJets"  : 3,
  "m_ptAsym" : 0.8,
#  "m_alpha" : 0.3,
#  "m_beta" : 1.0,
#  "m_ptThresh" : 25,  #in GeV
  ## Looser beta cut to improve statistics
#  "m_looseBetaCut" : True,

#------ Bootstrap Mode ------#
#  "m_bootstrap" : True,
#  "m_systTool_nToys" : 100,

#------ Validation Mode ------#
  #You should probably turn off the m_VjetCalibFile for this!!
  ## Apply the jet calibrations to the leading jet:
#  "m_validation" : True,
  ## Dijet validation mode: ##
#  "m_numJets"  : 2,
#  "m_ptAsym" : 1.0,

#------ B-tag Mode : Not yet implemented ------#
#  "m_bTagWPsString" : "77,85",
#  "m_bTagFileName" : "$ROOTCOREBIN/data/xAODAnaHelpers/2016-Winter-13TeV-MC15-CDI-February5_prov.root",
#  "m_bTagVar"  : "MV2c20",
#  "m_bTagLeadJetWP" : "",
#  "m_leadingInsitu" : True,
#  "m_noLimitJESPt" :  True,

#------ Closure Test Mode ------#
  ##Apply MJB correction to the leading jet:
#  "m_closureTest" : True,

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
  "m_debug"      :  False,
  ## Remove problematic Pileup events from low pt MC slices:
  "m_MCPileupCheckContainer" : "AntiKt4TruthJets",

  #!! "m_isAFII" : True,
  #!!  "m_isDAOD" : True,
#!!  "m_useCutFlow" : False,

  "m_XSFile"  : "$ROOTCOREBIN/data/MultijetBalance/XsAcc_13TeV.txt", 

### Tool Configurations ###

  #-- JetCalibTool --#
  "m_jetDef"            : "AntiKt4EMTopo",
  "m_jetCalibSequence"  : "JetArea_Residual_Origin_EtaJES_GSC",
## ICHEP 2016 20.7 calibration for validation
  "m_jetCalibConfig"    : "JES_MC15cRecommendation_May2016.config",

  #-- JetCleaning --#
  "m_jetCleanCutLevel"  : "LooseBad",
#  "m_jetCleanUgly"      : True,

  #-- JVT --#
  "m_JVTCut" : 0.59, # 2016

  #-- JetUncertaintiesTool --#
  #"m_jetUncertaintyConfig" : "$ROOTCOREBIN/data/JetUncertainties/JES_2015/Moriond2016/JES2015_AllNuisanceParameters.config"
  ### 2016 ICHEP offical Systematics
  "m_jetUncertaintyConfig" : "$ROOTCOREBIN/data/JetUncertainties/JES_2015/ICHEP2016/JES2015_AllNuisanceParameters.config"
  #"m_jetUncertaintyConfig" : "$ROOTCOREBIN/data/JetUncertainties/JES_2015/ICHEP2016/JES2015_SR_Scenario1.config"


  } )

