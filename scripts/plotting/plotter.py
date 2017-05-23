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

import ROOT, AtlasStyle


def plotHists( sample_list ):
#(files, normalize, ratio, isValidation, is2016):
  ratio = True
  isValidation = True
  is2016 = True
  normalize = True
  AtlasStyle.SetAtlasStyle()
  ROOT.gROOT.ProcessLine("gErrorIgnoreLevel = 2000") #Ignore TCanvas::Print info
  ROOT.gROOT.SetBatch(True)

  outDir = os.path.dirname(sample_list[0].histFile)+'/../plots/'
  if not os.path.exists(outDir+'eps'):
    os.makedirs(outDir+'eps')

  files = [sample.histFile for sample in sample_list]    

  inFiles = []
  nomDirs = []
  for file in files:

    inFiles.append( ROOT.TFile.Open(file, "READ") )
    keyList = [key.GetName() for key in inFiles[-1].GetListOfKeys()] #List of top level objects
    dirList = [key for key in keyList] #List of all directories

    nomDir = [dir for dir in dirList if "Nominal" in dir]
    if( not len(nomDir) == 1):
      print "Error, nominal directories are ", nomDir
      return
    else:
      nomDirs.append( inFiles[-1].Get( nomDir[0] ) )

  c1 = ROOT.TCanvas()


  ################ Set Color Palate ####################3
# Data, Herwig, Pythia8, Sherpa
  colors = [ROOT.kBlack, ROOT.kRed, ROOT.kBlue, ROOT.kViolet, ROOT.kGreen, ROOT.kCyan]
  markers = [20, 21, 23, 22, 33, 34]
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
  oneLine = ROOT.TF1("zl1","1", -10000, 10000)
  oneLine.SetTitle("")
  oneLine.SetLineWidth(1)
  oneLine.SetLineStyle(2)
  oneLine.SetLineColor(ROOT.kBlack)


  histList = [key.GetName() for key in nomDirs[0].GetListOfKeys()]
  for histName in histList:
    if "prof_" in histName or "ptSlice" in histName:
      continue

    tmpHist = nomDirs[0].Get( histName )
    if not type(tmpHist) == ROOT.TH1F and not type(tmpHist) == ROOT.TH1D and not type(tmpHist) == ROOT.TGraphErrors:  #Can't draw bands if not 1D
      continue

    leg = ROOT.TLegend(0.65, 0.72, 0.9, 0.93)
    leg.SetFillStyle(0)
    leg.SetTextFont(42)
#!    leg = TLegend(0.83, 0.15, 0.99, 0.95)
    if ratio:
      pad1 = ROOT.TPad("pad1", "", 0, 0.32, 1, 1)
      pad2 = ROOT.TPad("pad2", "", 0, 0, 1, 0.32)
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

      nomHist = nomDir.Get( histName )
      nomHist.SetName(nomHist.GetName())
      if "Beta" in histName:
        nomHist.Rebin(2)
      if "alpha" in histName:
        nomHist.Rebin(4)
      if "ptAsym" in histName:
        nomHist.Rebin(4)

      if "Eta" in histName and not type(nomHist) == ROOT.TGraphErrors:
        nomHist.Rebin(4)

      if( "recoilPt" in histName):
        for iBin in range(1, nomHist.GetNbinsX()+1):
          nomHist.SetBinContent(iBin, nomHist.GetBinContent(iBin)/ nomHist.GetBinWidth(iBin))

      if normalize and not type(nomHist) == ROOT.TGraphErrors and nomHist.Integral() > 0.:
        nomHist.Scale( 1./nomHist.Integral() )


