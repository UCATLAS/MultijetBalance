#!/usr/bin/env python

###################################################################
# plotSysRatios.py                                                #
# A MJB second stage python script                                #
# Author Jeff Dandoy, UChicago                                    #
#                                                                 #
# This script takes any scaled, appended, or MJBCorrection_initial #
# file and plots systematic relative differences with nominal.    #
#                                                                 #
# Systematics histograms must be original values, not differences.#
###################################################################


import os
import argparse
import math

from ROOT import *
import AtlasStyle


def main():
  outDir = args.file[:-5]+'/'
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  outDir += "plotSysRatios/"
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  AtlasStyle.SetAtlasStyle()

  gROOT.ProcessLine("gErrorIgnoreLevel = 2000") #Ignore TCanvas::Print info

  inFile = TFile.Open(args.file, "READ");
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


##################### Plot Systematic Difference  #################################
  #sysDirNameList = [dir for dir in dirList if not "NoCorr" in dir]
  sysDirNameList = [dir for dir in dirList if not "Nominal" in dir]

  ## Combine systematics in types ##
  sysTypes = ["ZJ", "GJ", "Flavor", "EtaIntercalibration", "MJB"]
  #sysTypes = ["ZJ", "GJ", "MJB_a", "MJB_b", "MJB_ptt", "MJB_pta", "Flavor", "EtaIntercalibration"]

  subSysDirList = [] #Corresponding list of all systematic directories
  for topSysName in sysTypes:
    subSysDirList.append([])
    for iSys, sysDirName in enumerate(sysDirNameList):
      if topSysName in sysDirName[11:]: #Remove prepended Iteration#_
        subSysDirList[-1].append( inFile.Get( sysDirName ) )

  print "Plotting systematic differences "

  histList = [key.GetName() for key in nomDir.GetListOfKeys()]
  for histName in histList:
    if "prof_" in histName or "ptSlice" in histName:
      continue

    nomHist = nomDir.Get( histName )
    nomHist.SetName(nomHist.GetName())

    if not type(nomHist) == TH1F and not type(nomHist) == TH1D:  #Can't draw bands if not 1D
      continue

    leg = TLegend(0.83, 0.15, 0.99, 0.95)
    pad1 = TPad("pad1", "", 0, 0, 0.83, 1)
    pad1.Draw()
    pad1.cd()
    #nomHist.SetMinimum(0.9)
    #if not "binned" in histName:
      #if nomHist.GetNbinsX()%3 == 0:
      #  nomHist.Rebin(3)
      #elif nomHist.GetNbinsX()%2 == 0:
      #  nomHist.Rebin()
    #nomHist.SetMaximum(2.)
    #nomHist.SetMinimum(-1.)
    tmpHist = nomHist.Clone()
    for iBin in range(1, tmpHist.GetNbinsX()+1):
        tmpHist.SetBinContent(iBin, 0.)
        tmpHist.SetBinError(iBin, 0.)

    tmpHist.GetYaxis().SetTitle( "Rel. Uncert. on "+tmpHist.GetYaxis().GetTitle())
    tmpHist.GetYaxis().SetRangeUser(-1., 2.)
    if("MJB" in histName):
      tmpHist.GetXaxis().SetRangeUser( 300, 2000 )
      tmpHist.GetXaxis().SetMoreLogLabels(True)
      tmpHist.GetYaxis().SetRangeUser(-0.02, 0.04)
    #elif( "Pt" in histName or "Energy" in histName):
    #  tmpHist.GetXaxis().SetRange(tmpHist.FindFirstBinAbove(0), tmpHist.FindLastBinAbove(0) )

    tmpHist.SetLineColor(kWhite)
    tmpHist.SetMarkerColor(kWhite)
    tmpHist.Draw()

    ## Add systematic bands ##
    sysHistUp = []
    sysHistDn = []
    sysHistAllUp = tmpHist.Clone("allSystUp")
    sysHistAllDn = tmpHist.Clone("allSystDn")
    sysHistAllUp.SetMarkerSize(0.8)
    sysHistAllUp.SetLineColor( kBlack )
    sysHistAllUp.SetMarkerColor( kBlack )
    sysHistAllDn.SetMarkerSize(0.8)
    sysHistAllDn.SetLineColor( kBlack )
    sysHistAllDn.SetMarkerColor( kBlack )

    for iTopSys, topSysName in enumerate(sysTypes):
      sysHistUp.append( tmpHist.Clone("systematicUp"+str(iTopSys)) )
      sysHistUp[-1].SetLineColor(gStyle.GetColorPalette(40*(iTopSys+1)))
      sysHistUp[-1].SetMarkerColor(gStyle.GetColorPalette(40*(iTopSys+1)))
      sysHistUp[-1].SetMarkerSize(0.8)
      sysHistDn.append( tmpHist.Clone("systematicDn"+str(iTopSys)) )
      sysHistDn[-1].SetLineColor(gStyle.GetColorPalette(40*(iTopSys+1)))
      sysHistDn[-1].SetMarkerColor(gStyle.GetColorPalette(40*(iTopSys+1)))
      sysHistDn[-1].SetMarkerSize(0.8)

      for iSubSysDir, subSysDir in enumerate( subSysDirList[iTopSys] ):
        #if not "binned" in histName:
          #if sysHists[-1].GetNbinsX()%3 == 0:
          #  sysHists[-1].Rebin(3)
          #elif sysHists[-1].GetNbinsX()%2 == 0:
          #  sysHists[-1].Rebin()
        #if( "Pt" in histName or "Energy" in histName):
        #  sysHists[-1].GetXaxis().SetRange(sysHists[-1].FindFirstBinAbove(0), sysHists[-1].FindLastBinAbove(0))
        thisSubSysHist = subSysDir.Get( histName )

        #Get fractional difference of subsystematic
        thisSubSysHist.Add( nomHist, -1.)
        thisSubSysHist.Divide( nomHist )
        #Add squared fractional difference of each bin of subsystematic to top systematic
        for iBin in range(1, thisSubSysHist.GetNbinsX()+1):
          if thisSubSysHist.GetBinContent(iBin) > 0.:
            sysHistUp[-1].SetBinContent(iBin, sysHistUp[-1].GetBinContent(iBin) + thisSubSysHist.GetBinContent(iBin)**2 )
            sysHistAllUp.SetBinContent(iBin, sysHistAllUp.GetBinContent(iBin) + thisSubSysHist.GetBinContent(iBin)**2 )
          elif thisSubSysHist.GetBinContent(iBin) < 0.:
            sysHistDn[-1].SetBinContent(iBin, sysHistDn[-1].GetBinContent(iBin) + thisSubSysHist.GetBinContent(iBin)**2 )
            sysHistAllDn.SetBinContent(iBin, sysHistAllDn.GetBinContent(iBin) + thisSubSysHist.GetBinContent(iBin)**2 )

      # Get fractional difference of top systematic
      for iBin in range(1, sysHistUp[-1].GetNbinsX()+1):
        if sysHistUp[-1].GetBinContent(iBin) > 0.:
          sysHistUp[-1].SetBinContent( iBin, math.sqrt(sysHistUp[-1].GetBinContent(iBin)) )
      # Get fractional difference of top systematic and make it negative
      for iBin in range(1, sysHistDn[-1].GetNbinsX()+1):
        if sysHistDn[-1].GetBinContent(iBin) > 0.:
          sysHistDn[-1].SetBinContent( iBin, -1.*math.sqrt(sysHistDn[-1].GetBinContent(iBin)) )


      sysHistUp[-1].Draw("same hist lp")
      sysHistDn[-1].Draw("same hist lp")
      if( topSysName == "EtaIntercalibration"):
        leg.AddEntry( sysHistUp[-1], "EIC", "lp")
      else:
        leg.AddEntry( sysHistUp[-1], topSysName, "lp")

    # Get fractional difference of total up systematic
    for iBin in range(1, sysHistAllUp.GetNbinsX()+1):
      if sysHistAllUp.GetBinContent(iBin) > 0:
        sysHistAllUp.SetBinContent( iBin, math.sqrt(sysHistAllUp.GetBinContent(iBin)) )
    # Get fractional difference of total down systematic
    for iBin in range(1, sysHistAllDn.GetNbinsX()+1):
      if sysHistAllDn.GetBinContent(iBin) > 0:
        sysHistAllDn.SetBinContent( iBin, -1.*math.sqrt(sysHistAllDn.GetBinContent(iBin)) )

    sysHistAllUp.Draw("same hist lp")
    sysHistAllDn.Draw("same hist lp")
    leg.AddEntry( sysHistAllUp, "Total", "lp")


    c1.cd()
    leg.Draw()
    if( "MJB" in histName):
      pad1.SetLogx()
    c1.SaveAs(outDir+nomHist.GetName()+"_fracDiff.png" )
    c1.Clear()


  inFile.Close()

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--file", dest='file', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  args = parser.parse_args()

  main()

