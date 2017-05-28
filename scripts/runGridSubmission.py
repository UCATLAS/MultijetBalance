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
config_name = "/home/jdandoy/Documents/Dijet/Rel21MJB/MultijetBalance/data/config_MJB.py"
#config_name = "/home/jdandoy/Documents/Dijet/Rel21MJB/MultijetBalance/data/config_Gjet.py"
#config_name = "/home/jdandoy/Documents/Dijet/Rel21MJB/MultijetBalance/data/config_Zjet.py"
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

samples = { 

  "MJBRel21"      : "sampleLists/data16_r21_JETM1_p3142.txt",
#  "MJBRel207"     : "sampleLists/data16_r207_JETM1_p2950.txt",
#  "MJBRel21"      : "data16_13TeV.*Main.deriv*JETM1.*p3142",
#  "MJBRel207"     : "data16_13TeV.*Main.merge*JETM1.*p2950",

    
#  "QCDPythia21" : "mc16_13TeV.3610*.Pythia8*JZ*W.*JETM1*_r9315_p3141",
#
#  "MJBData16"     : "data16_13TeV.*Main.merge*JETM1.*p2950",
#  "MJBData15"     : "data15_13TeV.*Main.merge*JETM1.*p2950",
#  "QCDPythia"     : "mc15_13TeV.3610*.Pythia8*JZ*W.merge.*JETM1.*_r77*r7676_p2666",
#  "QCDSherpa"     : "mc15_13TeV.4261*.Sherpa*JZ*.merge.*JETM1.*r7725_r7676_p2666",
#
#  'ZJBData16'     : 'data16_13TeV.*Main.merge*JETM3.*p2950',
#  'ZJBData15'     : 'data15_13TeV.*Main.merge*JETM3.*p2950',
#  'ELPythia'      : 'mc15_13TeV.361106*PowhegPythia*Zee*JETM3*p2996'], 
#  'ELSherpa'      : 'mc15_13TeV.363*Sherpa*NNPDF30NNLO*Zee_Pt*JETM3*p2879'],
#  'ELMadgraph'    : 'mc15_13TeV.36150*MadGraphPythia8*Zee*JETM3*p2879'],
#  'MUPythia'      : 'mc15_13TeV.361107*PowhegPythia*Zmumu*JETM3*p2996'],
#  'MUSherpa'      : 'mc15_13TeV.363*Sherpa*NNPDF30NNLO*Zmumu_Pt*JETM3*p2879'], 
#  'MUMadgraph'    : 'mc15_13TeV.36150*MadGraphPythia8*Zmumu*JETM3*p2879'],
#
#  'GJBData16'     : 'data16_13TeV.*Main.merge*JETM4.*p2950'],
#  'GJBData15'     : 'data15_13TeV.*Main.merge*JETM4.*p2950'],
#  'PHPythia'      : 'mc15_13TeV.423*Pythia8*gammajet*JETM4*p2839'], 
#  'PHSherpa'      : 'mc15_13TeV.3610*Sherpa*Photon*JETM4*p3017'],
#  'PHSherpaQ'     : 'mc15_13TeV.3432*Sherpa*SinglePhotonPt*QCUT5*JETM4*p3017']

}

files = []
outputTags = []

#files.append("MultijetBalance/scripts/sampleLists/data16_r207_JETM1_p2950.txt")
#outputTags.append("Rel207")

#for iFile, file_in in enumerate(files):

for sampleName, sample in samples.iteritems():

  output_tag = sampleName + extraTag + timestamp
  submit_dir = "gridOutput/gridJobs/submitDir_"+output_tag 

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

  if sample.startswith('sampleLists'):
    command += ' --inputList'
  command += ' --files '+sample
  command += ' --config '+config_name
  command += ' --force --submitDir '+submit_dir
  command += ' '+driverCommand


  print command
  if not test: os.system(command)

#  if not "Sherpa" in file_in:
#    command += ' --oneJobPerFile'
