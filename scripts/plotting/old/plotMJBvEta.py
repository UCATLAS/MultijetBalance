#!/usr/bin/env python

###################################################################
# plotMJBvEta.py                                                  #
# A MJB second stage python script                                #
# Author Jeff Dandoy, UChicago                                    #
#                                                                 #
# This script takes an MJB file and plots the MJB correction      #
# for each eta range on a single plot.                            #
#                                                                 #
###################################################################


import os
import argparse

from ROOT import *
import AtlasStyle



def plotMJBvEta(file, binnings):

  if not "MJB" in file:
    print "Error:  plotMJBvEta.py must run on a file with  \"MJB\", but was given", file
    print "Exiting..."
    return

  binnings = binnings.split(',')
  for iBinning in range(len(binnings)):
    if not binnings[iBinning][0] == '_':
      binnings[iBinning] = '_'+binnings[iBinning]
  print "Binnings are ", binnings

  outDir = file[:-5]+'/'
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  outDir += "plotMJBvEta/"
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  AtlasStyle.SetAtlasStyle()

  gROOT.ProcessLine("gErrorIgnoreLevel = 2000") #Ignore TCanvas::Print info

  inFile = TFile.Open(file, "READ");
  keyList = [key.GetName() for key in inFile.GetListOfKeys()] #List of top level objects
  dirList = [key for key in keyList if "Iteration" in key] #List of all directories

  #nomDir = [dir for dir in dirList if "NoCorr" in dir]
  nomDir = [dir for dir in dirList if "Nominal" in dir]
  if( not len(nomDir) == 1):
    print "Error, nominal directories are ", nomDir
    return
  else:
    nomDir = inFile.Get( nomDir[0] )

  c1 = TCanvas()

  histList = [key.GetName() for key in nomDir.GetListOfKeys() if "MJB" in key.GetName()]


  for binning in binnings:
    leg = TLegend(0.80, 0.15, 0.97, 0.95)
    pad1 = TPad("pad1", "", 0, 0, 0.85, 1)
    pad1.Draw()
    pad1.cd()
    hists = []
    counter = 0
    for histName in histList:
      if not binning in histName:
        continue
      counter += 1
      hists.append( nomDir.Get( histName ) )
      hists[-1].SetMarkerColor( gStyle.GetColorPalette(70* (len(hists)-1) ) )
      hists[-1].SetLineColor( gStyle.GetColorPalette(70* (len(hists)-1) ) )
      hists[-1].GetYaxis().SetRangeUser(0.9, 1.1)
      hists[-1].GetXaxis().SetMoreLogLabels(True)
      if( "DoubleMJB" in histName):
        hists[-1].GetXaxis().SetRangeUser(300, 2000)
      if(counter == 1):
        if type(hists[-1]) == TGraph:
          hists[-1].Draw("apXl")
        else:
          hists[-1].Draw()
        leg.AddEntry(hists[-1], "Nominal", "l")
      else:
        if type(hists[-1]) == TGraph:
          hists[-1].Draw("same pXl")
        else:
          hists[-1].Draw("same")
        leg.AddEntry(hists[-1], "Eta "+str(counter), "l")

    c1.cd()
    leg.Draw()
    pad1.SetLogx()
    c1.SaveAs(outDir+"/MJBvEta"+binning+".png", "png")
    pad1.Clear()
    c1.Clear()


  inFile.Close()


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--file", dest='file', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  parser.add_argument("--binnings", dest='binnings', default="",
           help="Comma Seperated List of binnings used")
  args = parser.parse_args()

  plotMJBvEta(args.file, args.binnings)
