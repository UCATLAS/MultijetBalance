#!/usr/bin/env python

###################################################################
# calcMJB.py                                                      #
# A MJB second stage python script                                #
# Author Jeff Dandoy, UChicago                                    #
#                                                                 #
# This script performs the calculations on MJB Samples.           #



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

import MJBSample, ROOT
import os

def fitBalance(sample):
  command = 'MJBFit --fit --file '+sample.histFile
#  if (doNominalOnly):
#  command += ' --sysType Nominal'
#  if (doBootstrap):
#    command += ' --rebinFileName '+rebinFile
  print command
  os.system(command)


def profileBalance(sample):

  inFile = ROOT.TFile.Open( sample.histFile, "UPDATE" )
  dirList = [key.GetName() for key in inFile.GetListOfKeys() if "Iteration" in key.GetName()] #List of directories

  ### In bootstrap mode, objects will be saved in histograms, not directories ###
  dirType = type(inFile.Get( dirList[0] ) )
  if dirType == ROOT.TH2F or dirType == ROOT.TH2D:
    if( sample.verbose ):  print "Running without directory structure, assuming a bootstrap object!"
    f_bootstrap = True
  else:
    f_bootstrap = False


  for thisDirName in dirList:

    ### If running on bootstrap, then make a new directory and move the original histogram to this new directory ###
    if f_bootstrap: 
      histName = thisDirName
      if not "_recoilPt_PtBal" in histName:
        continue
      newDirName = histName.replace("_recoilPt_PtBal", "")
      inFile.mkdir( newDirName )
      thisDir = inFile.Get( newDirName )
      thisHist = inFile.Get( histName )
      thisHist.SetDirectory( thisDir ) 
      histList = [histName]

    else:
    ### Just get a list of recoilPt_PtBal histograms in the directory ###
      thisDir = inFile.Get( thisDirName )
      histList = [key.GetName() for key in thisDir.GetListOfKeys() if 'recoilPt_PtBal' in key.GetName()]

    if any( 'profileBalance' in histName for histName in histList):
      if( sample.verbose ):  print "profileBalance histograms already exists for sample", sample.tag, ", continuing to next step"
      inFile.Close()
      return

    thisDir.cd()
    ### For each histogram, calculate the profile and save it to the input file ###
    for histName in histList:
      thisHist = thisDir.Get(histName)

      newHistName = histName.replace('recoilPt_PtBal', 'profileBalance')
      prof_balance = thisHist.ProfileX( newHistName+"_tmp" , 1, -1, "")
      proj_balance = prof_balance.ProjectionX(newHistName)
      proj_balance.SetTitle(newHistName)
      proj_balance.GetYaxis().SetTitle( "p_{T}^{Jet 1}/p_{T}^{Recoil}" )
      proj_balance.SetDirectory(thisDir)

      proj_balance.Write( proj_balance.GetName(), ROOT.TObject.kOverwrite )

  inFile.Close()
