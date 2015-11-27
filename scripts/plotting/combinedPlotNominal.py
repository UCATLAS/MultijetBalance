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
  dataFile = [file for file in files if ".data." in file][0]
  files.remove(dataFile)
  files.insert(0, dataFile)
  pythiaFile = [file for file in files if "Pythia" in file][0]
  files.remove(pythiaFile)
  files.insert(1, pythiaFile)
  sherpaFile = [file for file in files if "Sherpa" in file][0]
  files.remove(sherpaFile)
  files.insert(2, sherpaFile)
  # Save output to directory of first file
  outDir = dataFile[:-5]+'/'
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
# Data, Herwig, Pythia8, Sherpa
  colors = [kBlack, kViolet, kBlue, kRed, kCyan]
  markers = [20, 22, 23, 21, 33, 34]
#  colorMax = 240.
#  colorMin = 0. #20.
#  numInputs = len( nomDirs )
#  colors = []
#  if len(nomDirs) == 2:
#    colors = [kBlack, kRed]
#  else:
#    for iDir, nomDir in enumerate(nomDirs):
#      colorNum = int( colorMin+(colorMax-colorMin)*iDir/numInputs)
#      colors.append( gStyle.GetColorPalette( colorNum ))

##################### Plot All Nominal #################################

  print "Plotting nominal hists "
  oneLine = TF1("zl1","1", -10000, 10000)
  oneLine.SetTitle("")
  oneLine.SetLineWidth(1)
  oneLine.SetLineStyle(2)
  oneLine.SetLineColor(kBlack)


  histList = [key.GetName() for key in nomDirs[0].GetListOfKeys()]
  for histName in histList:
    if "prof_" in histName or "ptSlice" in histName:
      continue

    tmpHist = nomDirs[0].Get( histName )
    if not type(tmpHist) == TH1F and not type(tmpHist) == TH1D and not type(tmpHist) == TGraphErrors:  #Can't draw bands if not 1D
      continue

    leg = TLegend(0.65, 0.72, 0.9, 0.93)
    leg.SetFillStyle(0)
    leg.SetTextFont(42)
#!    leg = TLegend(0.83, 0.15, 0.99, 0.95)
    if ratio:
      pad1 = TPad("pad1", "", 0, 0.32, 1, 1)
      pad2 = TPad("pad2", "", 0, 0, 1, 0.32)
#!      pad1 = TPad("pad1", "", 0, 0.3, 0.83, 1)
#!      pad2 = TPad("pad2", "", 0, 0, 0.83, 0.3)
      pad1.SetBottomMargin(0.01)
      pad2.SetTopMargin(0)
      pad2.SetBottomMargin(0.45)
      pad1.Draw()
      pad2.Draw()
    else:
