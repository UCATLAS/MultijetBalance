import ROOT
from xAH_config import xAH_config

c = xAH_config()

c.setalg("BasicEventSelection",    { "m_name"   : "tmp",
"m_applyGRLCut"                 : True,
"m_msgLevel"      : "info",
                                     "m_GRLxml"                      : "$ROOTCOREBIN/data/MultijetBalance/data16_13TeV.periodAllYear_DetStatus-v88-pro20-21_DQDefects-00-02-04_PHYS_StandardGRL_All_Good_25ns.xml",
                                     "m_derivationName"              : "JETM1",
                                     "m_useMetaData"                 : False,
                                     "m_storePassHLT"                : True,
                                     "m_storeTrigDecisions"          : True,
                                     "m_applyTriggerCut"             : True,
                                     "m_triggerSelection"            : "HLT_j380|HLT_j260|HLT_j175|HLT_j110",
                                     #"m_triggerSelection"            : "HLT_j380|HLT_j260|HLT_j175|HLT_j110|HLT_j85",
                                     "m_checkDuplicatesData"         : False,
                                     "m_applyEventCleaningCut"       : True,
                                     "m_applyPrimaryVertexCut"       : True,
                                     "m_doPUreweighting"       : False,
                                     "m_PRWFileNames"          : "$ROOTCOREBIN/data/MultijetBalance/PRW_QCD.root",
                                     "m_lumiCalcFileNames"     : "$ROOTCOREBIN/data/MultijetBalance/ilumicalc_histograms_None_297730-311481_OflLumi-13TeV-005.root",
                                     } )
#prescales in run 307861
#15 - 301, 25 - 124, 35 - 18.3, 45 - 88,  55 - 40.7, 60 - 20, 85 - 14, 110 - 5.7, 150 - 11.8, 175 - 4.1, 260 - 1.2, 

