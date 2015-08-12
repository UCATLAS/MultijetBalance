#!/usr/bin/env python

###################################################################
# scaleHist.py                                                   #
# A MJB second stage python script                                #
# Author Jeff Dandoy, UChicago                                    #
#                                                                 #
# This script takes the output of MultijetBalanceAlgo and scales  #
# the MC according to the number of events that were run on.      #
# Output files may then be combined directly with hadd            #
#                                                                 #
###################################################################

import os
import argparse
from ROOT import *

def scaleHist(file):
  inFile = TFile.Open(file, "READ");
  outFile = TFile.Open(file[:-5]+".scaled.root", "RECREATE");

  keyList = [key.GetName() for key in inFile.GetListOfKeys()] #List of top level objects

  #Get cutflow histogram for MC scaling
  if not "data" in file:
    cutflowName = [key for key in keyList if key.startswith("cutflow") and not "weighted" in key]
    if( not len(cutflowName) == 1):
      print "Error, cutflow names are ", cutflowName
      return
    else:
      cutflowName = cutflowName[0]
    thisCutflow = inFile.Get( cutflowName )
    numEvents = thisCutflow.GetBinContent(1)
    scaleFactor = 1./numEvents
  else:
    scaleFactor = 1.


  dirList = [key for key in keyList if "Iteration" in key] #List of all directories


  print "Scaling and saving hists for file ", file
  for dir in dirList:
    #print "           ", dir
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

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--file", dest='file', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  args = parser.parse_args()

  scaleHist(args.file)

