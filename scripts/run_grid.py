#!/usr/bin/python

import os, math, sys
from time import strftime

test = False # does not run the jobs
validation = True
if not test:
  if not os.path.exists("gridOutput"):
    os.system("mkdir gridOutput")
  if not os.path.exists("gridOutput/gridJobs"):
    os.system("mkdir gridOutput/gridJobs")

timestamp = strftime("_%Y%m%d")
files = []
outputTags = []
files.append("../DijetFW/DijetResonanceAlgo/scripts/sampleLists/QCDPythia8_EXOT2_gridSamples.txt")
outputTags.append("QCDPy")

files.append("../DijetFW/DijetResonanceAlgo/scripts/sampleLists/QCDSherpa_gridSamples.txt")
outputTags.append("QCDS")

#files.append("../DijetFW/DijetResonanceAlgo/scripts/sampleLists/Data13TeV_Main_EXOT2_gridSamples.txt")
files.append("../DijetFW/DijetResonanceAlgo/scripts/sampleLists/mjb_gridSamples.txt")
outputTags.append("D")

files.append("../DijetFW/DijetResonanceAlgo/scripts/sampleLists/mjb2_gridSamples.txt")
outputTags.append("M")

if validation:
  for iFile, file_in in enumerate(files):

    ## Asym validation ##
    config_name = "$ROOTCOREBIN/data/MultijetBalanceAlgo/MultijetAlgo_AsymValidation.config"
    output_tag = "VA_"+outputTags[iFile]
    output_tag += timestamp
    submit_dir = "gridOutput/gridJobs/submitDir_"+os.path.basename(file_in).rsplit('.',1)[0]+"."+output_tag # define submit dir name
    command = "runMultijetBalance --file "+file_in+" --tag "+output_tag+" --submitDir "+submit_dir+" --config "+config_name
    command += ' --production phys-exotics'
    print command
    if not test: os.system(command)

    ## Full validation ##
    config_name = "$ROOTCOREBIN/data/MultijetBalanceAlgo/MultijetAlgo_FullValidation.config"
    output_tag = "VF_"+outputTags[iFile]
    output_tag += timestamp
    submit_dir = "gridOutput/gridJobs/submitDir_"+os.path.basename(file_in).rsplit('.',1)[0]+"."+output_tag # define submit dir name
    command = "runMultijetBalance --file "+file_in+" --tag "+output_tag+" --submitDir "+submit_dir+" --config "+config_name
    command += ' --production phys-exotics'
    print command
    if not test: os.system(command)

else:
  config_name = "$ROOTCOREBIN/data/MultijetBalanceAlgo/MultijetAlgo.config"

  for iFile, file_in in enumerate(files):
    output_tag = outputTags[iFile]

    output_tag += timestamp
    submit_dir = "gridOutput/gridJobs/submitDir_"+os.path.basename(file_in).rsplit('.',1)[0]+"."+output_tag # define submit dir name
    command = "runMultijetBalance --file "+file_in+" --tag "+output_tag+" --submitDir "+submit_dir+" --config "+config_name
    command += ' --production phys-exotics'
    print command
    if not test: os.system(command)

