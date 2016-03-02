#!/usr/bin/env python

import os, time
import argparse

from ROOT import *

def addNominalToBootstrap(nomFileName, bsFileName):

#  if not "initial" in inFileName:
#    print "Error, trying to run addNominalToBootstrap.py on non \"initial\" input ", inFileName
#    print "Exiting"
#    return

  nomFile = TFile.Open(nomFileName, "READ");
  bsFile = TFile.Open( bsFileName, "UPDATE")

  keyList = [key.GetName() for key in nomFile.GetListOfKeys()] #List of top level objects
  histList = [key for key in keyList if "recoilPt_PtBal_Fine" in key]

  print histList

#################### Get a new copy of original histograms and create new histograms  #################################
  print "Moving recoilPt_PtBal_Fine to bootstrap file"
  for histName in histList:
    dirName = histName.replace('_recoilPt_PtBal_Fine','')
    thisHist = nomFile.Get( histName )
    thisHist.SetName('recoilPt_PtBal_Fine')
    thisHist.SetTitle('recoilPt_PtBal_Fine')
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

  addNominalToBootstrap(args.nomFile, args.bsFile)

