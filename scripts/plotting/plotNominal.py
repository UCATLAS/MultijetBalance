#!/usr/bin/env python

###############################################################
# plotNominal.py                                              #
# A MJB second stage python script                            #
# Author Jeff Dandoy, UChicago                                #
#                                                             #
# This script takes any scaled, appened, or MJB file          #
# of histograms and plots the nominal values with systematic  #
# variations bands.  Systematic variations can be combined    #
# according to type.                                          #
#                                                             #
# Data MJB_initial files will look for similar MC files       #
# to plot on top.                                             #
# DoubleMJB_initial files will look for Gagik's results       #
# to plot on top.                                             #
#                                                             #
###############################################################

import os, glob
from array import array
import argparse
import time

from ROOT import *
import AtlasStyle

import plotSysRatios



def plotNominal(file, f_plotSys, f_addGagik):

  outDir = file[:-5]+'/'
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  outDir += "plotNominal/"
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  if not os.path.exists(outDir+'/eps'):
    os.mkdir(outDir+'/eps')
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



##################### Plot All Nominal With Systematic Bands  #################################

  print "Plotting nominal hists "

  if(f_plotSys):
    sysDirNameList = [dir for dir in dirList if not "Nominal" in dir]
    sysDirList = []
    for sysDirName in sysDirNameList:
      sysDirList.append( inFile.Get(sysDirName) )

    ## Combine systematics in types ##
    #sysTypes = ["MJB_a", "MJB_b", "MJB_ptt", "MJB_pta", "Flavor", "EtaIntercalibration"]
    sysTypes = ["ZJ", "GJ", "MJB", "Flavor", "EtaIntercalibration", "All"]
    if "All" in sysTypes:
      colorOffset = 240./(len(sysTypes)-1)
    else:
      colorOffset = 240./len(sysTypes)

  histList = [key.GetName() for key in nomDir.GetListOfKeys()]
  for histName in histList:
    if "prof_" in histName or "ptSlice" in histName:
      continue

    nomHist = nomDir.Get( histName )
    nomHist.SetName(nomHist.GetName())

    if not type(nomHist) == TH1F and not type(nomHist) == TH1D and not type(nomHist) == TGraphErrors:  #Can't draw bands if not 1D
      continue

    leg = TLegend(0.83, 0.15, 0.99, 0.95)
    pad1 = TPad("pad1", "", 0, 0, 0.83, 1)
    pad1.Draw()
    pad1.cd()
    leg.AddEntry( nomHist, "Nominal", "lp" )
    #nomHist.SetMinimum(0.9)
    if( "Pt" in histName or "Energy" in histName):
      nomHist.GetXaxis().SetRange(nomHist.FindFirstBinAbove(0), nomHist.FindLastBinAbove(0)+1 )
    nomHist.SetMaximum(1.5*nomHist.GetMaximum())
    nomHist.SetMinimum(0.0001)
    if( "MJB" in histName) :
      nomHist.GetXaxis().SetRangeUser( 300, 2500 )
      nomHist.SetMaximum(1.1)
      nomHist.SetMinimum(0.9)
      nomHist.GetXaxis().SetMoreLogLabels(True)
#    nomHist.SetMarkerSize(.75)
    if( "recoilPt" in histName):
      nomHist.GetYaxis().SetTitle("Entries / GeV")
      nomHist.GetXaxis().SetRangeUser( 500, 3100 )
      if( "recoilPt" in histName):
        for iBin in range(1, nomHist.GetNbinsX()+1):
          nomHist.SetBinContent(iBin, nomHist.GetBinContent(iBin)/ nomHist.GetBinWidth(iBin))
    nomHist.Draw()
    if not type(nomHist) == TGraphErrors:
      nomHist.Draw("p")
    else:
      nomHist.SetMarkerStyle(33)
      nomHist.SetMarkerSize(1.5)
      nomHist.SetLineWidth(4)
      nomHist.Draw("ap")

    if(f_plotSys):
      ## Get list of systematic histograms ##
      fullHistList = []
      for thisSysDir in sysDirList:
        fullHistList.append( thisSysDir.Get(histName) )
        fullHistList[-1].SetDirectory(0)

      ## Add systematic bands ##
      sysHistList = []
      for iTopSys, topSysName in enumerate(sysTypes):

        if topSysName == 'All':
          subHistList = [thisHist for thisName, thisHist in zip(sysDirNameList, fullHistList) if any(otherTopName in thisName for otherTopName in sysTypes) ]
        else:
          subHistList = [thisHist for thisName, thisHist in zip(sysDirNameList, fullHistList) if topSysName in thisName]

        sysHistUp, sysHistDn = plotSysRatios.getCombinedSysHist(nomHist, subHistList, topSysName)
        for iBin in range(1, nomHist.GetNbinsX()+1):
          sysHistUp.SetBinContent( iBin, nomHist.GetBinContent(iBin)*(1.+sysHistUp.GetBinContent(iBin)) )
          sysHistDn.SetBinContent( iBin, nomHist.GetBinContent(iBin)*(1.+sysHistDn.GetBinContent(iBin)) )

        if topSysName == 'All':
          color = kBlack
        else:
          color = gStyle.GetColorPalette(int(colorOffset*(iTopSys+1)))

        sysHistUp.SetLineColor(color)
        sysHistUp.SetMarkerColor(color)
        sysHistUp.SetMarkerSize(1.5)
        sysHistUp.SetMarkerStyle(34)
        sysHistDn.SetLineColor(color)
        sysHistDn.SetMarkerColor(color)
        sysHistDn.SetMarkerSize(1.5)
        sysHistDn.SetMarkerStyle(34)
        if( "Pt" in histName or "Energy" in histName):
          sysHistUp.GetXaxis().SetRange(sysHistUp.FindFirstBinAbove(0), sysHistUp.FindLastBinAbove(0)+1)
          sysHistDn.GetXaxis().SetRange(sysHistDn.FindFirstBinAbove(0), sysHistDn.FindLastBinAbove(0)+1)
        if not type(nomHist) == TGraphErrors:
          sysHistUp.Draw("same hist l")
          sysHistDn.Draw("same hist l")
        else:
          sysHistUp.SetMarkerStyle(4)
          sysHistUp.SetMarkerSize(1.2)
          sysHistUp.Draw("same l")
          sysHistDn.SetMarkerStyle(4)
          sysHistDn.SetMarkerSize(1.2)
          sysHistDn.Draw("same l")

        if( topSysName == "EtaIntercalibration"):
          leg.AddEntry( sysHistUp, "EIC", "lp")
        else:
          leg.AddEntry( sysHistUp, topSysName, "lp")

        # Save them in a list
        sysHistList.append( sysHistUp )
        sysHistList.append( sysHistDn )

