#!/usr/bin/python

import os, math, sys
from time import strftime

test = False # does not run the jobs
if not test:
  if not os.path.exists("gridOutput"):
    os.system("mkdir gridOutput")
  if not os.path.exists("gridOutput/gridJobs"):
    os.system("mkdir gridOutput/gridJobs")

## has all samples - pick your favorites
##file_in = "MultijetBalanceAlgo/scripts/sampleLists/Week1_EXOT2_gridSamples.txt"
##file_in = "MultijetBalanceAlgo/scripts/sampleLists/Initial13TeVData_gridSamples.txt"
##file_in = "MultijetBalanceAlgo/scripts/sampleLists/Data13TeV_Main_gridSamples.txt"


file_in = "../DijetFW/DijetResonanceAlgo/scripts/sampleLists/QCDPythia8_EXOT2_gridSamples.txt"
output_tag = "DiLT2"  # CHANGE BEFORE SUBMITTING - no periods!

config_name = "$ROOTCOREBIN/data/MultijetBalanceAlgo/MultijetAlgo.config"

timestamp = strftime("_%Y%m%d")
output_tag += timestamp

submit_dir = "gridOutput/gridJobs/submitDir_"+os.path.basename(file_in).rsplit('.',1)[0]+"."+output_tag # define submit dir name

command = "runMultijetBalance --file "+file_in+" --tag "+output_tag+" --submitDir "+submit_dir+" --config "+config_name
#command += " --inputTag data15_comm.*"
command += ' --production phys-exotics'
print command
if not test: os.system(command)



#
##file_in = "../DijetFW/DijetResonanceAlgo/scripts/sampleLists/Data13TeV_Main_EXOT2_gridSamples.txt"
#file_in = "../DijetFW/DijetResonanceAlgo/scripts/sampleLists/mjb_gridSamples.txt"
#output_tag = "MJB"  # CHANGE BEFORE SUBMITTING - no periods!
#
#config_name = "$ROOTCOREBIN/data/MultijetBalanceAlgo/MultijetAlgo.config"
#
#timestamp = strftime("_%Y%m%d")
#output_tag += timestamp
#
#submit_dir = "gridOutput/gridJobs/submitDir_"+os.path.basename(file_in).rsplit('.',1)[0]+"."+output_tag # define submit dir name
#
#command = "runMultijetBalance --file "+file_in+" --tag "+output_tag+" --submitDir "+submit_dir+" --config "+config_name
##command += " --inputTag data15_comm.*"
#command += ' --production phys-exotics'
#print command
#if not test: os.system(command)


#file_in = "../DijetFW/DijetResonanceAlgo/scripts/sampleLists/mjb2_gridSamples.txt"
#output_tag = "MJB"  # CHANGE BEFORE SUBMITTING - no periods!
#
#config_name = "$ROOTCOREBIN/data/MultijetBalanceAlgo/MultijetAlgo.config"
#
#timestamp = strftime("_%Y%m%d")
#output_tag += timestamp
#
#submit_dir = "gridOutput/gridJobs/submitDir_"+os.path.basename(file_in).rsplit('.',1)[0]+"."+output_tag # define submit dir name
#
#command = "runMultijetBalance --file "+file_in+" --tag "+output_tag+" --submitDir "+submit_dir+" --config "+config_name
##command += " --inputTag data15_comm.*"
#command += ' --production phys-exotics'
#print command
#if not test: os.system(command)
