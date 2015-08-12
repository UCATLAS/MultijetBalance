#!/usr/bin/env python

###################################################################
# plotSL.py                                                       #
# A MJB second stage python script                                #
# Author Jeff Dandoy, UChicago                                    #
#                                                                 #
# This script takes a data or MC appended file and plots the      #
# Sampling Layer Percentages on a single plot.                    #
#                                                                 #
###################################################################


import os
import argparse

from ROOT import *
import AtlasStyle



def plotSL(file, binning):

  if not "appended.root" in file:
    print "Error:  plotSL.py must run on a file ending in \"appended.root\", but was given", file
    print "Exiting..."
    return

  if not binning[0] == '_':
    binning = '_'+binning

  outDir = file[:-5]+'/'
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  outDir += "plotSL/"
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


  #################### New Sampling Layer Histograms ##########################
  numSamples = 24
  sampleName = ["PreSamplerB", "EMB1", "EMB2", "EMB3", "PreSamplerE", "EME1", "EME2", "HEC0", "HEC1", "HEC2", "HEC3", "TileBar0", "TileBar1", "TileBar2", "TileGap1", "TileGap2", "TileGap3", "TileExt0", "TileExt1", "TileExt2", "FCAL0", "FCAL1", "FCAL2", "MINIFCAL0", "MINIFCAL1", "MINIFCAL2", "MINIFCAL3"]

  print "Plotting new hists for "
  for dir in dirList:
    print "           ", dir
    if not os.path.exists(outDir+dir):
      os.mkdir( outDir+dir )

    thisDir = inFile.Get( dir )
    thisSample = []
    leg = TLegend(0.80, 0.15, 0.97, 0.95)
    pad1 = TPad("pad1", "", 0, 0, 0.85, 1)
    pad1.Draw()
    pad1.cd()
    ## Draw a ghost copy for it's axis first ##
    thisSample.append(thisDir.Get("prof_recoilPt_SamplingLayerPercent0"+binning) )
    thisSample[-1].Add( thisSample[-1], -1.)
    #for iBin in range(1, thisSample[-1].GetNbinsX()+1):
    #  thisSample[-1].SetBinError(iBin, 0.)

    #if("NoCorr" in dir):
    if("Nominal" in dir):
      thisSample[-1].GetYaxis().SetTitle("Percent of Recoil Energy per Layer")
      thisSample[-1].GetYaxis().SetRangeUser(0.004, 0.5)
    elif ("NoCorr" in dir):
      thisSample[-1].GetYaxis().SetTitle("Rel. Uncert. in % Recoil E per Layer")
      thisSample[-1].GetYaxis().SetRangeUser(-0.3, 0.3)
    else:
      thisSample[-1].GetYaxis().SetTitle("Rel. Uncert. in % Recoil E per Layer")
      thisSample[-1].GetYaxis().SetRangeUser(-0.1, 0.1)

    thisSample[-1].SetMarkerSize(0.)
    thisSample[-1].Draw()
    for iSample in range(0, numSamples):
      thisSample.append( thisDir.Get("prof_recoilPt_SamplingLayerPercent"+str(iSample)+binning) )
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

      #if( not "NoCorr" in dir):
      if( not "Nominal" in dir):
        nomSample = nomDir.Get("prof_recoilPt_SamplingLayerPercent"+str(iSample)+binning)
        thisSample[-1].Add(nomSample, -1.)
        thisSample[-1].Divide(nomSample)
#        for iBin in range(1, thisSample[-1].GetNbinsX()+1):
#          thisSample[-1].SetBinError(iBin, 0.)

      #if("NoCorr" in dir):
      if("Nominal" in dir):
        thisSample[-1].GetYaxis().SetTitle("Percent of Recoil Energy per Layer")
        thisSample[-1].GetYaxis().SetRangeUser(0.004, 0.5)
      elif ("NoCorr" in dir):
        thisSample[-1].GetYaxis().SetTitle("Rel. Uncert. in % Recoil E per Layer")
        thisSample[-1].GetYaxis().SetRangeUser(-0.3, 0.3)
      else:
        thisSample[-1].GetYaxis().SetTitle("Rel. Uncert. in % Recoil E per Layer")
        thisSample[-1].GetYaxis().SetRangeUser(-0.1, 0.1)

      leg.AddEntry(thisSample[-1], sampleName[iSample], "l")
      #if( "NoCorr" in dir):
      if( "Nominal" in dir):
        thisSample[-1].Draw("same hist pe")
      else:
        thisSample[-1].Draw("same hist l")

    c1.cd()
    leg.Draw()
    c1.SaveAs(outDir+dir+"/SamplingEnergy_all"+binning+".png")
    #if( "NoCorr" in dir):
    if( "Nominal" in dir):
      pad1.SetLogy()
      c1.SaveAs(outDir+dir+"/SamplingEnergy_all"+binning+"_logy.png")
      pad1.SetLogy(0)
    c1.Clear()


  inFile.Close()


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--file", dest='file', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  parser.add_argument("--binning", dest='binning', default="",
           help="Name of binning used")
  args = parser.parse_args()

  plotSL(args.file, args.binning)
