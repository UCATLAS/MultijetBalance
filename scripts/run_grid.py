#!/usr/bin/python

import os, math, sys
from time import strftime

test = False # does not run the jobs
if not test:
  if not os.path.exists("gridOutput"):
    os.system("mkdir gridOutput")
  if not os.path.exists("gridOutput/gridJobs"):
    os.system("mkdir gridOutput/gridJobs")

timestamp = strftime("_%Y%m%d")
files = []
outputTags = []

#validation = True
#jetTypes = ["EM"]  # no LC for Insitu validation!
#valTypes = ["VA"]
###valTypes = ["VA", "VF"]

validation = False
jetTypes = ["EM"]
#jetTypes = ["EM", "LC"]

extraTag = "_BS"


#files.append("../DijetFW/DijetResonanceAlgo/scripts/sampleLists/BootStrap_test_gridSamples.txt")
#outputTags.append("test")

files.append("../DijetFW/DijetResonanceAlgo/scripts/sampleLists/QCDPythia8_EXOT2_gridSamples.txt")
outputTags.append("QCDPy")
files.append("../DijetFW/DijetResonanceAlgo/scripts/sampleLists/QCDSherpa_JETM1_gridSamples.txt")
#files.append("../DijetFW/DijetResonanceAlgo/scripts/sampleLists/QCDSherpa_gridSamples.txt")
outputTags.append("QCDS")
files.append("../DijetFW/DijetResonanceAlgo/scripts/sampleLists/QCDHerwig_EXOT2_gridSamples.txt")
outputTags.append("QCDH")
files.append("../DijetFW/DijetResonanceAlgo/scripts/sampleLists/MJBD_EXOT2_gridSamples.txt")
outputTags.append("Ex")
files.append("../DijetFW/DijetResonanceAlgo/scripts/sampleLists/MJBD_Debug_gridSamples.txt")
outputTags.append("Db")

if validation:
  for iFile, file_in in enumerate(files):

    for jetType in jetTypes:
      for valType in valTypes:

        config_name = "$ROOTCOREBIN/data/MultijetBalanceAlgo/ValidationAlgo_"+valType+"_"+jetType+".config"
        output_tag = valType+"_"+jetType+"_"+outputTags[iFile]
        output_tag += extraTag
        output_tag += timestamp
        submit_dir = "gridOutput/gridJobs/submitDir_"+os.path.basename(file_in).rsplit('.',1)[0]+"."+output_tag # define submit dir name
        command = "runMultijetBalance --file "+file_in+" --tag "+output_tag+" --submitDir "+submit_dir+" --config "+config_name
        command += ' --production phys-exotics'
        if not "Sherpa" in file_in:
          command += ' --oneJobPerFile'
        print command
        if not test: os.system(command)

else:

  for iFile, file_in in enumerate(files):
    for jetType in jetTypes:
      config_name = "$ROOTCOREBIN/data/MultijetBalanceAlgo/MultijetAlgo_"+jetType+".config"
      output_tag = jetType+'_'+outputTags[iFile]

      output_tag += extraTag
      output_tag += timestamp
      submit_dir = "gridOutput/gridJobs/submitDir_"+os.path.basename(file_in).rsplit('.',1)[0]+"."+output_tag # define submit dir name
      command = "runMultijetBalance --file "+file_in+" --tag "+output_tag+" --submitDir "+submit_dir+" --config "+config_name
      command += ' --production phys-exotics'
      if not "Sherpa" in file_in:
        command += ' --oneJobPerFile'
      print command
      if not test: os.system(command)

