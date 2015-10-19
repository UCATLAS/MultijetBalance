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


import os, argparse, math, time
from ROOT import *
import AtlasStyle

def plotSysRatios(file):

  outDir = file[:-5]+'/'
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  outDir += "plotSysRatios/"
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


##################### Plot Systematic Difference  #################################
  sysDirNameList = [dir for dir in dirList if not "Nominal" in dir]
  sysDirList = []
  for sysDirName in sysDirNameList:
    sysDirList.append( inFile.Get(sysDirName) )

  ## Combine systematics in types ##
#  sysTypes = ["Zjet", "Gjet", "LAr", "Flavor", "EtaIntercalibration", "MJB", "All"]
  sysTypes = ["MJB_a", "MJB_b", "MJB_ptt", "MJB_pta", "All"]
# SingleParticle, RelativeNonClosure, Pileup, BJES, PunchThrough
  if "All" in sysTypes:
    colorOffset = 240./(len(sysTypes)-1)
  else:
    colorOffset = 240./len(sysTypes)

  print "Plotting systematic differences "

  histList = [key.GetName() for key in nomDir.GetListOfKeys()]
  for histName in histList:
    if "prof_" in histName or "ptSlice" in histName:
      continue

    nomHist = nomDir.Get( histName )
    nomHist.SetName(nomHist.GetName())

    if not type(nomHist) == TH1F and not type(nomHist) == TH1D:  #Can't draw bands if not 1D
      continue

    ## Get list of systematic histograms ##
    fullHistList = []
    for thisSysDir in sysDirList:
      fullHistList.append( thisSysDir.Get(histName) )
      fullHistList[-1].SetDirectory(0)

    ### Setup Plot ###
    leg = TLegend(0.83, 0.15, 0.99, 0.95)
    pad1 = TPad("pad1", "", 0, 0, 0.83, 1)
    pad1.Draw()
    pad1.cd()

    ## An empty histogram for plot settings ##
    settingsHist = nomHist.Clone()
    for iBin in range(1, settingsHist.GetNbinsX()+1):
        settingsHist.SetBinContent(iBin, 0.)
        settingsHist.SetBinError(iBin, 0.)

    settingsHist.GetYaxis().SetTitle( "Rel. Uncert. on "+settingsHist.GetYaxis().GetTitle())
    settingsHist.SetLineColor(kWhite)
    settingsHist.SetMarkerColor(kWhite)
    settingsHist.GetYaxis().SetRangeUser(-1., 2.)
    if("MJB" in histName):
      settingsHist.GetXaxis().SetRangeUser( 400, 3000 )
      settingsHist.GetXaxis().SetMoreLogLabels(True)
      settingsHist.GetYaxis().SetRangeUser(-0.04, 0.04)

    settingsHist.Draw()

    sysHistList = []
    for iTopSys, topSysName in enumerate(sysTypes):

      if topSysName == 'All':
        subHistList = [thisHist for thisName, thisHist in zip(sysDirNameList, fullHistList) if any(otherTopName in thisName for otherTopName in sysTypes) ]
      else:
        subHistList = [thisHist for thisName, thisHist in zip(sysDirNameList, fullHistList) if topSysName in thisName]

      sysHistUp, sysHistDn = getCombinedSysHist(nomHist, subHistList, topSysName)

      if topSysName == 'All':
        color = kBlack
      else:
        color = gStyle.GetColorPalette(int(colorOffset*(iTopSys+1)))

      sysHistUp.SetLineColor(color)
      sysHistUp.SetMarkerColor(color)
      sysHistUp.SetMarkerSize(0.8)
      sysHistDn.SetLineColor(color)
      sysHistDn.SetMarkerColor(color)
      sysHistDn.SetMarkerSize(0.8)

      sysHistUp.Draw("same hist lp")
      sysHistDn.Draw("same hist lp")
      if( topSysName == "EtaIntercalibration"):
        leg.AddEntry( sysHistUp, "EIC", "lp")
      else:
        leg.AddEntry( sysHistUp, topSysName, "lp")

      # Save them in a list
      sysHistList.append( sysHistUp )
      sysHistList.append( sysHistDn )

    c1.cd()
    leg.Draw()
    if( "MJB" in histName):
      pad1.SetLogx()
    c1.SaveAs(outDir+nomHist.GetName()+"_fracDiff.png" )
    c1.Clear()


  inFile.Close()

########################################################
# Take a list of systematics histograms and make a up  #
# and down hists of the fractional difference compared #
# to Nominal                                           #
########################################################
def getCombinedSysHist(nomHist, sysHistList, sysName = "tempSysHist"):
  sysHistUp = nomHist.Clone(sysName+"Up")
  sysHistDn = nomHist.Clone(sysName+"Dn")


  ## Clear Hist ##
  for iBin in range(1, sysHistUp.GetNbinsX()+1):
    sysHistUp.SetBinContent(iBin, 0.)
    sysHistUp.SetBinError(iBin, 0.)
    sysHistDn.SetBinContent(iBin, 0.)
    sysHistDn.SetBinError(iBin, 0.)

  for subSysHist in sysHistList:
    tmpSubSysHist = subSysHist.Clone("tmpSubSys")

    ## Get fractional difference of subsystematic ##
    tmpSubSysHist.Add( nomHist, -1.)
    tmpSubSysHist.Divide( nomHist )

    ## Add squared fractional difference of each bin of subsystematic to top systematic ##
    for iBin in range(1, tmpSubSysHist.GetNbinsX()+1):
      if tmpSubSysHist.GetBinContent(iBin) > 0.:
        sysHistUp.SetBinContent(iBin, sysHistUp.GetBinContent(iBin) + tmpSubSysHist.GetBinContent(iBin)**2 )
      elif tmpSubSysHist.GetBinContent(iBin) < 0.:
        sysHistDn.SetBinContent(iBin, sysHistDn.GetBinContent(iBin) + tmpSubSysHist.GetBinContent(iBin)**2 )

  # Get fractional difference of top systematic
  for iBin in range(1, sysHistUp.GetNbinsX()+1):
    if sysHistUp.GetBinContent(iBin) > 0.:
      sysHistUp.SetBinContent( iBin, math.sqrt(sysHistUp.GetBinContent(iBin)) )
  # Get fractional difference of bottom systematic and make it negative
  for iBin in range(1, sysHistDn.GetNbinsX()+1):
    if sysHistDn.GetBinContent(iBin) > 0.:
      sysHistDn.SetBinContent( iBin, -1.*math.sqrt(sysHistDn.GetBinContent(iBin)) )

  return sysHistUp, sysHistDn

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--file", dest='file', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  args = parser.parse_args()

  plotSysRatios(args.file)