#    ### Add MC for Data MJB! ###
#    if( "data" in file and "MJB" in histName ):
#      filePath = os.path.dirname( file )
#      if( "initial" in file):
#        mcFile = glob.glob( filePath+"/*mc14*MJB_initial.root")
#      elif( "final" in file):
#        mcFile = glob.glob( filePath+"/*mc14*MJB_final.root")
#      if len(mcFile) == 1:
#        mcFile = TFile.Open(mcFile[0], "READ")
#        mcKeyList = [key.GetName() for key in mcFile.GetListOfKeys()] #List of top level objects
#        mcDirList = [key for key in mcKeyList if "Iteration" in key] #List of all directories
#        #mcDir = [dir for dir in dirList if "NoCorr" in dir]
#        mcDir = [dir for dir in dirList if "Nominal" in dir]
#        if len(mcDir) == 1:
#          mcDir = mcFile.Get( mcDir[0] )
#          mcHist = mcDir.Get(histName)
#          mcHist.SetMarkerColor(kRed)
#          mcHist.SetLineColor(kRed)
#          mcHist.SetMarkerStyle(33)
#          mcHist.SetMarkerSize(1.3)
#          mcHist.Draw("same p")
#          leg.AddEntry(mcHist, "MC", "lp")
#
#    ### Add Gagik for DoubleMJB! ###
#    if( f_addGagik ):
#      if( "DoubleMJB" in histName ):
#        filePath = os.path.dirname( file )
#        gagikFile = TFile.Open(filePath+"/ThirdCycle.EM4.sherpa.1.v11.root", "READ")
#        gagikHist = gagikFile.Get("g_DoMC_fmean_vs_recoil")
#        gagikHist.SetMarkerColor(kRed)
#        gagikHist.SetLineColor(kRed)
#        gagikHist.SetMarkerStyle(33)
#        gagikHist.SetMarkerSize(1.3)
#        gagikHist.Draw("same p")
#        leg.AddEntry(gagikHist, "8 TeV", "lp")
#

    nomHist.Draw("same p")
    c1.cd()
    leg.Draw()
    AtlasStyle.ATLAS_LABEL(0.2,0.88, 1,"    Internal")
    AtlasStyle.myText(0.2,0.82,1, "#sqrt{s} = 13 TeV, 3.2 fb^{-1}")

    if "Pt" in histName or "alpha" in histName:
      pad1.SetLogy()
      c1.SaveAs(outDir+nomHist.GetName()+"_logy.png" )
      c1.SaveAs(outDir+'/eps/'+nomHist.GetName()+"_logy.eps" )
      pad1.SetLogy(0)
    if "MJB" in histName:
      pad1.SetLogx()
    c1.SaveAs((outDir+nomHist.GetName()+".png") )
    c1.SaveAs((outDir+'/eps/'+nomHist.GetName()+".eps") )
    c1.Clear()



  inFile.Close()

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--plotSys", dest='plotSys', action='store_true', default=False, help="Plot systematic bands around nominal")
  parser.add_argument("--addGagik", dest='addGagik', action='store_true', default=False, help="Plot previous 8TeV results")
  parser.add_argument("--file", dest='file', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  args = parser.parse_args()

  plotNominal(args.file, args.plotSys, args.addGagik)
