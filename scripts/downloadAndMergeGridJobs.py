#!/usr/bin/env python

#################################################################################################################
#   This is a python script for grabbing and sorting results from DijetResonanceAlgo grid jobs                  #
#   This script should be run from the top directory                                                            #
#   When grid jobs are submitted using run_grid.py, log files are put into gridOutput/gridJobs/                 #
#   This script looks through gridOutput/gridJobs/ for files that should be downloaded                          #
#   It checks this list of files against the trees downloaded in gridOutput/gridResults/ , and will only        #
#      download files that have not been previously downloaded                                                  #
#   It then combines similar trees from different grid sites, and sorts output files according to trees,        #
#      cutflows, histograms, and log files                                                                      #
#
#   Prerequisite: setup dq2
#         setupATLAS
#         localSetupDQ2Client
#         voms-proxy-init -voms atlas --valid 96:00
#
#################################################################################################################

import os, sys, glob

f_download = False #download files
f_merge = True #merge downloaded files

gridJobs = glob.glob("gridOutput/gridJobs/*/outputContainers.txt") #Get lists of the output containers
jobNames = []

for iFile, file in enumerate(gridJobs):
  file = open(file, 'r')
  for line in file:
    if not ( line.find("#") == 0 or len(line) < 10 ):
      thisName = line.rstrip()
      if thisName[-1] == '/':
        thisName = thisName[:-1]
      jobNames.append(thisName)

#####  Download the output files if they don't already exist  #####
if f_download:
  if not os.path.exists('gridOutput/rawDownload'):
    os.mkdir('gridOutput/rawDownload')
  previousJobs = [ name[:-1] for name in glob.glob('gridOutput/rawDownload/*')] #Jobs that have already been downloaded

  for fileName in jobNames:
    if not any(fileName in thisFileName for thisFileName in previousJobs): #If file wasn't already downloaded
      #print "hadn't downloaded ", fileName
      os.system('dq2-get '+fileName+'* | tee -a gridOutput/rawDownloadLogFile.txt') #Download new files
      os.system('mv '+fileName+'* gridOutput/rawDownload/')

#####  Merge output files from different grid sites and sort output files by type  #####
if f_merge:
  files = glob.glob("gridOutput/rawDownload/user.*")

  ##Sort files by type##
  filesHist = [ x for x in files if "_hist-output.root" in x ]
  filesCutflow = [ x for x in files if "_cutflow.root" in x ]
  filesTree = [ x for x in files if "_tree.root" in x ]
  if not os.path.exists('gridOutput/hists'):
    os.system('mkdir gridOutput/hists')
  if not os.path.exists('gridOutput/cutflows'):
    os.system('mkdir gridOutput/cutflows')
  if not os.path.exists('gridOutput/trees'):
    os.system('mkdir gridOutput/trees')
  if not os.path.exists('gridOutput/emptyFiles'):
    os.system('mkdir gridOutput/emptyFiles')

  ##Merge all root files from the same job##
  for iFile, file in enumerate(jobNames):
    if os.path.exists( 'gridOutput/hists/hist_'+file+'.root'):
      continue;
    mergeHists = [ x for x in filesHist if file in x ]
    mergeCutflows = [ x for x in filesCutflow if file in x ]
    mergeTrees = [ x for x in filesTree if file in x ]

    ## Merge and Move Histograms ##
    outString = ''
    if len(mergeHists) == 1 and len(glob.glob(mergeHists[0]+"/*.root")) == 1:
      thisFile = glob.glob(mergeHists[0]+"/*.root")
      os.system('cp '+thisFile[0]+' gridOutput/hists/hist_%s.root' %(file) )
    else:
      for mergeFile in mergeHists:
        if len(os.listdir(mergeFile)) > 0:
          outString += mergeFile+'/*.root '
        else:
          os.system('cp '+mergeFile+' gridOutput/emptyFiles')  #Move any empty files
      if len(outString) > 0:
        print 'hadd gridOutput/hists/hist_%s.root %s' %(file, outString)
        os.system('hadd gridOutput/hists/hist_%s.root %s' %(file, outString) ) #hadd similar files

    ## Merge and Move Cutflows ##
    outString = ''
    if len(mergeCutflows) == 1 and len(glob.glob(mergeCutflows[0]+"/*.root")) == 1:
      thisFile = glob.glob(mergeCutflows[0]+"/*.root")
      os.system('cp '+thisFile[0]+' gridOutput/cutflows/cutflow__%s.root' %(file) )
    else:
      for mergeFile in mergeCutflows:
        if len(os.listdir(mergeFile)) > 0:
          outString += mergeFile+'/*.root '
        else:
          os.system('cp '+mergeFile+' gridOutput/emptyFiles')  #Move any empty files
      if len(outString) > 0:
        os.system('hadd gridOutput/cutflows/cutflow_%s.root %s' %(file, outString) ) #hadd similar files

    ## Merge and Move Cutflows ##
    outString = ''
    if len(mergeTrees) == 1 and len(glob.glob(mergeTrees[0]+"/*.root")) == 1:
      thisFile = glob.glob(mergeTrees[0]+"/*.root")
      os.system('cp '+thisFile[0]+' gridOutput/trees/cutflow__%s.root' %(file) )
    else:
      for mergeFile in mergeTrees:
        if len(os.listdir(mergeFile)) > 0:
          outString += mergeFile+'/*.root '
        else:
          os.system('cp '+mergeFile+' gridOutput/emptyFiles')  #Move any empty files
      if len(outString) > 0:
        os.system('hadd gridOutput/trees/cutflow_%s.root %s' %(file, outString) ) #hadd similar files
