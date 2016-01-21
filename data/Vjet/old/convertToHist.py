#!/usr/bin/env python

########### Script for converting V+jet systematics text files into Histograms #############
### Contact Jeff Dandoy at jeff.dandoy@cern.ch

from ROOT import *

containerNames = ["EMJES_R4", "EMJES_R6", "LCJES_R4", "LCJES_R6"]
#systematicNames = ["gammajetdphi","gammajetoutofcone","gammajetgenerator","gammajetsubleadingjet","gammajetphscalez","gammajetpurity","gammajetphscalemat","gammajetphscaleps","gammajetphsmear","gammajetstat1","gammajetstat2","gammajetstat3","gammajetstat4","gammajetstat5","gammajetstat6","gammajetstat7","gammajetstat8","gammajetstat9","gammajetstat10","gammajetstat11","gammajetstat12","gammajetstat13","zjetescalez","zjetgenerator","zjetoutofcone","zjetsubleadingjet","zjetescalemat","zjetescaleps","zjetdphi","zjetjvf","zjetesmear","zjetmusmearid","zjetmuscale","zjetmusmearms","zjetstat1","zjetstat2","zjetstat3","zjetstat4","zjetstat5","zjetstat6","zjetstat7","zjetstat8","zjetstat9","zjetstat10","zjetstat11"]

systematicNames = ["gammajetdphi","gammajetgenerator","gammajetoutofcone","gammajetphesmaterial","gammajetphespresampler","gammajetphesz","gammajetpurity","gammajetstat1","gammajetstat2","gammajetstat3","gammajetstat4","gammajetstat5","gammajetstat6","gammajetstat7","gammajetstat8","gammajetstat9","gammajetsubleadingjet","zjetelecenergyzeeall","zjetelecenmat","zjetelecenpreshower","zjetfitmethod","zjetgenerator","zjetjvfcut","zjetoutofcone","zjetstat1","zjetstat10","zjetstat2","zjetstat3","zjetstat4","zjetstat5","zjetstat6","zjetstat7","zjetstat8","zjetstat9","zjetsubleadingjet"]



fileNames = []
histNames = []
for contName in containerNames:
  fileNames.append( contName+"/correction.txt" )
  histNames.append( contName+"_correction" )
  for sysName in systematicNames:
    fileNames.append( contName+"/SystError_"+sysName+".txt" )
    histNames.append( contName+"_SystError_"+sysName.replace("gammajet", "GJ_").replace("zjet", "ZJ_") )

#17->800 by bins of 1GeV

hists = []
for iFile, fileName in enumerate(fileNames):
  hists.append( TH1F( histNames[iFile], histNames[iFile], 800-17, 17.0, 800.0) ) #first bin 1, last bin 783

  f = open( fileName, 'r')
  for line in f:
    line = line.rstrip()
    line = line.split("  ")
    thisBin = float(line[0])+0.5
    thisValue = float(line[2])
    #print line, thisBin, thisValue
    hists[-1].SetBinContent( hists[-1].FindBin(thisBin) , thisValue )

outFile = TFile.Open("Vjet_Systematics.root", "RECREATE")
for thisHist in hists:
  thisHist.Write()
outFile.Close()
