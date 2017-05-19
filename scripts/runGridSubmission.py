#!/usr/bin/python

#####################################
# A script for MJB grid submission
# Submit grid jobs using text files with lists of containers
# For questions contact Jeff.Dandoy@cern.ch
#####################################


import os, math, sys
from time import strftime

### User Options ###
test = False # does not run the jobs
#config_name = "$ROOTCOREBIN/data/MultijetBalance/config_MJB_2016_EM207.py"
#config_name = "$ROOTCOREBIN/data/MultijetBalance/config_MJB_2015_Validation.py"
#config_name = "$ROOTCOREBIN/data/MultijetBalance/config_MJB.py"
config_name = "/home/jdandoy/Documents/Dijet/Rel21MJB/MultijetBalance/data/config_MJB.py"
extraTag = "" # Extra output tag for all files


timestamp = strftime("_%d%m%y")
if not test:
  if not os.path.exists("gridOutput"):
    os.system("mkdir gridOutput")
  if not os.path.exists("gridOutput/gridJobs"):
    os.system("mkdir gridOutput/gridJobs")


#### Driver options ####
runType = 'grid'   #CERN grid
#runType = 'local'
#runType = 'condor' #Uchicago Condor

## Set this only for group production submissions ##
production_name = ""
#production_name = "phys-exotics"



files = []
outputTags = []

#files.append("MultijetBalance/scripts/sampleLists/data16_r207_JETM1_p2950.txt")
#outputTags.append("Rel207")
files.append("MultijetBalance/scripts/sampleLists/data16_r21_JETM1_p3142.txt")
outputTags.append("Rel21")

## File lists and specific output Tags

##2016##
#files.append("MultijetBalance/scripts/sampleLists/2015/QCDPythia8_JETM1_207_gridSamples.txt")
#outputTags.append("QCDPythia8_JETM1_BS0")
#files.append("MultijetBalance/scripts/sampleLists/2015/QCDSherpa_JETM1_207_gridSamples.txt")
#outputTags.append("QCDSherpa_JETM1_BS0")
#files.append("MultijetBalance/scripts/sampleLists/2016/Data2016_Main_JETM1_gridSamples.txt")
#outputTags.append("Main2016_JETM1_BS0")
#files.append("MultijetBalance/scripts/sampleLists/Data2016_Debug_JETM1_gridSamples.txt")
#outputTags.append("Debug2016_JETM1_It0")

#files.append("MultijetBalance/scripts/sampleLists/2015/Data13TeV_Debug_EXOT2_207_gridSamples.txt")
#outputTags.append("Db_R1")
#files.append("MultijetBalance/scripts/sampleLists/2015/Data13TeV_Main_EXOT2_207_gridSamples.txt")
#outputTags.append("Ex_B1")
#files.append("MultijetBalance/scripts/sampleLists/2015/QCDPythia8_JETM1_207_gridSamples.txt")
#outputTags.append("QCDPJ_R0")
#files.append("MultijetBalance/scripts/sampleLists/2015/QCDSherpa_JETM1_207_gridSamples.txt")
#outputTags.append("QCDSJ_R0")
#files.append("MultijetBalance/scripts/sampleLists/2015/QCDHerwig_JETM1_207_gridSamples.txt")
#outputTags.append("QCDHJ_R0")

for iFile, file_in in enumerate(files):

  output_tag = outputTags[iFile]
  output_tag += extraTag
  output_tag += timestamp
  submit_dir = "gridOutput/gridJobs/submitDir_"+os.path.basename(file_in).rsplit('.',1)[0]+"."+output_tag # define submit dir name

  ## Configure submission driver ##
  driverCommand = ''
  if runType == 'grid':
    driverCommand = 'prun --optSubmitFlags="--skipScout" --optGridOutputSampleName='
    #driverCommand = 'prun --optSubmitFlags="--skipScout --excludedSite=ANALY_CERN_SHORT,ANALY_BNL_SHORT" --optGridOutputSampleName='
    if len(production_name) > 0:
      #driverCommand = ' prun --optSubmitFlags="--memory=5120 --official --skipScout" --optGridOutputSampleName='
      driverCommand = ' prun --optSubmitFlags="--official --skipScout" --optGridOutputSampleName='
      #driverCommand = ' prun --optSubmitFlags="--official" --optGridOutputSampleName='
      driverCommand += 'group.'+production_name
    else:
      driverCommand += 'user.%nickname%'
    driverCommand += '.%in:name[1]%.%in:name[2]%.'+output_tag
    #driverCommand += '.%in:name[1]%.%in:name[2]%.%in:name[3]%.'+output_tag
  elif runType == 'condor':
    driverCommand = ' condor --optFilesPerWorker 10 --optBatchWait'
  elif runType == 'local':
    driverCommand = ' direct'


  command = './xAODAnaHelpers/scripts/xAH_run.py'
  if runType == 'grid':
    command += ' --inputRucio '
  command += ' --inputList --files '+file_in
  command += ' --config '+config_name
  command += ' --force --submitDir '+submit_dir
  command += ' '+driverCommand


  print command
  if not test: os.system(command)

#  if not "Sherpa" in file_in:
#    command += ' --oneJobPerFile'
