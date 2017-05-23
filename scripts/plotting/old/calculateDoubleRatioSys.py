#!/usr/bin/env python

################################################################
# calculateDoubleRatioSys.py                                      #
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

def calculateDoubleRatioSys(inFileName, bootstrapFileName):

#  if (not ("MJB_initial" in mcFileName and "MJB_initial" in dataFileName)) and \
#     (not ("MJB_nominal" in mcFileName and "MJB_nominal" in dataFileName)) :
#    print "Error, trying to run calculateDoubleRatioSys.py on non \"MJB_initial\" input ", mcFileName, dataFileName
#    print "Exiting"
#    return
##################### Get Double MJB Correction Systematic #################################

  outName = inFileName.replace('DoubleMJB','DoubleMJB_sys')

  inFile = TFile.Open(inFileName, "READ");
  keyList = [key.GetName() for key in inFile.GetListOfKeys()] #List of top level objects
  dirList = [key for key in keyList if "Iteration" in key] #List of all directories
  nomDirName = [dir for dir in dirList if "Nominal" in dir]
  if( not len(nomDirName) == 1):
    print "Error, nominal directories in new output file are ", nomDirName
    return
  else:
    nomDir = inFile.Get( nomDirName[0] )

  bsList = []
  if len(bootstrapFileName) > 0:
    bsFile = TFile.Open(bootstrapFileName, "READ");
    bsKeyList = [key.GetName() for key in bsFile.GetListOfKeys()]
    bsList = [key for key in bsKeyList if "Iteration" in key]


  outFile = TFile.Open(outName, "RECREATE");

  print "Creating Systematic Differences for Double MJB Correction Hists"
  for dir in dirList:
    #print "           ", dir
    outFile.mkdir( dir )
    newDir = outFile.Get( dir )
    thisDir = inFile.Get( dir )

    ##Get recoilPt histogram and save it ##
    #thisPtHist = thisDir.Get( "recoilPt_center" )
    #thisPtHist.SetDirectory( newDir )

    histList = [key.GetName() for key in thisDir.GetListOfKeys() if "DoubleMJB" in key.GetName()]
    for histName in histList:
      if "Nominal" in dir:
        thisHist = thisDir.Get( histName )
        thisHist.SetDirectory( newDir )
      else:
        thisHist = thisDir.Get( histName )
        thisHist.SetName( "diff_"+thisHist.GetName() )
        thisHist.SetDirectory( newDir )


        if dir in bsList:
          bsDir = bsFile.Get( dir )
          nomHist = bsDir.Get( histName )
        else:
          nomHist = nomDir.Get( histName )

        thisHist.Add(nomHist, -1.)
        thisHist.Divide(nomHist)

  outFile.Write()
  outFile.Close()
  inFile.Close()



if __name__ == "__main__":
  #put argparse before ROOT call.  This allows for argparse help options to be printed properly (otherwise pyroot hijacks --help) and allows -b option to be forwarded to pyroot
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--inFile", dest='inFile', default="",
           help="Input file name")
  parser.add_argument("--bootstrapFile", dest='bootstrapFile', default="",
           help="Input file name")
  args = parser.parse_args()

  calculateDoubleRatioSys(args.inFile, args.bootstrapFile)

