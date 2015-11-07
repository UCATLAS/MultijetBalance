#!/usr/bin/env python

###################################################################
# calculateFinalMJBGraphs.py                                      #
# A MJB second stage python script                                #
# Author Jeff Dandoy, UChicago                                    #
#                                                                 #
# This script takes any MJBCorrection or DoubleMJBCorrection file #
# of histograms and makes them TGraphErrors using exact x-axis    #
# locations taken from a finely binned recoilPt histogram.        #
#                                                                 #
# This is necessary as the number of events is not flat within    #
# a recoilPt_binned bin. I.E. a single MJB bin can                #
# span from 1700 to 1900 GeV, even if there are few events above  #
# 1800 GeV.                                                       #
#                                                                 #
###################################################################

import os
from array import array
import argparse
import math

from ROOT import *

iterationNumber="0"
jetType='_AntiKt4EmTopo'

def renameMJB(file, mcFiles, binning):

  if not "MJB_sys_initial":
    print "Error, trying to run calculateMJBHists.py on non \"MJB_sys_initial\" input ", file
    print "Exiting"
    return


  histName = "diff_DoubleMJB_"+binning

  inFile = TFile.Open(file, "READ");
  outFile = TFile.Open(file.replace('initial','renamed'), "RECREATE");

  keyList = [key.GetName() for key in inFile.GetListOfKeys()] #List of top level objects
  dirList = [key for key in keyList if "Iteration" in key] #List of all directories


  outHistNames = []
  inDirNames = []


  ## Get Systematics to Rename ##
  for dirName in dirList:
    if "_JCS_" in dirName:
      continue
    outHistName = dirName
    if('MJB' in outHistName):
      outHistName = outHistName.replace('_pos', '').replace('_neg', '')
    outHistName = outHistName.replace('Iteration'+iterationNumber+'_', '').replace('pos','up').replace('neg','down').replace('MCTYPE', 'MC12')
    outHistName += jetType

    inDirNames.append( dirName )
    outHistNames.append( outHistName )


  ## Rename existing systematics ##
  for iHist, inDirName in enumerate( inDirNames ):
    if 'Nominal' in inDirName:
      inHist = inFile.Get( inDirName+'/'+histName.replace('diff_',''))
    else:
      inHist = inFile.Get( inDirName+'/'+histName)
    inHist.SetName( outHistNames[iHist] )
    inHist.SetTitle( outHistNames[iHist] )
    outFile.cd()
    inHist.Write()


  nomDir = [dir for dir in dirList if "Nominal" in dir][0]
  nominalHist = inFile.Get(nomDir+'/'+histName.replace('diff_','') )

  ## Calculate Statistical Systematics ##
  startBin = nominalHist.GetXaxis().FindBin(1000)
  endBin = nominalHist.FindLastBinAbove(0)
  print "Statistical variations for bins ", startBin, endBin
  for iBin in range(startBin, endBin+1):
    statHistName = 'MJB_Stat'+str(iBin)+jetType
    newStatHist = nominalHist.Clone()
    newStatHist.SetName( statHistName )
    newStatHist.SetTitle( statHistName )
    newStatHist.SetBinContent(iBin, newStatHist.GetBinContent(iBin)*(1+newStatHist.GetBinError(iBin)) )
    newStatHist.Add(nominalHist, -1.)
    outFile.cd()
    newStatHist.Write()

  ## Calculate MC Systematics ##
  mcFiles = mcFiles.split(',')
  mcHistUp = nominalHist.Clone()
  mcHistDn = nominalHist.Clone()
  mcHistUp.SetName('MJB_Fragmentation_up'+jetType)
  mcHistUp.SetTitle('MJB_Fragmentation_up'+jetType)
  mcHistDn.SetName('MJB_Fragmentation_down'+jetType)
  mcHistDn.SetTitle('MJB_Fragmentation_down'+jetType)
  for iBin in range(1, nominalHist.GetNbinsX()+1):
    mcHistUp.SetBinContent(iBin, 0)
    mcHistUp.SetBinError(iBin, 0)
    mcHistDn.SetBinContent(iBin, 0)
    mcHistDn.SetBinError(iBin, 0)

  for mcFile in mcFiles:
    inMCFile = TFile.Open(mcFile, "READ")
    thisMCHist = inMCFile.Get(nomDir+'/'+histName.replace('diff_','') )

    for iBin in range(1, nominalHist.GetNbinsX()+1):
      thisBinDiff = thisMCHist.GetBinContent(iBin) - nominalHist.GetBinContent(iBin)
      if thisBinDiff > mcHistUp.GetBinContent(iBin):
        mcHistUp.SetBinContent(iBin, math.fabs(thisBinDiff) )
        mcHistDn.SetBinContent(iBin, -math.fabs(thisBinDiff) )

  outFile.cd()
  mcHistUp.Write()
  mcHistDn.Write()

  outFile.Close()
  inFile.Close()



if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--file", dest='file', default="",
           help="Input file")
  parser.add_argument("--mcFile", dest='mcFiles', default="",
           help="MC files")
  parser.add_argument("--binning", dest='binning', default="Fine",
           help="Single binning to use")
  args = parser.parse_args()
  renameMJB(args.file, args.mcFiles, args.binning)
