#!/usr/bin/env python

###################################################################
# plotAll.py                                                      #
# A MJB second stage python script                                #
# Author Jeff Dandoy, UChicago                                    #
#                                                                 #
# This script directly plots every TObject within the file.       #
# Systematic histograms are plotted as the relative difference    #
# with the Nominal.                                               #
#                                                                 #
###################################################################


#### This file plots every sing histogram for Nominal ,and the relative difference for every systematics histogram

import os
import argparse

from ROOT import *
import AtlasStyle

def plotAll(file):
  outDir = file[:-5]+'/'
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  outDir += "plotAll/"
  if not os.path.exists(outDir):
    os.mkdir(outDir)

  AtlasStyle.SetAtlasStyle()

  gROOT.ProcessLine("gErrorIgnoreLevel = 2000") #Ignore TCanvas::Print info

  inFile = TFile.Open(file, "READ");
  keyList = [key.GetName() for key in inFile.GetListOfKeys()] #List of top level objects
  dirList = [key for key in keyList if "Iteration" in key] #List of all directories

  nomDir = [dir for dir in dirList if "Nominal" in dir]
  if( not len(nomDir) == 1):
    print "Error, nominal directories are ", nomDir
    return
  else:
    nomDir = inFile.Get( nomDir[0] )

  c1 = TCanvas()

##################### Plot every single histogram  #################################

  print "Plotting All Histograms for "
  for dir in dirList:
    print "           ", dir
    if not os.path.exists(outDir+dir):
      os.mkdir( outDir+dir )

    thisDir = inFile.Get( dir )
    histList = [key.GetName() for key in thisDir.GetListOfKeys()]
    ### Save all histograms ###
    for histName in histList:
      print dir, histName
      thisHist = thisDir.Get(histName)
      if( "Pt" in histName):
        thisHist.GetXaxis().SetRangeUser(0, 2000.)
      if( not "Nominal" in dir):
        nomHist = nomDir.Get(histName)
        thisHist.Add(nomHist, -1)
        thisHist.Divide(nomHist)
        thisHist.GetYaxis().SetTitle("Relative Difference w/ Nominal")
      if type(thisHist) == TH2F or type(thisHist) == TH3F:
        thisHist.Draw("colz")
      else:
        thisHist.Draw()
      c1.SaveAs( outDir+dir+"/"+thisHist.GetName()+".png" )
      c1.Clear()
  inFile.Close()

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--file", dest='file', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  args = parser.parse_args()

  plotAll(args.file)
