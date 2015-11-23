#!/usr/bin/env python

################################################################
# calculateDoubleRatio.py                                      #
# A MJB second stage python script                             #
# Author Jeff Dandoy, UChicago                                 #
#                                                              #
# This script takes two files, a data and a MC MJB file.       #
# A double ratio of MJB  is created for each systematic.       #
# These systematics are still stand alone MJBs.                #
#                                                              #
# As a second step, systematic differences for each systematic #
# directory are created against the nominal DoubleMJB.         #
#                                                              #
################################################################

import os
import argparse

from ROOT import *

def calculateDoubleRatio(dataFileName, mcFileName):

  if not "MJB_initial" in mcFileName or not "MJB_initial" in dataFileName:
    print "Error, trying to run calculateDoubleRatio.py on non \"MJB_initial\" input ", mcFileName, dataFileName
    print "Exiting"
    return
##################### Get Double MJB Correction  #################################

  dirName = os.path.dirname(dataFileName)+'/'

  mcFile = TFile.Open(mcFileName, "READ");
  mcKeyList = [key.GetName() for key in mcFile.GetListOfKeys()] #List of top level objects
  mcDirList = [key for key in mcKeyList if "Iteration" in key] #List of all directories
  mcType = mcFileName.split('.')[-3]


  dataFile = TFile.Open(dataFileName, "READ");
  dataKeyList = [key.GetName() for key in dataFile.GetListOfKeys()] #List of top level objects
  dataDirList = [key for key in dataKeyList if "Iteration" in key] #List of all directories

  outName = 'hist.combined.'+mcType
  if "fit_MJB" in dataFileName:
    outName += '.Fit_DoubleMJB_initial.root'
  else:
    outName += '.DoubleMJB_initial.root'
  outFile = TFile.Open(dirName+outName, "RECREATE");


  print "Creating Double MJB Correction Hist"
  for dir in mcDirList:
    if not dir in dataDirList:
      print "Error, dir ", dir, " is in MC but not in data"
      continue

    #print "           ", dir
    outFile.mkdir( dir )
    newDir = outFile.Get( dir )
    mcDir = mcFile.Get( dir )
    dataDir = dataFile.Get( dir )

    ##Get recoilPt histogram and save it ##
    thisDataPtHist = dataDir.Get( "recoilPt_center" )
    thisDataPtHist.SetDirectory( newDir )

    mcHistList = [key.GetName() for key in mcDir.GetListOfKeys() if "MJB" in key.GetName()]
    dataHistList = [key.GetName() for key in dataDir.GetListOfKeys()]
    ### Save all histograms ###
    for histName in mcHistList:
      if not histName in dataHistList:
        print "Error, dir / hist ", dir, "/", histName, "is in MC but not in data"
      thisHist = dataDir.Get(histName)
      thisHist.SetName("Double"+thisHist.GetName())
      thisHist.SetTitle("Double "+thisHist.GetTitle())
      thisHist.GetYaxis().SetTitle("Double MJB")
      thisHist.SetDirectory( newDir )
      mcHist = mcDir.Get(histName)
      thisHist.Divide(mcHist)
#      for iBin in range(1, thisHist.GetNbinsX()+1):
#        thisHist.SetBinError(iBin, 0.)

  outFile.Write()
  outFile.Close()
  mcFile.Close()
  dataFile.Close()

############ Calculate systematic differences for Double MJB Corrections #####################3

  inFile = TFile.Open(dirName+outName, "READ");
  keyList = [key.GetName() for key in inFile.GetListOfKeys()] #List of top level objects
  dirList = [key for key in keyList if "Iteration" in key] #List of all directories
  nomDirName = [dir for dir in dirList if "Nominal" in dir]
  if( not len(nomDirName) == 1):
    print "Error, nominal directories in new output file are ", nomDir
    return
  else:
    nomDir = inFile.Get( nomDirName[0] )

  outName = outName.replace("DoubleMJB", "DoubleMJB_sys")
  outFile = TFile.Open(dirName+outName, "RECREATE");

  print "Creating Systematic Differences for Double MJB Correction Hists"
  for dir in dirList:
    #print "           ", dir
    outFile.mkdir( dir )
    newDir = outFile.Get( dir )
    thisDir = inFile.Get( dir )

    ##Get recoilPt histogram and save it ##
    thisPtHist = thisDir.Get( "recoilPt_center" )
    thisPtHist.SetDirectory( newDir )

    histList = [key.GetName() for key in thisDir.GetListOfKeys() if "DoubleMJB" in key.GetName()]
    for histName in histList:
      if "Nominal" in dir:
        thisHist = thisDir.Get( histName )
        thisHist.SetDirectory( newDir )
      else:
        thisHist = thisDir.Get( histName )
        thisHist.SetName( "diff_"+thisHist.GetName() )
        thisHist.SetDirectory( newDir )
        nomHist = nomDir.Get( histName )
        thisHist.Add(nomHist, -1.)

  outFile.Write()
  outFile.Close()
  inFile.Close()



if __name__ == "__main__":
  #put argparse before ROOT call.  This allows for argparse help options to be printed properly (otherwise pyroot hijacks --help) and allows -b option to be forwarded to pyroot
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--dataFile", dest='dataFile', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  parser.add_argument("--mcFile", dest='mcFile', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  args = parser.parse_args()

  calculateDoubleRatio(args.dataFile, args.mcFile)