#      if "Sherpa" in files[iDir] and "MJB" in histName:
#        for iBin in range(31, nomHist.GetNbinsX()+1):
#          nomHist.SetBinContent(iBin, 0)
#          nomHist.SetBinError(iBin, 0)
      thisName = sample_list[iDir].tag

      nomHist.SetLineColor(colors[iDir])
      nomHist.SetMarkerColor(colors[iDir])
      nomHist.SetMarkerStyle(markers[iDir])
      thisEntry = leg.AddEntry( nomHist, thisName.replace('Sherpa', 'Sherpa 2.1').replace('Herwig','Herwig++'), "lp" )
      #thisEntry = leg.AddEntry( nomHist, thisFileStr.replace('d','D').replace('Sherpa', 'Sherpa 2.1').replace('Herwig','Herwig++'), "lp" )
      thisEntry.SetTextColor( colors[iDir] )
      #nomHist.SetMinimum(0.9)
#      if( "jetPt" in histName or "jetEnergy" in histName):
#        maxBinX.append(nomHist.FindLastBinAbove(0)+1)
#        nomHist.SetMaximum(1.5*nomHist.GetMaximum())
#        nomHist.SetMinimum(0.000101)
#      else:
      nomHist.SetMaximum(1.5*nomHist.GetMaximum())
      #nomHist.SetMinimum(0.000101)
      if( "MJB" in histName) :
        nomHist.SetMinimum(0.0000101)
        #nomHist.GetXaxis().SetRangeUser( 500, 2800 ) #!!public
        #nomHist.SetMaximum(1.06) #!!public
        #nomHist.SetMinimum(0.9701) #!!public
#        nomHist.GetXaxis().SetRangeUser( 200, endPt )
        if(isValidation):
          nomHist.SetMaximum(1.1)
          #nomHist.SetMinimum(0.93001)
          nomHist.SetMinimum(0.95001)
        else:
          nomHist.SetMaximum(1.2)
          nomHist.SetMinimum(0.90001)
        
        nomHist.GetXaxis().SetMoreLogLabels(True)
        nomHist.GetYaxis().SetTitle("#LT p_{T}^{lead jet}/p_{T}^{recoil} #GT")
        nomHist.GetYaxis().SetTitleSize(0.09)
        nomHist.GetYaxis().SetTitleOffset(0.7)
        nomHist.GetYaxis().SetLabelSize(0.06)
        nomHist.GetYaxis().SetLabelOffset(0.01)
        nomHist.SetMarkerSize(.8)
        nomHist.SetLineWidth(1)
      elif( "Pt" in histName) :
        nomHist.GetYaxis().SetTitle("AU")
#        if( "jet0" in histName):
#          nomHist.GetXaxis().SetRangeUser( 200, endJetPt )
#        else:
#          nomHist.GetXaxis().SetRangeUser( 0, endJetPt-500 )
      else:
        nomHist.GetYaxis().SetTitle("AU")
      if( "recoilPt" in histName):
        nomHist.GetYaxis().SetTitle("1/N dp_{T}^{recoil}/dN")
#        nomHist.GetXaxis().SetRangeUser( 200, endPt )
      if not type(nomHist) == ROOT.TGraphErrors:
        #drawString = "histsamep"
        drawString = "psame e"
      else:
        drawString = "e0 apsame"
        nomHist.SetMarkerStyle(33)
        nomHist.SetMarkerSize(1.5)
        nomHist.SetLineWidth(4)

      nomHists.append(nomHist)
      maxVal.append( nomHist.GetMaximum() )

    maxDir = maxVal.index( max(maxVal) )
    if( not "MJB" in histName) :
      nomHists[maxDir]
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
          ratioHists[iDir-1].SetMaximum(2)
          ratioHists[iDir-1].SetMinimum(0.5)
          ratioHists[iDir-1].GetXaxis().SetLabelOffset(.015)
          ratioHists[iDir-1].GetXaxis().SetTitleOffset(1.3)
          ratioHists[iDir-1].GetXaxis().SetLabelSize(0.13)
          ratioHists[iDir-1].GetXaxis().SetTitleSize(0.16)
