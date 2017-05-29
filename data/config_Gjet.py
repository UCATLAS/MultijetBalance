import ROOT
from xAH_config import xAH_config

c = xAH_config()

c.setalg("BasicEventSelection",    { "m_name"   : "tmp",
"m_applyGRLCut"                 : True,
"m_msgLevel"      : "info",
                                     "m_GRLxml"                      : "MultijetBalance/data16_13TeV.periodAllYear_DetStatus-v88-pro20-21_DQDefects-00-02-04_PHYS_StandardGRL_All_Good_25ns.xml",
                                     "m_derivationName"              : "JETM4",
                                     "m_useMetaData"                 : False,
                                     "m_storePassHLT"                : True,
                                     "m_storeTrigDecisions"          : True,
                                     "m_applyTriggerCut"             : True,
                                     "m_triggerSelection"            : "HLT_g140_loose|HLT_g120_loose|HLT_g100_loose|HLT_g80_loose|HLT_g70_loose|HLT_g60_loose|HLT_g50_loose|HLT_g35_loose|HLT_g25_loose|HLT_g20_loose|HLT_g15_loose|HLT_g10_loose",
                                     "m_checkDuplicatesData"         : False,
                                     "m_applyEventCleaningCut"       : True,
                                     "m_applyPrimaryVertexCut"       : True,
                                     "m_doPUreweighting"       : True,
                                     "m_PRWFileNames"          : "MultijetBalance/mc15c_v2_defaults.NotRecommended.prw.root",
                                     #"m_PRWFileNames"          : "MultijetBalance/PRW_QCD.root",
                                     "m_lumiCalcFileNames"     : "MultijetBalance/ilumicalc_histograms_None_297730-311481_OflLumi-13TeV-005.root",
                                     } )

c.setalg("PhotonCalibrator", {
    "m_name"                : "photon_calib",
    "m_msgLevel"            : "info",
    "m_inContainerName"     : "Photons",
    "m_inputAlgoSystNames"  : "",
    "m_outContainerName"    : "Photons_Calib",
    "m_outputAlgoSystNames" : "Photons_Calib_Syst",
    "m_esModel"             : "es2016data_mc15c",
    "m_decorrelationModel"  : "1NP_v1",
    #"m_esModel"            : "es2016PRE",
    #"m_decorrelationModel"  : "1NPCOR_PLUS_UNCOR",
    "m_systName"            : "",
    "m_systVal"             : 0.0,
    } )

c.setalg("PhotonSelector", {
    "m_name"                      : "photon_selection",
    "m_msgLevel"                  : "info",
    "m_inContainerName"           : "Photons_Calib",
    "m_outContainerName"          : "Photons_Selected",
    "m_createSelectedContainer"   : True,
    "m_decorateSelectedObjects"   : True,
    "m_pT_min"                    : 25e3,
    "m_eta_max"                   : 2.37,
    "m_pass_min"                  : 1,
#Jamie    "m_eta_max"                   : 1.37,
    "m_vetoCrack"                 : True,
    "m_doAuthorCut"               : True,
    "m_doOQCut"                   : True,
    "m_photonIdCut"               : "Tight",
    "m_IsoWPList"                 : "FixedCutTight",
#    "m_IsoWPList"                 : "FixedCutTightCaloOnly,FixedCutTight,FixedCutLoose",
    } )

