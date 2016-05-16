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
config_name = "$ROOTCOREBIN/data/MultijetBalance/config_MJB.py"
extraTag = "MJB" # Extra output tag for all files


timestamp = strftime("_%Y%m%d")
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
#production_name = ""
production_name = "phys-exotics"



files = []
outputTags = []

## File lists and specific output Tags

#files.append("MultijetBalance/scripts/sampleLists/QCDPythia8_EXOT2_gridSamples.txt")
#outputTags.append("QCDPy")
#files.append("MultijetBalance/scripts/sampleLists/QCDHerwig_EXOT2_gridSamples.txt")
#outputTags.append("QCDH")
#files.append("MultijetBalance/scripts/sampleLists/Data13TeV_Main_EXOT2_gridSamples.txt")
#outputTags.append("Ex")
files.append("MultijetBalance/scripts/sampleLists/Data13TeV_Main_gridSamples.txt")
outputTags.append("Mn")
#files.append("MultijetBalance/scripts/sampleLists/Data13TeV_Debug_gridSamples.txt")
#outputTags.append("Db")

for iFile, file_in in enumerate(files):

  output_tag = outputTags[iFile]
  output_tag += extraTag
  output_tag += timestamp
  submit_dir = "gridOutput/gridJobs/submitDir_"+os.path.basename(file_in).rsplit('.',1)[0]+"."+output_tag # define submit dir name

  ## Configure submission driver ##
  driverCommand = ''
  if runType == 'grid':
    driverCommand = 'prun --optGridOutputSampleName='
    if len(production_name) > 0:
      driverCommand = ' prun --optSubmitFlags="--official" --optGridOutputSampleName='
      driverCommand += 'group.'+production_name
    else:
      driverCommand += 'user.%nickname%'
    driverCommand += '.%in:name[1]%.%in:name[2]%.%in:name[3]%.'+output_tag
  elif runType == 'condor':
    driverCommand = ' condor --optFilesPerWorker 10 --optBatchWait'
  elif runType == 'local':
    driverCommand = ' direct'


  command = './xAODAnaHelpers/scripts/xAH_run.py'
  if runType == 'grid':
    command += ' --inputDQ2'
  command += ' --inputList --files '+file_in
  command += ' --config '+config_name
  command += ' --force --submitDir '+submit_dir
  command += ' '+driverCommand


  print command
  if not test: os.system(command)

#  if not "Sherpa" in file_in:
#    command += ' --oneJobPerFile'
