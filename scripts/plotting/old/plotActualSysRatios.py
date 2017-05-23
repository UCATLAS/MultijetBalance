#!/usr/bin/env python

###################################################################
# plotActualSysRatios.py                                                #
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

def plotActualSysRatios(file):

  outDir = file[:-5]+'/'
  if not os.path.exists(outDir):
    os.mkdir(outDir)
  outDir += "plotActualSysRatios/"
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
  sysDirNameList = [dirName for dirName in dirList if not "Nominal" in dirName]

  sysDirList = []
  for sysDirName in sysDirNameList:
    sysDirList.append( inFile.Get(sysDirName) )

  ## This removes extra MJB systematics
#  MJBsToUse = ["a40","a20","b15","b05","pta90","pta70","ptt30","ptt20"]
#  sysDirList = [sysDir for sysDir in sysDirList if (not "MJB" in sysDir.GetName() or any(MJBtouse in sysDir.GetName() for MJBtouse in MJBsToUse) )]
  sysDirList = [sysDir for sysDir in sysDirList if not "Nominal" in sysDir.GetName()]

  ## Combine systematics in types ##

  #sysTypesToUse = ["Zjet_dPhi","Zjet_MC","Zjet_MuScale","Zjet_MuSmearID","Zjet_MuSmearMS","Zjet_KTerm","Zjet_Veto","Zjet_Stat1","Zjet_Stat2","Zjet_Stat3","Zjet_Stat4","Zjet_Stat5","Zjet_Stat6","Zjet_Stat7","Zjet_Stat8","Zjet_Stat9","Zjet_Stat10","Zjet_Stat11","Zjet_Stat12","Zjet_Stat13"]
  #sysTypesToUse = ["Gjet_dPhi_","Gjet_Generator_","Gjet_OOC_","Gjet_Purity_","Gjet_Veto_","Gjet_Stat1_","Gjet_Stat2_","Gjet_Stat3_","Gjet_Stat4_","Gjet_Stat5_","Gjet_Stat6_","Gjet_Stat7_","Gjet_Stat8_","Gjet_Stat9_","Gjet_Stat10_","Gjet_Stat11_","Gjet_Stat12_","Gjet_Stat13_","Gjet_Stat14_","Gjet_Stat15_"]
  #sysTypesToUse = ["Gjet_dPhi","Gjet_Generator","Gjet_Stat14_","Gjet_Stat15_"]


  sysTypesToUse = ["Zjet", "Gjet", "Flavor", "EtaIntercalibration", "PunchThrough", "Pileup", "MCType", "MJB"]
  #sysTypesToUse = ["Zjet", "Gjet", "Flavor", "EtaIntercalibration", "PunchThrough", "Pileup", "MCType", "MJB"]
####sysTypesToUse = ["EtaIntercalibration_Modelling", "EtaIntercalibration_TotalStat", "EtaIntercalibration_NonClosure", "EtaIntercalibration_OFCs"]
###  #sysTypesToUse = ["Zjet_Jvt", "Zjet_ElecESZee", "Zjet_ElecEsmear", "Gjet_Jvt", "Gjet_GamESZee", "LAr_Esmear"]
####  sysTypesToUse = ["MJB_a", "MJB_b", "MJB_ptt", "MJB_pta", "MCType"]
#### SingleParticle, RelativeNonClosure, Pileup, BJES, PunchThrough

  sysTypes = []
  for sysType in sysTypesToUse:
    if any(sysType in sysDir for sysDir in sysDirNameList):
      sysTypes.append( sysType )

  if len(sysTypes) == 0:
    print "Error, found no systematics!!"
    exit(1)
  if len(sysTypes) > 1:
    sysTypes.append("All")
  if "All" in sysTypes:
    colorOffset = 240./(len(sysTypes)-1)
  else:
    colorOffset = 240./len(sysTypes)

  #print "Plotting systematic differences "
  #print sysTypes

  histList = [key.GetName() for key in sysDirList[0].GetListOfKeys()]



  for histName in histList:
    if "prof_" in histName or "ptSlice" in histName:
      continue

    nomHist = sysDirList[0].Get( histName )
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

    settingsHist.GetYaxis().SetTitle( "Rel. Uncert. "+nomHist.GetYaxis().GetTitle())
    #settingsHist.GetYaxis().SetTitle( "Rel. Uncert. on "+settingsHist.GetYaxis().GetTitle())
    settingsHist.SetLineColor(kWhite)
    settingsHist.SetMarkerColor(kWhite)
    settingsHist.GetYaxis().SetRangeUser(-1., 2.)
    if("MJB" in histName):
      settingsHist.GetXaxis().SetRangeUser( 200, 4000 )
      settingsHist.GetXaxis().SetMoreLogLabels(True)
      settingsHist.GetYaxis().SetRangeUser(-0.03, 0.03)

    settingsHist.Draw()

    sysHistList = []
    for iTopSys, topSysName in enumerate(sysTypes):


      if topSysName == 'All':
        subHistList = [thisHist for thisDir, thisHist in zip(sysDirList, fullHistList) if any(otherTopName in thisDir.GetName() for otherTopName in sysTypes) ]
      else:
        subHistList = [thisHist for thisDir, thisHist in zip(sysDirList, fullHistList) if topSysName in thisDir.GetName()]

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
      #if( topSysName == "EtaIntercalibration"):
      #  leg.AddEntry( sysHistUp, "#eta-inter", "lp")
      if( "EtaIntercalibration" in topSysName ):
        leg.AddEntry( sysHistUp, topSysName.replace('EtaIntercalibration','#eta-inter'), "lp")
        #leg.AddEntry( sysHistUp, topSysName.replace('EtaIntercalibration_',''), "lp")
      elif( topSysName == "PunchThrough"):
        leg.AddEntry( sysHistUp, "PunchTh", "lp")
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


    ## This is already set!
    ### Get fractional difference of subsystematic ##
    #tmpSubSysHist.Add( nomHist, -1.)
    #tmpSubSysHist.Divide( nomHist )

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

  plotActualSysRatios(args.file)
