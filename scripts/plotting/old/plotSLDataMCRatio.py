#!/usr/bin/env python

###################################################################
# plotSLDataMCRatio.py                                            #
# A MJB second stage python script                                #
# Author Jeff Dandoy, UChicago                                    #
#                                                                 #
# This script takes a data and a MC appended file and plots the   #
# Ratio of all Sampling Layer Percentages on a single plot.       #
#                                                                 #
###################################################################


import os
import argparse

from ROOT import *
import AtlasStyle


def plotSLDataMCRatio(dataFile, mcFile, binning):
  if not "appended.root" in mcFile or not "appended.root" in dataFile:
    print "Error:  plotSL.py must run on two files ending in \"appended.root\", but was given", mcFile, "and", dataFile
    print "Exiting..."
    return

  if not binning[0] == '_':
    binning = '_'+binning

  outDir = dataFile[:-5]+'/'
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  outDir += "plotSLDataMCRatio/"
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  AtlasStyle.SetAtlasStyle()

  gROOT.ProcessLine("gErrorIgnoreLevel = 2000") #Ignore TCanvas::Print info

  mcFile = TFile.Open(mcFile, "READ");
  mcKeyList = [key.GetName() for key in mcFile.GetListOfKeys()] #List of top level objects
  #mcDir = [key for key in mcKeyList if "Iteration" in key and "NoCorr" in key] #List of all directories
  mcDir = [key for key in mcKeyList if "Iteration" in key and "Nominal" in key] #List of all directories
  mcDir = mcFile.Get( mcDir[0] )
  dataFile = TFile.Open(dataFile, "READ");
  dataKeyList = [key.GetName() for key in dataFile.GetListOfKeys()] #List of top level objects
  #dataDir = [key for key in dataKeyList if "Iteration" in key and "NoCorr" in key] #List of all directories
  dataDir = [key for key in dataKeyList if "Iteration" in key and "Nominal" in key] #List of all directories
  dataDir = dataFile.Get( dataDir[0] )

  c1 = TCanvas()
  numSamples = 24
  sampleName = ["PreSamplerB", "EMB1", "EMB2", "EMB3", "PreSamplerE", "EME1", "EME2", "HEC0", "HEC1", "HEC2", "HEC3", "TileBar0", "TileBar1", "TileBar2", "TileGap1", "TileGap2", "TileGap3", "TileExt0", "TileExt1", "TileExt2", "FCAL0", "FCAL1", "FCAL2", "MINIFCAL0", "MINIFCAL1", "MINIFCAL2", "MINIFCAL3"]

  sampleSubset = [0, 1, 2, 3, 11, 12, 13]

  #################### New Histograms ##########################
  print "Plotting sampling layer energy ratio "

  thisSample = []
  leg = TLegend(0.83, 0.15, 0.99, 0.95)
  pad1 = TPad("pad1", "", 0, 0, 0.83, 1)
  pad1.SetRightMargin(0.1)
  pad1.Draw()
  pad1.cd()

  ## Draw a ghost copy for it's axis first ##
  thisSample.append(mcDir.Get("prof_recoilPt_SamplingLayerPercent0"+binning) )
  #thisSample[-1].Add(thisSample[-1], -1.)
  thisSample[-1].GetYaxis().SetTitle( "Rel. Uncert. of Recoil Energy per Layer" )
  thisSample[-1].GetYaxis().SetRangeUser(-0.2,0.2)
  thisSample[-1].SetLineColor(kWhite)
  thisSample[-1].SetMarkerColor(kWhite)
  thisSample[-1].Draw("hist l")

  for iSample in range(0, numSamples):
    thisSample.append( mcDir.Get("prof_recoilPt_SamplingLayerPercent"+str(iSample)+binning) )
    thisSample[-1].SetName(thisSample[-1].GetName()+str(iSample))
    dataSample = dataDir.Get("prof_recoilPt_SamplingLayerPercent"+str(iSample)+binning)
    thisSample[-1].Add(dataSample, -1.)
    thisSample[-1].Divide(dataSample)
    thisSample[-1].GetYaxis().SetRangeUser(-0.2,0.2)


    #thisSample[-1].GetXaxis().SetRangeUser(200., 2000.)
    thisSample[-1].SetLineColor( gStyle.GetColorPalette(10*iSample+1) )
    thisSample[-1].SetMarkerColor( gStyle.GetColorPalette(10*iSample+1) )
    thisSample[-1].SetMarkerSize(0.5)
    if(iSample%3 == 0):
      thisSample[-1].SetLineStyle(1)
    elif(iSample%3==1):
      thisSample[-1].SetLineStyle(9)
      thisSample[-1].SetLineWidth(3)
    else:
      thisSample[-1].SetLineStyle(2)

    leg.AddEntry(thisSample[-1], sampleName[iSample], "l")
    thisSample[-1].Draw("same hist l")
  c1.cd()
  leg.Draw()
  c1.SaveAs(outDir+"Frac_SamplingEnergy_all"+binning+".png")
