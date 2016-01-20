#!/usr/bin/env python

import os, time
import argparse

from ROOT import *

def addDataSystToMC(mcFile, dataFile):

  if not "scaled_noDataSyst" in mcFile:
    print "Error, trying to run addDataSystToMC.py on non \"scaled_noDataSyst\" input ", mcFile
    print "Exiting"
    return

  inFile = TFile.Open(mcFile, "READ");
  outFileName = mcFile.replace('scaled_noDataSyst','scaled')
  outFile = TFile.Open(outFileName, "RECREATE");
  dataFile = TFile.Open(dataFile, "READ");

  keyList = [key.GetName() for key in inFile.GetListOfKeys()] #List of top level objects
  dirList = [key for key in keyList if "Iteration" in key] #List of all directories
  nominalDirName = [dirName for dirName in dirList if "Nominal" in dirName]
  if len(nominalDirName) != 1:
    print "Error, not 1 nominal directory in mcFile, but we find: ", nominalDirName
    exit(1)
  nominalDirName = nominalDirName[0]

  dataKeyList = [key.GetName() for key in dataFile.GetListOfKeys()] #List of top level objects
  dataDirList = [key for key in dataKeyList if "Iteration" in key] #List of all directories

##################### Get a new copy of original histograms and create new histograms  #################################
  print "Creating new hists "
  for dirName in dirList:
    thisHist = inFile.Get(dirName+"/recoilPt_PtBal_Fine")
    outFile.mkdir( dirName )
    newDir = outFile.Get( dirName )
    thisHist.SetDirectory(newDir)


  for dataDirName in dataDirList:
    ## Skip systematics that already exist in MC
    if dataDirName in dirList:
      continue

    thisHist = inFile.Get( nominalDirName+'/recoilPt_PtBal_Fine')
    outFile.mkdir( dataDirName )
    newDir = outFile.Get( dataDirName )
    thisHist.SetDirectory( newDir )

  outFile.Write()
  outFile.Close()
  dataFile.Close()
  inFile.Close()



if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--mcFile", dest='mcFile', default="", help="MC input file to add new systematics to")
  parser.add_argument("--dataFile", dest='dataFile', default="", help="Data input file to extract new systematics from")
  args = parser.parse_args()

  addDataSystToMC(args.mcFile, args.dataFile)