#!      pad1 = TPad("pad1", "", 0, 0, 0.83, 1)
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

      if( "recoilPt" in histName):
        for iBin in range(1, nomHist.GetNbinsX()+1):
          nomHist.SetBinContent(iBin, nomHist.GetBinContent(iBin)/ nomHist.GetBinWidth(iBin))

      if normalize and not type(nomHist) == TGraphErrors and nomHist.Integral() > 0.:
        nomHist.Scale( 1./nomHist.Integral() )


      print files[iDir]
      if "Sherpa" in files[iDir] and "MJB" in histName:
        for iBin in range(31, nomHist.GetNbinsX()+1):
          nomHist.SetBinContent(iBin, 0)
          nomHist.SetBinError(iBin, 0)

      nomHist.SetLineColor(colors[iDir])
      nomHist.SetMarkerColor(colors[iDir])
      nomHist.SetMarkerStyle(markers[iDir])
      leg.AddEntry( nomHist, thisFileStr.replace('d','D'), "lp" )
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
        nomHist.GetXaxis().SetRangeUser( 500, 2800 )
        nomHist.SetMaximum(1.06)
        nomHist.SetMinimum(0.9701)
        nomHist.GetXaxis().SetMoreLogLabels(True)
        nomHist.GetYaxis().SetTitle("<p_{T}^{lead jet}/p_{T}^{recoil}>")
        nomHist.GetYaxis().SetTitleSize(0.09)
        nomHist.GetYaxis().SetTitleOffset(0.7)
        nomHist.GetYaxis().SetLabelSize(0.06)
        nomHist.GetYaxis().SetLabelOffset(0.01)
        nomHist.SetMarkerSize(.7)
      elif( "Pt" in histName) :
        nomHist.GetXaxis().SetRangeUser( 200, 3500 )
        nomHist.GetYaxis().SetTitle()
      else:
        nomHist.GetYaxis().SetTitle("AU")
      if( "recoilPt" in histName):
        nomHist.GetYaxis().SetTitle("1/N dp_{T}^{recoil}/dN")
        nomHist.GetXaxis().SetRangeUser( 500, 3200 )
      if not type(nomHist) == TGraphErrors:
        #drawString = "histsamep"
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
    oneLine.Draw("same")
    for iDir, nomDir in enumerate(nomDirs):
      nomHists[iDir].Draw( drawString )
    nomHists[0].Draw( drawString ) ## Draw data on top


    if ratio:
      pad2.cd()
      ratioHists = []



      for iDir in range(1,len(nomDirs)):
        #ratioHists.append( nomHists[0].Clone() )
        #ratioHists[iDir-1].Add(nomHists[iDir], -1.)
        #ratioHists[iDir-1].Divide(nomHists[iDir])
        ratioHists.append( nomHists[iDir].Clone() )
        ratioHists[iDir-1].Divide(nomHists[0])

        ratioHists[iDir-1].SetMarkerColor( colors[iDir] )
        ratioHists[iDir-1].SetMarkerStyle( markers[iDir] )
        ratioHists[iDir-1].SetLineColor( colors[iDir] )
        if iDir == 1:
          ratioHists[iDir-1].GetXaxis().SetLabelOffset(.015)
          ratioHists[iDir-1].GetXaxis().SetTitleOffset(1.3)
          ratioHists[iDir-1].GetXaxis().SetLabelSize(0.13)
          ratioHists[iDir-1].GetXaxis().SetTitleSize(0.16)
          ratioHists[iDir-1].GetXaxis().SetTitle("p_{T}^{recoil} [GeV]");
#          ratioHists[iDir-1].GetXaxis().SetTitle(nomHists[0].GetXaxis().GetTitle());
          ratioHists[iDir-1].GetXaxis().SetMoreLogLabels()
          ratioHists[iDir-1].GetYaxis().SetLabelSize(0.13)
          ratioHists[iDir-1].GetYaxis().SetTitleSize(0.16)
          ratioHists[iDir-1].GetYaxis().SetLabelOffset(.01)
          ratioHists[iDir-1].GetYaxis().SetTitleOffset(0.37)
          #ratioHists[iDir-1].GetYaxis().SetTitle("Significance")
          ratioHists[iDir-1].GetYaxis().SetTitle("   MC / Data")
          ratioHists[iDir-1].GetYaxis().SetNdivisions(7)
          if( "Pt" in histName) :
            ratioHists[iDir-1].GetXaxis().SetRangeUser( 200, 3500 )
          if( "MJB" in histName) :
            ratioHists[iDir-1].GetXaxis().SetRangeUser( 500, 2800 )
            ratioHists[iDir-1].SetMaximum(1.05)
            ratioHists[iDir-1].SetMinimum(0.95)
          if( "recoilPt" in histName):
            ratioHists[iDir-1].GetXaxis().SetRangeUser( 500, 3200 )

          if( "jetBeta" in histName):
            ratioHists[iDir-1].SetMaximum(1)
            ratioHists[iDir-1].SetMinimum(-1)

      ratioHists[0].Draw( "p" )
      oneLine.Draw("same")
      for iDir in range(1,len(nomDirs)):
        ratioHists[iDir-1].Draw( "psame" )


    c1.cd()
    leg.Draw()
    AtlasStyle.ATLAS_LABEL(0.2,0.88, 1,"    Internal")
    AtlasStyle.myText(0.2,0.82,1, "#sqrt{s} = 13 TeV, 3.3 fb^{-1}")
    typeText = "anti-k_{t} R = 0.4"
    if "_LC_" in dataFile:
      typeText += ", LC+JES (in-situ)"
    else:
      typeText += ", EM+JES (in-situ)"
    AtlasStyle.myText(0.2,0.76,1, typeText)
    AtlasStyle.myText(0.2,0.7,1, "#left|#eta^{lead jet}#right| < 1.2")
#    AtlasStyle.myText(0.1,0.75,1, "m_{jj} Correction")

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
    c1.SaveAs((outDir+"/eps/"+nomHist.GetName()+".pdf") )
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