#  pad1.SetLogy()
#  c1.SaveAs(outDir+"SamplingEnergy_all"+binning+"_logy.png")
#  pad1.SetLogy(0)
  c1.Clear()

  ###### Do straight difference ##########
  thisSample = []
  leg = TLegend(0.83, 0.15, 0.99, 0.95)
  pad1 = TPad("pad1", "", 0, 0, 0.83, 1)
  pad1.Draw()
  pad1.cd()

  ## Draw a ghost copy for it's axis first ##
  thisSample.append(mcDir.Get("prof_recoilPt_SamplingLayerPercent0"+binning) )
  thisSample[-1].Add(thisSample[-1], -1.)
  thisSample[-1].GetYaxis().SetTitle( "Difference in Recoil Energy per Layer" )
  thisSample[-1].GetYaxis().SetRangeUser(-0.1,0.1)
  thisSample[-1].SetLineColor(kWhite)
  thisSample[-1].SetMarkerColor(kWhite)
  thisSample[-1].Draw("hist l")

  for iSample in range(0, numSamples):
    thisSample.append( mcDir.Get("prof_recoilPt_SamplingLayerPercent"+str(iSample)+binning) )
    thisSample[-1].SetName(thisSample[-1].GetName()+str(iSample))
    dataSample = dataDir.Get("prof_recoilPt_SamplingLayerPercent"+str(iSample)+binning)
    thisSample[-1].Add(dataSample, -1.)
    thisSample[-1].GetYaxis().SetRangeUser(-0.1,0.1)

    #thisSample[-1].GetXaxis().SetRangeUser(200., 2000.)
    thisSample[-1].SetLineColor( gStyle.GetColorPalette(10*iSample+1) )
    thisSample[-1].SetMarkerColor( gStyle.GetColorPalette(10*iSample+1) )
    thisSample[-1].SetMarkerSize(0.5)
    if(iSample%3 == 0):
      thisSample[-1].SetLineStyle(1)
    elif(iSample%3==1):
      thisSample[-1].SetLineStyle(9)
      thisSample[-1].SetLineWidth(3)
    else:
      thisSample[-1].SetLineStyle(2)

    leg.AddEntry(thisSample[-1], sampleName[iSample], "l")
    thisSample[-1].Draw("same hist l")
  c1.cd()
  leg.Draw()
  c1.SaveAs(outDir+"Diff_SamplingEnergy_all"+binning+".png")
  c1.Clear()


