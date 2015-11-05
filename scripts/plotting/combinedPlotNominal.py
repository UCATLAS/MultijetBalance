#!/usr/bin/env python

###############################################################
# combinedPlotNominal.py                                      #
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


def combinedPlotNominal(files, normalize, ratio):

  files = files.split(',')
  # Save output to directory of first file
  outDir = files[0][:-5]+'/'
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  if normalize:
    outDir += "combinedPlotNominal_normalized/"
  else:
    outDir += "combinedPlotNominal/"
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  if not os.path.exists(outDir+'/eps'):
    os.mkdir(outDir+'/eps')
  AtlasStyle.SetAtlasStyle()

  gROOT.ProcessLine("gErrorIgnoreLevel = 2000") #Ignore TCanvas::Print info

  inFiles = []
  nomDirs = []
  for file in files:

    inFiles.append( TFile.Open(file, "READ") )
    keyList = [key.GetName() for key in inFiles[-1].GetListOfKeys()] #List of top level objects
    dirList = [key for key in keyList if "Iteration" in key] #List of all directories

    nomDir = [dir for dir in dirList if "Nominal" in dir]
    if( not len(nomDir) == 1):
      print "Error, nominal directories are ", nomDir
      return
    else:
      nomDirs.append( inFiles[-1].Get( nomDir[0] ) )

  c1 = TCanvas()


  ################ Set Color Palate ####################3
  colorMax = 240.
  colorMin = 0. #20.
  numInputs = len( nomDirs )
  colors = []
  if len(nomDirs) == 2:
    colors = [kBlack, kRed]
  else:
    for iDir, nomDir in enumerate(nomDirs):
      colorNum = int( colorMin+(colorMax-colorMin)*iDir/numInputs)
      colors.append( gStyle.GetColorPalette( colorNum ))

##################### Plot All Nominal #################################

  print "Plotting nominal hists "


  histList = [key.GetName() for key in nomDirs[0].GetListOfKeys()]
  for histName in histList:
    if "prof_" in histName or "ptSlice" in histName:
      continue

    tmpHist = nomDirs[0].Get( histName )
    if not type(tmpHist) == TH1F and not type(tmpHist) == TH1D and not type(tmpHist) == TGraphErrors:  #Can't draw bands if not 1D
      continue

    leg = TLegend(0.83, 0.15, 0.99, 0.95)
    if ratio:
      pad1 = TPad("pad1", "", 0, 0.3, 0.83, 1)
      pad2 = TPad("pad2", "", 0, 0, 0.83, 0.3)
      pad1.SetBottomMargin(0)
      pad2.SetTopMargin(0)
      pad2.SetBottomMargin(0.3)
      pad1.Draw()
      pad2.Draw()
    else:
      pad1 = TPad("pad1", "", 0, 0, 0.83, 1)
      pad1.Draw()
    pad1.cd()

    nomHists = []
    drawString = ""
    maxVal = []
    maxBinX = []

    for iDir, nomDir in enumerate(nomDirs):
      thisFileStr = files[iDir].split('.')[2]
      if "all" in thisFileStr:
        thisFileStr = files[iDir].split('.')[1]

      nomHist = nomDir.Get( histName )
      nomHist.SetName(nomHist.GetName())
      if "Beta" in histName:
        nomHist.Rebin(2)
      if "alpha" in histName:
        nomHist.Rebin(4)
      if "ptAsym" in histName:
        nomHist.Rebin(4)

      if "Eta" in histName and not type(nomHist) == TGraphErrors:
        nomHist.Rebin(4)
      if normalize and not type(nomHist) == TGraphErrors and nomHist.Integral() > 0.:
        nomHist.Scale( 1./nomHist.Integral() )


      nomHist.SetLineColor(colors[iDir])
      nomHist.SetMarkerColor(colors[iDir])
      leg.AddEntry( nomHist, thisFileStr, "lp" )
      #nomHist.SetMinimum(0.9)
#      if( "jetPt" in histName or "jetEnergy" in histName):
#        maxBinX.append(nomHist.FindLastBinAbove(0)+1)
#        nomHist.SetMaximum(1.5*nomHist.GetMaximum())
#        nomHist.SetMinimum(0.000101)
#      else:
      nomHist.SetMaximum(1.5*nomHist.GetMaximum())
      nomHist.SetMinimum(0.0000101)
      #nomHist.SetMinimum(0.000101)
      if( "MJB" in histName) :
        nomHist.GetXaxis().SetRangeUser( 400, 3500 )
        nomHist.SetMaximum(1.05)
        nomHist.SetMinimum(0.90)
        nomHist.GetXaxis().SetMoreLogLabels(True)