#          ratioHists[iDir-1].GetXaxis().SetTitle(nomHists[0].GetXaxis().GetTitle());
          ratioHists[iDir-1].GetXaxis().SetMoreLogLabels()
          ratioHists[iDir-1].GetYaxis().SetLabelSize(0.13)
          ratioHists[iDir-1].GetYaxis().SetTitleSize(0.16)
          ratioHists[iDir-1].GetYaxis().SetLabelOffset(.01)
          ratioHists[iDir-1].GetYaxis().SetTitleOffset(0.37)
          #ratioHists[iDir-1].GetYaxis().SetTitle("Significance")
          ratioHists[iDir-1].GetYaxis().SetTitle("   MC / Data")
          ratioHists[iDir-1].GetYaxis().SetNdivisions(7)
#          if( "Pt" in histName) :
#            if( "jet0" in histName):
#              ratioHists[iDir-1].GetXaxis().SetRangeUser( 200, endJetPt )
#            else:
#              ratioHists[iDir-1].GetXaxis().SetRangeUser( 0, endJetPt-500 )
          if( "MJB" in histName) :
#            ratioHists[iDir-1].GetXaxis().SetRangeUser( 200, endPt )
            if( isValidation):
              #ratioHists[iDir-1].SetMaximum(1.03)
              #ratioHists[iDir-1].SetMinimum(0.97)
              ratioHists[iDir-1].SetMaximum(1.02)
              ratioHists[iDir-1].SetMinimum(0.98)
            else:
              ratioHists[iDir-1].SetMaximum(1.05)
              ratioHists[iDir-1].SetMinimum(0.95)
            
            ratioHists[iDir-1].GetXaxis().SetTitle("p_{T}^{recoil} [GeV]");
          if( "recoilPt" in histName):
#            ratioHists[iDir-1].GetXaxis().SetRangeUser( 200, endPt )
            ratioHists[iDir-1].GetXaxis().SetTitle("p_{T}^{recoil} [GeV]");

     #     if( "jetBeta" in histName):
     #       ratioHists[iDir-1].SetMaximum(1)
     #       ratioHists[iDir-1].SetMinimum(-1)

      ratioHists[0].Draw( "e0 p" )
      oneLine.Draw("same")
      for iDir in range(1,len(nomDirs)):
        ratioHists[iDir-1].Draw( "e0 psame" )


    c1.cd()
    leg.Draw()
    if( "MJB" in histName) :
      AtlasStyle.ATLAS_LABEL(0.2,0.88, 1,"    Preliminary")
      if (is2016):
        AtlasStyle.myText(0.2,0.82,1, "#sqrt{s} = 13 TeV, 33 fb^{-1}")
      else:
        AtlasStyle.myText(0.2,0.82,1, "#sqrt{s} = 13 TeV, 33 fb^{-1}")
      if (isValidation):
        #AtlasStyle.myText(0.2, 0.76, 1, "Dijet Events, Validation")
        AtlasStyle.myText(0.2, 0.76, 1, "Multijet Events, Validation")
      else:
        AtlasStyle.myText(0.2, 0.76, 1, "Multijet Events")
      typeText = "anti-k_{t} R = 0.4"
      if "LC" in dataFile:
        typeText += ", LC+JES (in-situ)"
      else:
        typeText += ", EM+JES (in-situ)"
      AtlasStyle.myText(0.2,0.7,1, typeText)
      AtlasStyle.myText(0.2,0.64,1, "#left|#eta^{lead jet}#right| < 1.2")
    else:
      AtlasStyle.ATLAS_LABEL(0.35,0.88, 1,"    Preliminary")
      if (is2016):
        AtlasStyle.myText(0.35,0.82,1, "#sqrt{s} = 13 TeV, 33 fb^{-1}")
      else:
        AtlasStyle.myText(0.35,0.82,1, "#sqrt{s} = 13 TeV, 33 fb^{-1}")

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