###################################### Subset of sample layers only ###################################################
  #################### New Histograms ##########################
  print "Plotting sampling layer energy ratio "

  thisSample = []
  leg = TLegend(0.83, 0.15, 0.99, 0.95)
  pad1 = TPad("pad1", "", 0, 0, 0.83, 1)
  pad1.SetRightMargin(0.1)
  pad1.Draw()
  pad1.cd()

  ## Draw a ghost copy for it's axis first ##
  thisSample.append(mcDir.Get("prof_recoilPt_SamplingLayerPercent0"+binning) )
  #thisSample[-1].Add(thisSample[-1], -1.)
  thisSample[-1].GetYaxis().SetTitle( "Rel. Uncert. of Recoil Energy per Layer" )
  thisSample[-1].GetYaxis().SetRangeUser(-0.2,0.2)
  thisSample[-1].SetLineColor(kWhite)
  thisSample[-1].SetMarkerColor(kWhite)
  thisSample[-1].Draw("hist l")

  for iSample in sampleSubset:
    thisSample.append( mcDir.Get("prof_recoilPt_SamplingLayerPercent"+str(iSample)+binning) )
    thisSample[-1].SetName(thisSample[-1].GetName()+str(iSample))
    dataSample = dataDir.Get("prof_recoilPt_SamplingLayerPercent"+str(iSample)+binning)
    thisSample[-1].Add(dataSample, -1.)
    thisSample[-1].Divide(dataSample)
    thisSample[-1].GetYaxis().SetRangeUser(-0.2,0.2)


    #thisSample[-1].GetXaxis().SetRangeUser(200., 2000.)
    thisSample[-1].SetLineColor( gStyle.GetColorPalette(10*iSample+1) )
    thisSample[-1].SetMarkerColor( gStyle.GetColorPalette(10*iSample+1) )
    thisSample[-1].SetMarkerSize(0.5)
    if(iSample%3 == 0):
      thisSample[-1].SetLineStyle(1)
    elif(iSample%3==1):
      thisSample[-1].SetLineStyle(9)
      thisSample[-1].SetLineWidth(3)
    else:
      thisSample[-1].SetLineStyle(2)

    leg.AddEntry(thisSample[-1], sampleName[iSample], "l")
    thisSample[-1].Draw("same hist l")
  c1.cd()
  leg.Draw()
  c1.SaveAs(outDir+"Frac_SubsetSamplingEnergy_all"+binning+".png")
#  pad1.SetLogy()
#  c1.SaveAs(outDir+"SamplingEnergy_all"+binning+"_logy.png")
#  pad1.SetLogy(0)
  c1.Clear()

  ###### Do straight difference ##########
  thisSample = []
  leg = TLegend(0.83, 0.15, 0.99, 0.95)
  pad1 = TPad("pad1", "", 0, 0, 0.83, 1)
  pad1.Draw()
  pad1.cd()

  ## Draw a ghost copy for it's axis first ##
  thisSample.append(mcDir.Get("prof_recoilPt_SamplingLayerPercent0"+binning) )
  thisSample[-1].Add(thisSample[-1], -1.)
  thisSample[-1].GetYaxis().SetTitle( "Difference in Recoil Energy per Layer" )
  thisSample[-1].GetYaxis().SetRangeUser(-0.1,0.1)
  thisSample[-1].SetLineColor(kWhite)
  thisSample[-1].SetMarkerColor(kWhite)
  thisSample[-1].Draw("hist l")

  for iSample in sampleSubset:
    thisSample.append( mcDir.Get("prof_recoilPt_SamplingLayerPercent"+str(iSample)+binning) )
    thisSample[-1].SetName(thisSample[-1].GetName()+str(iSample))
    dataSample = dataDir.Get("prof_recoilPt_SamplingLayerPercent"+str(iSample)+binning)
    thisSample[-1].Add(dataSample, -1.)
    thisSample[-1].GetYaxis().SetRangeUser(-0.1,0.1)

    #thisSample[-1].GetXaxis().SetRangeUser(200., 2000.)
    thisSample[-1].SetLineColor( gStyle.GetColorPalette(10*iSample+1) )
    thisSample[-1].SetMarkerColor( gStyle.GetColorPalette(10*iSample+1) )
    thisSample[-1].SetMarkerSize(0.5)
    if(iSample%3 == 0):
      thisSample[-1].SetLineStyle(1)
    elif(iSample%3==1):
      thisSample[-1].SetLineStyle(9)
      thisSample[-1].SetLineWidth(3)
    else:
      thisSample[-1].SetLineStyle(2)

    leg.AddEntry(thisSample[-1], sampleName[iSample], "l")
    thisSample[-1].Draw("same hist l")
  c1.cd()
  leg.Draw()
  c1.SaveAs(outDir+"Diff_SubsetSamplingEnergy_all"+binning+".png")
  c1.Clear()


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--dataFile", dest='dataFile', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  parser.add_argument("--mcFile", dest='mcFile', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  parser.add_argument("--binning", dest='binning', default="",
           help="Name of binning used")
  args = parser.parse_args()

  plotSLDataMCRatio(args.dataFile, args.mcFile, args.binning):
