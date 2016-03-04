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

import ROOT

iterationNumber="1"
jetType='_AntiKt4EMTopo'

def renameMJB(sysFileName):


  histName = "diff_DoubleMJB"

  outFile = ROOT.TFile.Open(sysFileName.replace('Fit_DoubleMJB_sys_final','Renamed'), "RECREATE");

  sysFile = ROOT.TFile.Open(sysFileName, "READ");
  keyList = [key.GetName() for key in sysFile.GetListOfKeys()] #List of top level objects
  dirList = [key for key in keyList if "Iteration" in key] #List of all directories


  histName = "/graph_diff_DoubleMJB"

  ## Get Systematics to Rename ##
  for dirName in dirList:
    if "Nominal" in dirName:
      continue
    if "_JCS_" in dirName:
      continue
#    if "LAr_" in dirName:
#      continue
    if "MJB_a25_" in dirName or "MJB_a35_" in dirName or \
       "MJB_b12_" in dirName or "MJB_b08_" in dirName or \
       "MJB_pta75_" in dirName or "MJB_pta85_" in dirName or \
       "MJB_ptt27_" in dirName or "MJB_ptt23_" in dirName:
      continue


    outHistName = dirName

    outHistName = outHistName.replace('Iteration'+iterationNumber+'_', '')
    outHistName = outHistName.replace('pos','up').replace('neg','down')
    outHistName = outHistName.replace('MCTYPE', 'MC12').replace('Nominal','MJB_Nominal')
    outHistName = outHistName.replace('MJB_pta90_','MJB_Asym_').replace('MJB_b15_','MJB_Beta_')
    outHistName = outHistName.replace('MJB_pta70_','MJB_Asym_').replace('MJB_b05_','MJB_Beta_')
    outHistName = outHistName.replace('MJB_a40_','MJB_Alpha_').replace('MJB_ptt30_','MJB_Threshold_')
    outHistName = outHistName.replace('MJB_a20_','MJB_Alpha_').replace('MJB_ptt20_','MJB_Threshold_')
    outHistName = outHistName.replace('MCType','MJB_Fragmentation')
    outHistName = outHistName.replace('Comupition','Composition')
    outHistName = outHistName.replace('LAr_Esmear','Gjet_GamEsmear')
#    outHistName = outHistName.replace('','').replace('','')
    outHistName += jetType


    if 'Nominal' in dirName:

      statHist = sysFile.Get( dirName+"/graph_DoubleMJB" )
      statHist.SetName("MJB_Stat"+jetType)
      statHist.SetTitle("MJB_Stat"+jetType)
      d1, d2 = ROOT.Double(0), ROOT.Double(0)
      for iP in range(0, statHist.GetN()):
        statHist.GetPoint( iP, d1, d2 )
        print iP, d1, d2
        print statHist.GetErrorY(iP)
        statHist.SetPoint(iP, d1, statHist.GetErrorY(iP) )
        statHist.SetPointError(iP, 0., 0.)
      outFile.cd()
      statHist.Write()


      inHist = sysFile.Get( dirName+"/graph_DoubleMJB" )
    else:
      inHist = sysFile.Get( dirName+histName )

    inHist.SetName(outHistName)
    inHist.SetTitle(outHistName)

    outFile.cd()
    inHist.Write()


  outFile.Close()
  sysFile.Close()



if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--file", dest='file', default="",
           help="Input file")
  parser.add_argument("--mcFile", dest='mcFiles', default="",
           help="MC files")
  args = parser.parse_args()

  thisDir = "/home/jdandoy/Documents/Dijet/MultijetBalanceFW/gridOutput/workarea/Sys_Iter1_EM/workarea/"
  #thisDir = "/home/jdandoy/Documents/Dijet/MultijetBalanceFW/gridOutput/workarea/Iter1_EM_BS3/workarea/"
  sysFile = thisDir+"hist.combined.Pythia.Fit_DoubleMJB_sys_final.root"
#  nominalFile = thisDir+"hist.combined.Pythia.Fit_DoubleMJB_final.root"
  renameMJB(sysFile)
