#!/usr/bin/env python

###################################################################
# calculateMJBHists.py                                            #
# A MJB second stage python script                                #
# Author Jeff Dandoy, UChicago                                    #
#                                                                 #
# This script takes any scaled file of histograms and creates new #
# histograms from the results into a file named *_appended.       #
# A unique file for MJBs is also created.                         #
#                                                                 #
# Histograms saved to the new file are:                           #
#   A copy of every histogram in scaled.                          #
#   A profile hist of each sampling layer (Percent).              #
#   A MJB correction.                                             #
#   Each slice in recoilPt of recoilPt_PtBalCor.                  #
#   newDir = appended , correctionDir = MJB only                  #
#                                                                 #
###################################################################


import os, time
import argparse

from ROOT import *

def calculateMJBHists(file, binnings, f_extraPlots):


  binnings = binnings.split(',')
  for iBinning in range(len(binnings)):
    if not binnings[iBinning][0] == '_':
      binnings[iBinning] = '_'+binnings[iBinning]
  print "Binnings are ", binnings

  if not "scaled" in file:
    print "Error, trying to run calculateMJBHists.py on non \"scaled\" input ", file
    print "Exiting"
    return

  inFile = TFile.Open(file, "READ");
  file = file.replace('scaled','appended')
  outFile = TFile.Open(file, "RECREATE");
  file = file.replace('appended','MJB_initial')
  correctionFile = TFile.Open(file, "RECREATE");

  keyList = [key.GetName() for key in inFile.GetListOfKeys()] #List of top level objects
  dirList = [key for key in keyList if "Iteration" in key] #List of all directories

  MJBcorrectionTags = [""]
  #MJBcorrectionTags = ["", "_eta1", "_eta2", "_eta3"]
  numSamplingLayers = 24


##################### Get a new copy of original histograms and create new histograms  #################################
  print "Creating new hists "
  for dir in dirList:
    #print "           ", dir
    outFile.mkdir( dir )
    newDir = outFile.Get( dir )
    oldDir = inFile.Get( dir )
    correctionFile.mkdir( dir )
    correctionDir = correctionFile.Get( dir )

    histList = [key.GetName() for key in oldDir.GetListOfKeys()]
    ### Save all previous histograms ###
    for histName in histList:
      thisHist = oldDir.Get(histName)
      thisHist.SetDirectory( newDir )
      if( "Phi" in histName and type(thisHist) is TH1F):
        thisHist.Rebin(4)
      if( "Beta" in histName and type(thisHist) is TH1F):
        thisHist.Rebin(2)

      if thisHist.GetName() == "recoilPt":
        correctionDir.cd()
        thisHist.Write()

    if(f_extraPlots):
      #### Create Sampling Layer unbinned histograms ####
      for iSample in range(0, numSamplingLayers):
        sampHist2D = oldDir.Get( ("recoilPt_SamplingLayerPercent"+str(iSample)) )
        sampHist2D.SetDirectory(0)
#        sampHist2D.Sumw2()
        prof_sampHist = sampHist2D.ProfileX("tmpProf_"+sampHist2D.GetName(), 1, -1, "")
        prof_sampHist.SetDirectory(0)
        sampHist = prof_sampHist.ProjectionX("prof_"+sampHist2D.GetName())
        sampHist.SetTitle( "Profile "+sampHist2D.GetTitle() )
        sampHist.SetDirectory(newDir)

      #### Create Sampling Layer binned histograms ####
      for iSample in range(0, numSamplingLayers):
        sampHist2D = oldDir.Get( ("recoilPt_SamplingLayerPercent"+str(iSample)+binning) )
        sampHist2D.SetDirectory(0)
#        sampHist2D.Sumw2()
        prof_sampHist = sampHist2D.ProfileX("tmpProf_"+sampHist2D.GetName(), 1, -1, "")
        prof_sampHist.SetDirectory(0)
        sampHist = prof_sampHist.ProjectionX("prof_"+sampHist2D.GetName())
        sampHist.SetTitle( "Profile "+sampHist2D.GetTitle() )
        sampHist.SetDirectory(newDir)

    ## Save finely binned recoilPt histogram for finalMJB calculation ##
    recoilPt_center = oldDir.Get( "recoilPt_center" )
    if recoilPt_center:
      correctionDir.cd()
      recoilPt_center.Write()

    for binning in binnings:

      for thisTag in MJBcorrectionTags:  ## for each eta range ##
        ### Get actual MJB correction using ProfileX ###
        recoilPt_PtBal = oldDir.Get( "recoilPt_PtBal"+thisTag+binning )
#        recoilPt_PtBal.Rebin2D(2,1)
#        recoilPt_PtBal.Sumw2()
        prof_MJBcorrection = recoilPt_PtBal.ProfileX("prof_MJB"+thisTag+binning, 1, -1, "")
        MJBcorrection = prof_MJBcorrection.ProjectionX("MJB"+thisTag+binning)
        MJBcorrection.SetTitle("MJBcorrection"+thisTag)
        MJBcorrection.GetYaxis().SetTitle( "p_{T}^{Jet 1}/p_{T}^{Recoil}" )
        correctionDir.cd()
        MJBcorrection.Write()
        MJBcorrection.SetDirectory(newDir)

        if(f_extraPlots):
          ### Get numevents per correction for each pt slice using ProjectionY ###
          for iBin in range(1,recoilPt_PtBal.GetNbinsX()+1):
            histSlice = recoilPt_PtBal.ProjectionY( "recoilPt_PtBal"+thisTag+binning+"_ptSlice"+str(iBin), iBin, iBin)
            histSlice.SetTitle(recoilPt_PtBal.GetTitle()+" for ptSlice "+str(iBin) )
            histSlice.SetDirectory(newDir)

      ### Get extra MJB correction based on leadpT ###
      leadJetPt_PtBal = oldDir.Get( "leadJetPt_PtBal"+binning )
#      leadJetPt_PtBal.Sumw2()
      prof_MJBcorrection = leadJetPt_PtBal.ProfileX("prof_MJB_leadJet"+binning, 1, -1, "")
      MJBcorrection = prof_MJBcorrection.ProjectionX("MJB_leadJet"+binning)
      MJBcorrection.SetTitle("MJBcorrection_leadJet")
      MJBcorrection.GetYaxis().SetTitle( "p_{T}^{Jet 1}/p_{T}^{Recoil}" )
      correctionDir.cd()
      MJBcorrection.Write()
      MJBcorrection.SetDirectory(newDir)


#  print "Writing file ", outFile.GetName()
  outFile.Write()
  outFile.Close()
  correctionFile.Close()
  inFile.Close()



if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("-extraPlots", dest='extraPlots', action='store_true', default=False, help="Plot profile of recoilPt vs PtBal for all eta's and pt slices, and create extra sampling layer plots")
  parser.add_argument("--file", dest='file', default="submitDir/hist-data12_8TeV.root",
           help="Input file name")
  parser.add_argument("--binnings", dest='binnings', default="",
           help="Comma Seperated List of binnings used")
  args = parser.parse_args()

  calculateMJBHists(args.file, args.binnings, args.extraPlots)

