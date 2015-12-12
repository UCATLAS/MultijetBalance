#!/usr/bin/env python

#######################################################################################
# runSingleFit.py
#
# This is a script for running singleFit.py on all the expected mjj histograms.
# Lists to loop over include:
#   files: a set of different input files
#   fitsToUse: The fits to use
#   systematics: Systematic directories in the files
#   histNameVariations:  Potential variations to the input histograms (i.e. luminosity)
#   fitVariations:  Potential variations to fits (i.e. tolerance)
#       This variation should both change the fitting and include an addition
#       to the output name.
#
#######################################################################################


import subprocess, os, time, sys, glob
import datetime, argparse
import ROOT

def reformatHists(inFileName):

  pids = []
  logFiles = []
  NCORES = 6
  if not os.path.exists("parallelLogs/"):
    os.makedirs("parallelLogs/")

  #######   Setup the list of commands  ######

  inFile = ROOT.TFile.Open(inFileName, 'READ')
  outFileName = inFileName.replace('histOnlyFormat', 'scaled')
  outFile = ROOT.TFile.Open( outFileName, "RECREATE")

  for key in inFile.GetListOfKeys():
    thisKeyName = key.GetName()
    if thisKeyName.endswith('_recoilPt_PtBal_Fine'):
      dirName = thisKeyName.replace('_recoilPt_PtBal_Fine','')
      outFile.mkdir(dirName)
      outDir = outFile.Get(dirName)
      thisHist = inFile.Get( thisKeyName )
      thisHist.SetDirectory( outDir )
      thisHist.SetName('recoilPt_PtBal_Fine')

  print "Writing to file..."
  outFile.Write()
  outFile.Close()


if __name__ == "__main__":
  beginTime = datetime.datetime.now().time()

  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--file", dest='file', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  args = parser.parse_args()

  reformatHists(args.file)
  print "Starting reformatting at", beginTime
  print "Finished reformatting at", datetime.datetime.now().time()



