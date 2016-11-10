.. _PreviousVersions:

Previous Versions
=================

The following stable configurations were used for previous results.
The documentation for 2015 results was located at https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MultijetBalanceRunIIFramework.

MJB Calibration 2015 (20.1) with EM-scale jets
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The calibration was performed with the following releases:

 * Analysis Base 2.3.44
 * MJB tag 00-00-03
 * xAODAnaHelpers tag 00-03-32

  
The MJB tag may also be found on svn at atlasperf/CombPerf/JetETMiss/Run2/Jet/Calibration/JetCalibrationTools/InSitu/MultijetBalance/MultijetBalanceAlgo/tags/MultijetBalanceAlgo-00-00-03.

The xAODAnaHelpers tag may also be found on svn at atlasinst/Institutes/UChicago/xAODAnaHelpers/tags/xAODAnaHelpers-00-03-32.

The V+jet calibration files may be found within  MultijetBalance/data/Vjet_2015_201/

Data samples that were used include (EXOT2):

 * scripts/sampleLists/2015/Data13TeV_Main_EXOT2_201_gridSamples.txt
 * scripts/sampleLists/2015/QCDPythia8_EXOT2_201_gridSamples.txt  (Nominal MC sample)
 * scripts/sampleLists/2015/QCDHerwig_EXOT2_201_gridSamples.txt  (Systematic MC sample)

MJB Calibration 2015 (20.7) with EM-scale jets
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The calibration was performed with the following releases:

 * Analysis Base 2.4.7
 * MJB commit 6cbefd5c
 * xAODAnaHelpers commit 2d502e1ecc
 * atlasoff/Reconstruction/Jet/JetCalibTools/tags/JetCalibTools-00-04-64
 * atlasoff/Reconstruction/Jet/JetUncertainties/tags/JetUncertainties-00-09-40

New config files are required for the JetUncertainties package, and should be placed within share/JES/2015/Moriond2016/
and linked in the MJB configuration file:

 * data/JetUncertaintiesConfigs/JESNPsForMJB_2015_207_EM.config
 * data/JetUncertaintiesConfigs/JESUncertaintyForMJB_2015_207_EM.root


The V+jet calibration files may be found within  MultijetBalance/data/Vjet_2015_207/

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

New config files are required for the JetUncertainties package, and should be placed within share/JES/2015/Moriond2016/
and linked in the MJB configuration file:

 * data/JetUncertaintiesConfigs/JESNPsForMJB_2015_207_LC.config
 * data/JetUncertaintiesConfigs/JESUncertaintyForMJB_2015_207_LC.root


The V+jet calibration files may be found within  MultijetBalance/data/Vjet_2015_207_LC/

The GRL used was data15_13TeV.periodAllYear_DetStatus-v79-repro20-02_DQDefects-00-02-02_PHYS_StandardGRL_All_Good_25ns.xml 


Data samples that were used include (EXOT2):

 * scripts/sampleLists/2015/Data13TeV_Main_EXOT2_207_gridSamples.txt
 * scripts/sampleLists/2015/QCDPythia8_EXOT2_207_gridSamples.txt  (Nominal MC sample)
 * scripts/sampleLists/2015/QCDHerwig_EXOT2_207_gridSamples.txt  (Systematic MC sample)
