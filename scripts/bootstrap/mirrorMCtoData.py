#!/usr/bin/env python

import os, time
import argparse

from ROOT import *

def mirrorMCtoData(mcFileName):

  nToys = 100
  mcFile = TFile.Open(mcFileName, "UPDATE");

  mcKeyList = [key.GetName() for key in mcFile.GetListOfKeys()] #List of top level objects

#################### Get a new copy of original histograms and create new histograms  #################################

  for dirName in mcKeyList:
    print dirName
    for i in range(0, 100):
      thisHist = mcFile.Get(dirName+'/MJB_Fine')
      newDirName = dirName+'_'+str(i)
      mcFile.mkdir( newDirName )
      newDir = mcFile.Get( newDirName )
      thisHist.SetDirectory( newDir )
  mcFile.Write()
  mcFile.Close()

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--mcFile", dest='mcFile', default="", help="MC file to add new histograms")
  args = parser.parse_args()

  mirrorMCtoData(args.mcFile)

