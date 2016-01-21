#!/usr/bin/env python

import os, time
import argparse

from ROOT import *

def stripExtraHists(nomFileName, bsFileName):

#  if not "initial" in inFileName:
#    print "Error, trying to run stripExtraHists.py on non \"initial\" input ", inFileName
#    print "Exiting"
#    return

  nomFile = TFile.Open(nomFileName, "READ");
  bsFile = TFile.Open( bsFileName, "UPDATE")

  keyList = [key.GetName() for key in nomFile.GetListOfKeys()] #List of top level objects
  dirList = [key for key in keyList if "Iteration" in key] #List of all directories

##################### Get a new copy of original histograms and create new histograms  #################################
  print "Moving recoilPt_PtBal_Fine to bootstrap file"
  for dirName in dirList:
    thisHist = nomFile.Get(dirName+"/recoilPt_PtBal_Fine")
    bsFile.mkdir( dirName )
    newDir = bsFile.Get( dirName )
    thisHist.SetDirectory(newDir)

  bsFile.Write()
  bsFile.Close()
  nomFile.Close()



if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--nomFile", dest='nomFile', default="", help="Nominal file of histograms to strip and add to Bootstrap")
  parser.add_argument("--bsFile", dest='bsFile', default="", help="Bootstrap input file of good histograms")
  args = parser.parse_args()

  stripExtraHists(args.nomFile, args.bsFile)