#      nomHist.SetMarkerSize(.75)
      elif( "Pt" in histName) :
        nomHist.GetXaxis().SetRangeUser( 200, 3500 )
      else:
        nomHist.GetYaxis().SetTitle("AU")
      if not type(nomHist) == TGraphErrors:
        drawString = "psame"
      else:
        drawString = "apsame"
        nomHist.SetMarkerStyle(33)
        nomHist.SetMarkerSize(1.5)
        nomHist.SetLineWidth(4)

      nomHists.append(nomHist)
      maxVal.append( nomHist.GetMaximum() )

    maxDir = maxVal.index( max(maxVal) )
    nomHists[maxDir].Draw( drawString )
    if maxBinX:
      maxBinX = max( maxBinX )
      for iDir, nomDir in enumerate(nomDirs):
        nomHists[iDir].GetXaxis().SetRange(1, maxBinX )
    for iDir, nomDir in enumerate(nomDirs):
      nomHists[iDir].Draw( drawString )


    if ratio:
      pad2.cd()
      ratioHists = []

#      if maxBinX:
#        oneLine = TF1("zl1","0", 1, maxBinX)
#      else:
      oneLine = TF1("zl1","0", nomHists[0].GetXaxis().GetBinLowEdge(1), nomHists[0].GetXaxis().GetBinUpEdge(nomHists[0].GetNbinsX()) )
      oneLine.SetTitle("")
      oneLine.SetLineWidth(1)
      oneLine.SetLineStyle(7)
      oneLine.SetLineColor(kBlack)


      for iDir in range(1,len(nomDirs)):
        ratioHists.append( nomHists[0].Clone() )
        ratioHists[iDir-1].Add(nomHists[iDir], -1.)
        ratioHists[iDir-1].Divide(nomHists[iDir])
        if iDir == 1:
          ratioHists[iDir-1].GetXaxis().SetLabelOffset(.015)
          ratioHists[iDir-1].GetXaxis().SetLabelSize(0.11)
          ratioHists[iDir-1].GetXaxis().SetTitleSize(0.12)
          ratioHists[iDir-1].GetXaxis().SetTitle(nomHists[0].GetXaxis().GetTitle());
          ratioHists[iDir-1].GetXaxis().SetMoreLogLabels()
          ratioHists[iDir-1].GetYaxis().SetLabelSize(0.11)
          ratioHists[iDir-1].GetYaxis().SetTitleSize(0.12)
          ratioHists[iDir-1].GetYaxis().SetLabelOffset(.01)
          ratioHists[iDir-1].GetYaxis().SetTitleOffset(0.5)
          ratioHists[iDir-1].GetYaxis().SetTitle("Significance")
          ratioHists[iDir-1].GetYaxis().SetNdivisions(7)
          if( "Pt" in histName) :
            ratioHists[iDir-1].GetXaxis().SetRangeUser( 200, 3500 )
          if( "MJB" in histName) :
            ratioHists[iDir-1].GetXaxis().SetRangeUser( 400, 3500 )
            ratioHists[iDir-1].SetMaximum(0.05)
            ratioHists[iDir-1].SetMinimum(-0.05)
          ratioHists[iDir-1].Draw( "p" )
        else:
          ratioHists[iDir-1].Draw( "psame" )
      oneLine.Draw("same")


    c1.cd()
    leg.Draw()
    if "MJB" in histName:
      pad1.SetLogx()
      if ratio:
        pad2.SetLogx()
    else:
      pad1.SetLogx(0)
      if ratio:
        pad2.SetLogx(0)
    #if "Pt" in histName or "alpha" in histName:
    pad1.SetLogy()
    c1.SaveAs(outDir+nomHist.GetName()+"_logy.png" )
    c1.SaveAs(outDir+"/eps/"+nomHist.GetName()+"_logy.eps" )
    pad1.SetLogy(0)
    c1.SaveAs((outDir+nomHist.GetName()+".png") )
    c1.SaveAs((outDir+"/eps/"+nomHist.GetName()+".eps") )
    c1.Clear()


  for inFile in inFiles:
    inFile.Close()

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--files", dest='files', default="",
           help="Comma seperated list of files to use")
  parser.add_argument("--normalize", dest='normalize', action='store_true', default=False, help="Normalize all histograms to unit area")
  parser.add_argument("--ratio", dest='ratio', action='store_true', default=False, help="Draw ratio plots w.r.t. first plot")
  args = parser.parse_args()

  combinedPlotNominal(args.files, args.normalize, args.ratio)
