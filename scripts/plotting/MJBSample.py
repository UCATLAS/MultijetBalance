#!/usr/bin/env python

###################################################################
# MJBSample.py                                                    #
# A MJB python script                                             #
# Author Jeff Dandoy, UChicago                                    #
#                                                                 #
# Define Sample objects correspond to one data or MC sample.      #
# Samples are associated to a set of input histograms, and        #
# new TFiles are created for scaled and combined histograms.      #
#                                                                 #
# This script may be run directly to scale a single MC file       #
# (typically one JZ slice) according to the cutflow.              #
# Output files may then be combined directly with hadd.           #
#                                                                 #
###################################################################

import os
import argparse
from ROOT import *


### A generic sample type ###
class Sample():
  def __init__(self, tag):
    self.tag = tag
    self.verbose = True #Print each command as it runs
    self.inputDir = None #Input directory for raw histograms
    self.redo = False #Recreate intermediary files


    self.histFile = ""  #File with the histograms

  def setInputDir(self, inputDir):
    self.inputDir = inputDir

### A sample extension for data ###
class DataSample(Sample):
  def __init__(self, tag):
    Sample.__init__(self, tag )
    self.isMC = False

  def gatherInput(self):

    ### Do not recreate combined file if it already exists and redo is false
    combinedFileName = self.inputDir+'/files/'+self.tag+'.all.root'
    self.histFile = combinedFileName

    if os.path.isfile(combinedFileName) and not self.redo :
      if(self.verbose):  print combinedFileName, "exists, will not recreate"
      return
   
    ### Hadd all data files together 
    command = 'hadd -f '+self.inputDir+'/files/'+self.tag+'.all.root '+self.inputDir+'/hist/*'+self.tag+'*_hist.root'
    if(self.verbose):  print command
    os.system(command)



### A sample extension for MC ###
class MCSample(Sample):
  def __init__(self, tag=""):
    Sample.__init__(self, tag )
    self.isMC = True

  def gatherInput(self):
    
    ### Do not recreate combined file if it already exists and redo is false
    combinedFileName = self.inputDir+'/files/'+self.tag+'.all.root'
    self.histFile = combinedFileName

    if os.path.isfile(combinedFileName) and not self.redo:
      if(self.verbose):  print combinedFileName, "exists, will not recreate"
      return

    ### Scale MC files by correct normalization
    raw_files = glob.glob(self.inputDir+'/hist/*'+self.tag+'*_hist.root')
    for raw_file in raw_files:
      if(self.verbose):  print "scalehist on ", raw_file
      scaleHist.scaleHist(raw_file)
    
    ### Hadd all MC files together 
    command = 'hadd '+self.inputDir+'/files/'+self.tag+'.all.root '+self.inputDir+'/files/*'+self.tag+'*_hist.scaled.root'
    if(self.verbose):  print command
    os.system(command)
    

### A function to scale a single MC input file ###
### A new output file is created with the same name, but ending in 'scaled.root' ###
def scaleHist(file):

  ### Create input and output files ###
  inFile = TFile.Open(file, "READ");
  outFileName = file.replace('.root', '.scaled.root').replace('/hist/','/files/')
  outFile = TFile.Open(outFileName, "RECREATE");
  
  print "scaleHist", file, "->", outFileName

  ### Get the cutflow histogram used to scale ###
  keyList = [key.GetName() for key in inFile.GetListOfKeys()] #List of top level objects
  cutflowName = [key for key in keyList if key.startswith("cutflow") and not "weighted" in key]
  if( not len(cutflowName) == 1):
    print "Error, cutflow names are ", cutflowName
    return
  else:
    cutflowName = cutflowName[0]
  thisCutflow = inFile.Get( cutflowName )
  numEvents = thisCutflow.GetBinContent(1)
  scaleFactor = 1./numEvents


  ### Recreate each MJB directory in the input file ###
  dirList = [key for key in keyList if "Iteration" in key] #List of all directories

  for dir in dirList:
    outFile.mkdir( dir )
    newDir = outFile.Get( dir )
    oldDir = inFile.Get( dir )

    histList = [key.GetName() for key in oldDir.GetListOfKeys()]
    ### Save all histograms ###
    for histName in histList:
      thisHist = oldDir.Get(histName)
      thisHist.SetDirectory( newDir )
      thisHist.Scale( scaleFactor )

  outFile.Write()
  outFile.Close()
  inFile.Close()

### Add a new systematic variation, corresponding to the nominal result of a different MC sample ###
def addMCSyst( nomSample, sysSample ):
  if nomSample.verbose:  print "addMCSyst on ",nomSample.tag


  ### Check if there is a 'Nominal' hist directory in the systematic file, and exit if not ###
  sysFile = TFile.Open(sysSample.histFile, "READ")
  sysDir = [sysFile.Get(key.GetName()) for key in sysFile.GetListOfKeys() if "Nominal" in key.GetName()]
  if len(sysDir) != 1:
    print "Error, MC systematic sample", sysSample.tag, "does not have only 1 Nominal file, but contains the following.  Exiting.."
    print sysDir
    exit(1)

  sysDir = sysDir[0]
  iterationString = sysDir.GetName()[:10]

  ### Create new directory in nominal file ###
  nomFile = TFile.Open(nomSample.histFile, "UPDATE")
  newDirName = sysDir.GetName().replace('Nominal',sysSample.tag)

  ## If directory already exists, do not recreate ##
  if nomFile.Get( newDirName ):
    if nomSample.verbose:  print "nom file", nomSample.tag, " already has dir for MC syst file", sysSample.tag, ", we will not recreate"
    return

  nomFile.mkdir( newDirName )
  nomDir = nomFile.Get( newDirName )
  nomDir.cd()

  ### Loop over each histogram in the systematic sample directory, and add it to the nominal MC file ###
  for key in sysDir.GetListOfKeys():
    obj = key.ReadObj()
    obj.SetDirectory(nomDir)
    obj.Write()

  nomFile.Close()
  sysFile.Close()



if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--file", dest='file', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  args = parser.parse_args()

  scaleHist(args.file)