### MJB Configuration  ###
c.setalg("MultijetBalanceAlgo",   { "m_name"                : "MultijetAlgo",

#---------------------------------------------------------------------------------------------
#### Jet collection and associated observables ####
  "m_inContainerName"     : "AntiKt4EMTopoJets",
  "m_jetDef"              : "AntiKt4EMTopo",
#  "m_inContainerName"     : "AntiKt4LCTopoJets",
#  "m_jetDef"              : "AntiKt4LCTopo",

  "m_jetCalibSequence"  : "JetArea_Residual_Origin_EtaJES_GSC",
# ICHEP 2016 20.7 calibration for validation
  "m_jetCalibConfig"    : "JES_2016data_Oct2016_EtaIntercalOnly.config",
  #"m_jetCalibConfig"    : "JES_MC15cRecommendation_May2016.config",

#  "m_inContainerName"     : "AntiKt4EMPFlowJets",
#  "m_jetDef"              : "AntiKt4EMPFlow",
#  "m_jetCalibSequence"  : "JetArea_Residual_EtaJES_GSC",
#  "m_jetCalibConfig"    : "JES_MC15cRecommendation_PFlow_Aug2016.config",

#---------------------------------------------------------------------------------------------
#### MJB iteration ####

#  "m_triggerAndPt" : "", #Leave this empty for efficiency studies!
  "m_triggerAndPt" : "HLT_j380:550,HLT_j260:400,HLT_j175:300,HLT_j110:200",
#  min binning 2016 from study on tight beta cut: "HLT_j380:466,HLT_j260:320, HLT_j175:180",
#  min binning 2016 from study on loose beta cut: ,
  #"m_triggerAndPt" : "HLT_j380:500,HLT_j260:350,HLT_j175:250,HLT_j110:200",  #Used for validation
#  "m_triggerAndPt" : "HLT_j360:420,HLT_j260:325,HLT_j200:250,HLT_j175:225,HLT_j150:200,HLT_j110:175,HLT_j85:125", #original

  "m_MJBIteration" : 0,
  ## The pt thresholds on the subleading jets for event inclusion. -1 gets taken from the V+jet file.
  ## This is set automatically in m_validation to a large number!
  "m_MJBIterationThreshold" : "9999999999",
  #"m_MJBIterationThreshold" : "-1,2000",
  ## For higher iterations:
  "m_MJBCorrectionFile" : "",
  #"m_MJBCorrectionFile" : "Bootstrap_Iteration0_LC_hist.combined.Pythia.DoubleMJB_initial.root",
  #"m_MJBCorrectionFile" : "Iteration0_2016LC_hist.combined.Pythia.Fit_DoubleMJB_initial.root",

## 2016 initial validation binning
  "m_binning"   : "200,250,300,350,400,450,500,550,600,650,700,750,800,850,900,950,1000,1050,1100,1150,1200,1300,1500,1700,2000,2300,2600", #23bins
#  "m_binning"   : "300,325,350,375,400,425,450,475,500,525,550,575,600,625,650,675,700,725,750,775,800,825,850,875,900,925,950,975,1000,1050,1100,1150,1200,1300,1500,1700,2000,2300", #37 bins
#  "m_binning"  : "300,350,400,450,500,550,600,650,700,750,800,850,900,950,1000,1100,1200,1300,1400,1600,1800,2000,2500,3000,3500",


 ## Use dedicated V+jet calibrations, requires JetCalibTools to only run on eta-intercalibration! ##
# "m_VjetCalibFile"  : "$ROOTCOREBIN/data/MultijetBalance/Configs/Vjet_Nominal.root",
 "m_VjetCalibFile"  : "",

 ## Use GSC value, not insitu, for leading jet.
#"m_leadingGSC" : True,

  ## Systematic Variations to use:
  "m_sysVariations" : "Nominal",
#  "m_sysVariations" : "All",
#  "m_sysVariations" : "Nominal-localMJB",

#------ Event Selections ------#
#  "m_numJets"  : 3,
#  "m_ptAsym" : 0.8,
#  "m_alpha" : 0.3,
  "m_beta" : 1.0,
#  "m_ptThresh" : 60,  #in GeV
  ## Looser beta cut to improve statistics
  "m_looseBetaCut" : True,

#------ Bootstrap Mode ------#
  "m_bootstrap" : False,
  "m_systTool_nToys" : 50,  #100

#------ Validation Mode ------#
  #You should probably turn off the m_VjetCalibFile for this!!
  ## Apply the jet calibrations to the leading jet:
#  "m_validation" : True,
#  ## Dijet validation mode: ##
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
  "m_jetDetailStr" : "kinematic", 
#  "m_jetDetailStr" : "kinematic truth truth_details flavorTag sfFTagVL sfFTagL sfFTagM sfFTagT",
  "m_trigDetailStr" : "basic passTriggers",

### Extra Options ###
#!  "m_debug"      :  False,
  ## Remove problematic Pileup events from low pt MC slices:
  "m_MCPileupCheckContainer" : "AntiKt4TruthJets",

  #!! "m_isAFII" : True,
  #!!  "m_isDAOD" : True,
#!!  "m_useCutFlow" : False,

  "m_XSFile"  : "$ROOTCOREBIN/data/MultijetBalance/XsAcc_13TeV.txt", 

#---------------------------------------------------------------------------------------------
#### Tool Configurations ####

  #-- JetCleaning --#
  "m_jetCleanCutLevel"  : "LooseBad",
#  "m_jetCleanUgly"      : True,

  #-- JVT --#
  "m_JVTWP" : "Medium", # 2016

  #-- JetUncertaintiesTool --#
  "m_jetUncertaintyConfig" : "data/JES_2016/Moriond2017/JES2016_AllNuisanceParameters.config",
#!!  "m_jetUncertaintyConfig" : "$ROOTCOREBIN/data/JetUncertainties/JES_2016/Moriond2017/JESNuisanceParametersForMJB.config",

  #-- JetResolutionTool --#
  "m_JERUncertaintyConfig" : "JetResolution/Prerec2015_xCalib_2012JER_ReducedTo9NP_Plots_v2.root", 
  "m_JERApplySmearing"     : False,
  #"m_JERSystematicMode"    : "Full",
  "m_JERSystematicMode"    : "Simple",
  
  #-- TileCorrectionTool --#
  "m_TileCorrection"      : False



  } )

