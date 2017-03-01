.. _PreviousVersions:

Previous Versions
=================

The following stable configurations were used for previous results.
The documentation for 2015 results was located at https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MultijetBalanceRunIIFramework.

MJB Calibration 2016 (20.7) with EM and LC-scale jets
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The calibration was performed with the following releases:

 * Analysis Base 2.4.22
 * xAODAnaHelpers commit 3d2a40eca4f2d110b8c32c3
 * atlasoff/Reconstruction/Jet/JetUncertainties/tags/JetUncertainties-00-09-50

Config files can be found within either data/PreviousConfigs/2016_EM/ or data/PreviousConfigs/2016_LC/.
The event selection and input config files are fully defined within config_MJB_2016_EM207.py and config_MJB_2016_LC207.py

New config files are required for the JetUncertainties package, and should be placed within share/JES/2015/Moriond2016/
and linked in the MJB configuration file:

 * JESNuisanceParametersForMJB.config
 * JESUncertainty_forMJB.root


The V+jet calibration file is named Vjet_Nominal.root.

Data samples that were used include (JETM1):

 * scripts/sampleLists/2016/Data2016_Main_JETM1_gridSamples.txt
 * scripts/sampleLists/2015/QCDPythia8_JETM1_207_gridSamples.txt  (Nominal MC sample)
 * scripts/sampleLists/2015/QCDSherpa_JTEM1_207_gridSamples.txt  (Systematic MC sample)

MJB Calibration 2015 (20.7) with EM-scale jets
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The calibration was performed with the following releases:

 * Analysis Base 2.4.7
 * MJB commit 6cbefd5c
 * xAODAnaHelpers commit 2d502e1ecc
 * atlasoff/Reconstruction/Jet/JetCalibTools/tags/JetCalibTools-00-04-64
 * atlasoff/Reconstruction/Jet/JetUncertainties/tags/JetUncertainties-00-09-40

All necessary config files can be found in data/PreviousConfigs/2015_207_EM, and the event selection is defined by config_MJB_2015_EM207.py.
New config files are required for the JetUncertainties package, and should be placed within share/JES/2015/Moriond2016/
and linked in the MJB configuration file:

 * data/PreviousConfigs/2015_207_EM/JESNPsForMJB_2015_207_EM.config
 * data/PreviousConfigs/2015_207_EM/JESUncertaintyForMJB_2015_207_EM.root


The V+jet calibration files may be found within  data/PreviousConfigs/2015_207_EM/Vjet_2015_207/

The GRL used was data15_13TeV.periodAllYear_DetStatus-v75-repro20-01_DQDefects-00-02-02_PHYS_StandardGRL_All_Good_25ns.xml

Data samples that were used include (EXOT2):

 * scripts/sampleLists/2015/Data13TeV_Main_EXOT2_207_gridSamples.txt
 * scripts/sampleLists/2015/QCDPythia8_EXOT2_207_gridSamples.txt  (Nominal MC sample)
 * scripts/sampleLists/2015/QCDHerwig_EXOT2_207_gridSamples.txt  (Systematic MC sample)


MJB Calibration 2015 (20.7) with LC-scale jets
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The calibration was performed with the following releases:

 * Analysis Base 2.4.17
 * xAODAnaHelpers commit fe266d40623
 * atlasoff/Reconstruction/Jet/JetCalibTools/tags/JetCalibTools-00-04-66
 * atlasoff/Reconstruction/Jet/JetUncertainties/tags/JetUncertainties-00-09-47

All necessary config files can be found in data/PreviousConfigs/2015_207_LC, and the event selection is defined by config_MJB_2015_LC207.py.
New config files are required for the JetUncertainties package, and should be placed within share/JES/2015/Moriond2016/
and linked in the MJB configuration file:

 * data/PreviousConfigs/JESNPsForMJB_2015_207_LC.config
 * data/PreviousConfigs/JESUncertaintyForMJB_2015_207_LC.root


The V+jet calibration files may be found within data/PreviousConfigs/2015_207_LC/Vjet_2015_207_LC_Config4/.
Four iterations were run to account for downward response trends in the gamma+jet analysis at high pt, 
and the fourth configuration includes the rebinning that was used in the final result.

The GRL used was data15_13TeV.periodAllYear_DetStatus-v79-repro20-02_DQDefects-00-02-02_PHYS_StandardGRL_All_Good_25ns.xml 


Data samples that were used include (EXOT2):

 * scripts/sampleLists/2015/Data13TeV_Main_EXOT2_207_gridSamples.txt
 * scripts/sampleLists/2015/QCDPythia8_EXOT2_207_gridSamples.txt  (Nominal MC sample)
 * scripts/sampleLists/2015/QCDHerwig_EXOT2_207_gridSamples.txt  (Systematic MC sample)

MJB Calibration 2015 (20.1) with EM-scale jets
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The calibration was performed with the following releases:

 * Analysis Base 2.3.44
 * MJB tag 00-00-03
 * xAODAnaHelpers tag 00-03-32

  
The MJB tag may also be found on svn at atlasperf/CombPerf/JetETMiss/Run2/Jet/Calibration/JetCalibrationTools/InSitu/MultijetBalance/MultijetBalanceAlgo/tags/MultijetBalanceAlgo-00-00-03.

The xAODAnaHelpers tag may also be found on svn at atlasinst/Institutes/UChicago/xAODAnaHelpers/tags/xAODAnaHelpers-00-03-32.

The V+jet calibration files may be found within data/PreviousConfigs/2015_201_EM/Vjet_2015_201/

Data samples that were used include (EXOT2):

 * scripts/sampleLists/2015/Data13TeV_Main_EXOT2_201_gridSamples.txt
 * scripts/sampleLists/2015/QCDPythia8_EXOT2_201_gridSamples.txt  (Nominal MC sample)
 * scripts/sampleLists/2015/QCDHerwig_EXOT2_201_gridSamples.txt  (Systematic MC sample)

