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



def plotNominal(file, f_plotSys, f_addGagik):

  outDir = file[:-5]+'/'
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  outDir += "plotNominal/"
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



##################### Plot All Nominal With Systematic Bands  #################################

  print "Plotting nominal hists "

  #sysDirNameList = [dir for dir in dirList if not "NoCorr" in dir]
  sysDirNameList = [dir for dir in dirList if not "Nominal" in dir]

  ## Get list of names without Iteration#_ prepended ##
  sysNameList = []
  for dirName in sysDirNameList:
    dirName = '_'.join(dirName.split('_')[1:])
    sysNameList.append( dirName )

  ## Combine systematics in types ##
  sysTypes = ["MJB_a", "MJB_b", "MJB_ptt", "MJB_pta", "Flavor", "EtaIntercalibration"]
  #sysTypes = ["ZJ", "GJ", "MJB", "Flavor", "EtaIntercalibration"]
  colorOffset = 250./ len(sysTypes) #Set range for setting color

  subSysList = []
  sysDirList = []
  for topSysName in sysTypes:
    subSysList.append([])
    sysDirList.append([])
    for iSys, sysName in enumerate(sysNameList):
      if topSysName in sysName:
        subSysList[-1].append(sysName)
        sysDirList[-1].append( inFile.Get( sysDirNameList[iSys] ) )


  for iSys, sysName in enumerate(sysTypes):
    print sysName, "includes",
    for thisSys in subSysList[iSys]:
      print thisSys,
    print ""

  histList = [key.GetName() for key in nomDir.GetListOfKeys()]
  for histName in histList:
    if "prof_" in histName or "ptSlice" in histName:
      continue

    nomHist = nomDir.Get( histName )
    nomHist.SetName(nomHist.GetName())

    if not type(nomHist) == TH1F and not type(nomHist) == TH1D and not type(nomHist) == TGraphErrors:  #Can't draw bands if not 1D
      continue

    leg = TLegend(0.75, 0.15, 0.99, 0.95)
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
      nomHist.GetXaxis().SetRangeUser( 300, 2000 )
      nomHist.SetMaximum(1.1)
      nomHist.SetMinimum(0.9)
      nomHist.GetXaxis().SetMoreLogLabels(True)
    nomHist.SetMarkerSize(.75)
    nomHist.Draw()
    if not type(nomHist) == TGraphErrors:
      nomHist.Draw("p")
    else:
      nomHist.SetMarkerStyle(33)
      nomHist.SetMarkerSize(1.5)
      nomHist.SetLineWidth(4)
      nomHist.Draw("ap")

    if(f_plotSys):
      ## Add systematic bands ##
      sysHists = []
      for iTopSys, topSysName in enumerate(sysTypes):

        for iSubSys, subSysName in enumerate( subSysList[iTopSys] ):
          sysHists.append( sysDirList[iTopSys][iSubSys].Get(histName) )
          sysHists[-1].SetName( sysHists[-1].GetName()+str(iTopSys)+"_"+str(iSubSys) )
          sysHists[-1].SetLineColor(gStyle.GetColorPalette( int(colorOffset*(iTopSys+1))) )
          sysHists[-1].SetMarkerColor(gStyle.GetColorPalette( int(colorOffset*(iTopSys+1))) )
          if( iSubSys == 0):
            sysHists[-1].SetMarkerStyle(26)
          elif( iSubSys == 1):
            sysHists[-1].SetMarkerStyle(32)
          else:
            sysHists[-1].SetMarkerStyle(34)
          sysHists[-1].SetMarkerSize(1.5)
          if( "Pt" in histName or "Energy" in histName):
            sysHists[-1].GetXaxis().SetRange(sysHists[-1].FindFirstBinAbove(0), sysHists[-1].FindLastBinAbove(0)+1)
          if not type(nomHist) == TGraphErrors:
            sysHists[-1].Draw("same hist l")
          else:
            sysHists[-1].SetMarkerStyle(4)
            sysHists[-1].SetMarkerSize(1.2)
            sysHists[-1].Draw("same l")
        leg.AddEntry( sysHists[-1], topSysName, "l")

    ### Add MC for Data MJB! ###
    if( "data" in file and "MJB" in histName ):
      filePath = os.path.dirname( file )
      if( "initial" in file):
        mcFile = glob.glob( filePath+"/*mc14*MJB_initial.root")
      elif( "final" in file):
        mcFile = glob.glob( filePath+"/*mc14*MJB_final.root")
      if len(mcFile) == 1:
        mcFile = TFile.Open(mcFile[0], "READ")
        mcKeyList = [key.GetName() for key in mcFile.GetListOfKeys()] #List of top level objects
        mcDirList = [key for key in mcKeyList if "Iteration" in key] #List of all directories
        #mcDir = [dir for dir in dirList if "NoCorr" in dir]
        mcDir = [dir for dir in dirList if "Nominal" in dir]
        if len(mcDir) == 1:
          mcDir = mcFile.Get( mcDir[0] )
          mcHist = mcDir.Get(histName)
          mcHist.SetMarkerColor(kRed)
          mcHist.SetLineColor(kRed)
          mcHist.SetMarkerStyle(33)
          mcHist.SetMarkerSize(1.3)
          mcHist.Draw("same p")
          leg.AddEntry(mcHist, "MC", "lp")

    ### Add Gagik for DoubleMJB! ###
    if( f_addGagik ):
      if( "DoubleMJB" in histName ):
        filePath = os.path.dirname( file )
        gagikFile = TFile.Open(filePath+"/ThirdCycle.EM4.sherpa.1.v11.root", "READ")
        gagikHist = gagikFile.Get("g_DoMC_fmean_vs_recoil")
        gagikHist.SetMarkerColor(kRed)
        gagikHist.SetLineColor(kRed)
        gagikHist.SetMarkerStyle(33)
        gagikHist.SetMarkerSize(1.3)
        gagikHist.Draw("same p")
        leg.AddEntry(gagikHist, "8 TeV", "lp")


    nomHist.Draw("same p")
    c1.cd()
    leg.Draw()
    if "Pt" in histName or "alpha" in histName:
      pad1.SetLogy()
      c1.SaveAs(outDir+nomHist.GetName()+"_logy.png" )
      pad1.SetLogy(0)
    if "MJB" in histName:
      pad1.SetLogx()
    c1.SaveAs((outDir+nomHist.GetName()+".png") )
    c1.Clear()



  inFile.Close()

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--file", dest='file', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  parser.add_argument("--plotSys", dest='plotSys', action='store_true', default=False, help="Plot systematic bands around nominal")
  parser.add_argument("--addGagik", dest='addGagik', action='store_true', default=False, help="Plot previous 8TeV results")
  args = parser.parse_args()

  plotNominal(args.file, args.plotSys, args.addGagik)
