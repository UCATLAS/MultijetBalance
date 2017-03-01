#!/usr/bin/env python

########### Script for converting V+jet systematics text files into Histograms #############
### Contact Jeff Dandoy at jeff.dandoy@cern.ch

import ROOT
from array import array

containerNames = ["EMJES_R4","LCJES_R4"]
#containerNames = ["EMJES_R4", "EMJES_R6", "LCJES_R4", "LCJES_R6"]



fileNames = []
histNames = []
for contName in containerNames:
  fileNames.append( "Vjet_"+contName+".txt" )
  histNames.append( contName+"_correction" )


hists = []
for iFile, fileName in enumerate(fileNames):

  ## Read in values from file
  f = open( fileName, 'r')
  binStart, binEnd, binValue = [], [], []
  for line in f:
    line = line.rstrip()
    line = line.split("  ")
    binStart.append( float(line[0]) )
    binEnd.append( float(line[1]) )
    binValue.append( float(line[2]) )

  ## Create proper histogram
  xBins = binStart + [binEnd[-1]]
  xBins_array = array('d', xBins)
  hists.append( ROOT.TH1F( histNames[iFile], histNames[iFile], len(binStart), xBins_array) )

  ## Fill histogram with values
  for iBin in range(len(binValue)):
    hists[-1].SetBinContent( hists[-1].FindBin( binStart[iBin]+0.01 ), binValue[iBin] )

outFile = ROOT.TFile.Open("Vjet_Systematics.root", "RECREATE")
for thisHist in hists:
  thisHist.Write()
outFile.Close()