### MJB Configuration  ###
c.setalg("MultijetBalanceAlgo",   { "m_name"                : "GammaJetBalance",

#---------------------------------------------------------------------------------------------
#### Jet collection and associated observables ####
  "m_modeStr"   : "Gjet",
  "m_inContainerName_photons"     : "Photons_Selected",

  "m_inContainerName_jets"     : "AntiKt4EMTopoJets",
  "m_jetDef"              : "AntiKt4EMTopo",
  
  "m_jetCalibSequence"  : "JetArea_Residual_Origin_EtaJES_GSC",
  "m_jetCalibConfig"    : "JES_data2016_data2015_Recommendation_Dec2016.config",

#---------------------------------------------------------------------------------------------

#  "m_triggerAndPt" : "", #Leave this empty for efficiency studies!
  "m_triggerAndPt" : "HLT_g140_loose:145,HLT_g120_loose:125,HLT_g100_loose:105,HLT_g80_loose:85,HLT_g70_loose:75,HLT_g60_loose:65,HLT_g50_loose:55,HLT_g35_loose:40,HLT_g25_loose:30,HLT_g20_loose:25,HLT_g15_loose:20,HLT_g10_loose:15",


  "m_binning"   : "15, 20, 25, 30, 40, 55, 65, 75, 85, 105, 125, 145, 170, 200, 250, 300, 400, 500, 600, 800, 1000, 1200, 1400, 1600, 2000, 10000",
  #Jamie's binning
  #"m_binning"   : "25, 45, 65, 85, 105, 125, 160, 210, 260, 310, 400, 500, 600, 800, 1000, 1200, 1400, 1600, 2000, 10000",



  ## Systematic Variations to use:
  "m_sysVariations" : "Nominal",
#  "m_sysVariations" : "Nominal-JCS-JES-JER",
#  "m_sysVariations" : "All",

#------ Event Selections ------#
  "m_numJets"  : 1,
  "m_alpha"     : 0.3,
  "m_ptAsymVar" : 0.1,
  "m_ptAsymMin" : 20,
  "m_leadJetPtThresh" : 10,
  "m_overlapDR" : 0.2,

#------ Bootstrap Mode ------#
  "m_bootstrap" : False,
  "m_systTool_nToys" : 50,  #100

#------ Validation Mode ------#
  #You should probably turn off the m_VjetCalibFile for this!!
  ## Apply the jet calibrations to the leading jet:
#  "m_validation" : True,

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
  "m_writeTree" : True,
#  "m_writeNominalTree" : True,
#  "m_MJBDetailStr" : "bTag85 bTag77",
#  "m_MJBDetailStr" : "extraMJB",
  "m_eventDetailStr" : "pileup",
  "m_jetDetailStr" : "kinematic", 
#  "m_jetDetailStr" : "kinematic truth truth_details flavorTag sfFTagVL sfFTagL sfFTagM sfFTagT",
  "m_trigDetailStr" : "basic passTriggers",

### Extra Options ###
  "m_debug"      :  False,
  ## Remove problematic Pileup events from low pt MC slices:
  "m_MCPileupCheckContainer" : "AntiKt4TruthJets",

  #!! "m_isAFII" : True,
  #!!  "m_isDAOD" : True,
#!!  "m_useCutFlow" : False,

  "m_XSFile"  : "MultijetBalance/XsAcc_13TeV.txt", 

#---------------------------------------------------------------------------------------------
#### Tool Configurations ####

  #-- JetCleaning --#
  "m_jetCleanCutLevel"  : "LooseBad",
#  "m_jetCleanUgly"      : True,

  #-- JVT --#
  "m_JVTWP" : "Medium", # 2016
  "m_JVTVar" : "JvtJvfcorr",

  #-- JetUncertaintiesTool --#
  "m_jetUncertaintyConfig" : "JES2016_AllNuisanceParameters.config",
#  "m_jetUncertaintyConfig" : "data/JES_2016/Moriond2017/JES2016_AllNuisanceParameters.config",
#!!  "m_jetUncertaintyConfig" : "$ROOTCOREBIN/data/JetUncertainties/JES_2016/Moriond2017/JESNuisanceParametersForMJB.config",

  #-- JetResolutionTool --#
  "m_JERUncertaintyConfig" : "JetResolution/Prerec2015_xCalib_2012JER_ReducedTo9NP_Plots_v2.root", 
  "m_JERApplySmearing"     : False,
  #"m_JERSystematicMode"    : "Full",
  "m_JERSystematicMode"    : "Simple",
  
  #-- TileCorrectionTool --#
  "m_TileCorrection"      : False



  } )

