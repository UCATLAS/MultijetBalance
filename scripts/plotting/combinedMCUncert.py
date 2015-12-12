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
import argparse
from ROOT import *

def combinedMCUncert(nominalFileName, sysFileName):

  if not "scaled" in nominalFileName or not "scaled" in sysFileName:
    print "Error, files must run on scaled input"
    print "Exiting"
    return


  sysFile = TFile.Open(sysFileName, "READ")
  sysDir = [sysFile.Get(key.GetName()) for key in sysFile.GetListOfKeys() if "Nominal" in key.GetName()][0]
  iterationString = sysDir.GetName()[:10]

  nomFile = TFile.Open(nominalFileName, "UPDATE")
  nomFile.mkdir(iterationString+'_MCType')
  nomDir = nomFile.Get(iterationString+'_MCType')
  nomDir.cd()


  for key in sysDir.GetListOfKeys():
    obj = key.ReadObj()
    obj.SetDirectory(nomDir)
    obj.Write()

  nomFile.Close()
  sysFile.Close()

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--nominal", dest='nominal', default="",
           help="Nominal MC file Name")
  parser.add_argument("--sys", dest='sys', default="",
           help="Systematic MC File Name")
  args = parser.parse_args()
  combinedMCUncert(args.nominal, args.sys)
