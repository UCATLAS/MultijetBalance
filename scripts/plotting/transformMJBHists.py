## Jeff Dandoy ##

import glob, os
import argparse



parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
#parser.add_argument("--dir", dest='workDir', default='gridOutput/Oct21_2nd_corrected_area', help="Typically gridOutput")
#parser.add_argument("--dir", dest='workDir', default='gridOutput/Oct21_2nd_area', help="Typically gridOutput")
parser.add_argument("--dir", dest='workDir', default='gridOutput/VF_EM_Nov15_workarea/', help="Typically gridOutput")
#parser.add_argument("--dir", dest='workDir', default='gridOutput/combinedArea', help="Typically gridOutput")
args = parser.parse_args()



import getRecoilPtTree
import scaleHist
import calculateMJBHists
import calculateDoubleRatio
import calculateDoubleRatioSys
import calculateFinalMJBGraphs
import plotNominal
import plotSysRatios
import plotMJBvEta
import plotAll

# Do not run commands, but print them in the order they are applied
f_printOnly = False

## Choose the MC types to run on
## The first MC type will be the nominal, while the second will be a systematic variation
#mcTypes = ["Pythia", "Herwig", "Data207"]
#mcTypes = ["Pythia","CombineLast2","DropLast1","DropLast2"]
mcTypes = ["Pythia","Sherpa"]


isValidation = False
is2016 = False

## Order of steps to perform.  All are turned to False by default
f_getPtHist = False
f_scale = False
f_combine = False
f_calculateMJB = False
f_plotRelevant = False
f_plotAll = False
f_plotSL = False

#f_getPtHist = True
#f_scale = True #Scale MC files
#f_combine = True #Combine MC files
#f_calculateMJB = True # Calculate new histograms to be added to root file
f_plotRelevant = True # Most important Plots
f_plotAll = True # All plots
###f_plotSL = True  # Plot Sampling Layers

################# Running Options ########################3

## lastPt to use for MJB result
if (is2016):
  endPt = 3500
else:
  endPt = 2000

#doFinal will turn on the final step of finding the proper bin center from the jet pt spectrum
#it requires input TTrees
doFinal = True

#doBootstrap will combine bins based on previously derived bootstrap binnings
doBootstrap = True
rebinFile = "gridOutput/workarea_LC207_Aug15/B1/workarea/hist.data.all.significant.root"

#doFit will perform a gaussian fit on each bin rather than get the mean
doFit = True
doNominalOnly = False #Fit only nominal (for quicker results)

#doAverage will create extra output histograms and MJB histograms based on the mean values, not fits
doAverage = False

# Data and MC may be run separately for plotting purposes
doData = True
doMC = True

#Systematics may be turned off to speed things up
doSys = True

#Individual slices of MC may be run on separately for plotting purposes
doJZSlices = False

if not os.path.exists(args.workDir+'/workarea/initialFiles'):
  os.makedirs(args.workDir+'/workarea/initialFiles')

args.workDir = args.workDir+'/workarea/'

if(f_getPtHist and doFinal):
  treeDir = args.workDir+'/../tree/'
  if not os.path.exists( treeDir ):
    print "Warning, there is no tree directory in ", args.workDir+'/../'
    print "A pT histogram will not be created"
  else:
    print 'python MultijetBalance/scripts/plotting/getRecoilPtTree.py --pathToTrees '+treeDir
    getRecoilPtTree.getRecoilPtTree(treeDir, "outTree_Nominal")
    if (doData):   #Only pt distribution for data
      command = 'hadd '+args.workDir+'/ptBinTree_data15.root '+treeDir+'/ptBinTree_*data15*.root'
      print command
      os.system(command)


if(f_scale and doMC):
  for mcType in mcTypes:
    ## input -> scaled ##
    files = glob.glob(args.workDir+'/../hist/*mc15*'+mcType+'*_hist.root')
    for file in files:
      print 'python MultijetBalance/scripts/plotting/scaleHist.py --file '+file
      scaleHist.scaleHist(file)
    files = glob.glob(args.workDir+'/../hist/*scaled.root')
    for file in files:
      os.system('mv '+file+' '+args.workDir+'/initialFiles/')


if(f_combine):

  ## Hadd MC slices together into scaled ##
  if( doMC ):
    for mcType in mcTypes:
      command = 'hadd '+args.workDir+'/hist.mc.'+mcType+'.scaled.root '+args.workDir+'/initialFiles/*.mc*'+mcType+'*_hist.scaled.root'
      print command
      os.system(command)

    ## Create MCType uncertainty using 2 different MC generators ##
    if len(mcTypes) > 1:
      nominalFile = args.workDir+'/hist.mc.'+mcTypes[0]+'.scaled.root'
      sysFile =     args.workDir+'/hist.mc.'+mcTypes[1]+'.scaled.root'
      command = 'python MultijetBalance/scripts/plotting/combinedMCUncert.py --nominal '+nominalFile+' --sys '+sysFile
      print command
      os.system(command)


  if( doData ):
    command = 'hadd '+args.workDir+'/hist.data.all.scaled.root '+args.workDir+'/../hist/*.data*_hist.root'
    print command
    os.system(command)

  ## Create output for individual MC slices
  if(doJZSlices):
    files = glob.glob(args.workDir+'/initialFiles/*.mc*'+mcType+'*_hist.scaled.root')
    for file in files:
      JZString = os.path.basename(file).split('.')[4].split('_')[-1]
      os.system('cp '+file+' '+args.workDir+'/hist.mc.'+mcType+'_'+JZString+'.scaled.root')

  # Bootstrap rebinning requires fits encompassing several bins, where these bins vary for all systematics
  # MC is required to mirror data, so to accomplish this the nominal MC results are copied for each data-only systematic
  # Takes scaled_noDataSyst -> scaled
  if( doBootstrap and doMC and doData):
    for mcType in mcTypes:
      command = 'mv '+args.workDir+'/hist.mc.'+mcType+'.scaled.root '+args.workDir+'/hist.mc.'+mcType+'.scaled_noDataSyst.root'
      print command
      os.system(command)
      command = 'python MultijetBalance/scripts/plotting/addDataSystToMC.py --mcFile '+args.workDir+'/hist.mc.'+mcType+'.scaled_noDataSyst.root --dataFile '+args.workDir+'/hist.data.all.scaled.root'
      print command
      os.system(command)


if(f_calculateMJB):

  ## Create extra plots (appended) and a MJB output using mean values
  ## scaled -> .appended and .MJB_initial ##
  if( doAverage ):
    files = glob.glob(args.workDir+'/hist.*.*.scaled.root')
    for file in files:
      print 'python MultijetBalance/scripts/plotting/calculateMJBHists.py --file '+file
      if (not f_printOnly):
        f_extraPlots = False
        calculateMJBHists.calculateMJBHists(file, f_extraPlots)
        #os.system('mv '+file+' '+args.workDir+'/initialFiles/')

  if( doFit ):
    files = glob.glob(args.workDir+'/hist.*.*.scaled.root')


    for file in files:
      command = 'runFit --fit --file '+file
      command += ' --upperEdge '+str(endPt)
      if (doNominalOnly):
        command += ' --sysType Nominal'
      if (doBootstrap):
        command += ' --rebinFileName '+rebinFile
      print command
      if (not f_printOnly):
        os.system(command)

      ## If we're doing bootstrap, we must also create a new nominal version for each systematic
      ## so that their binnings may be matched, and the division of histograms may be performed
      if (doBootstrap):
        command = 'runFit_NominalRebinning --fit --file '+file
        command += ' --upperEdge '+str(endPt)
        if (doNominalOnly):
          command += ' --sysType Nominal'
        if (doBootstrap):
          command += ' --rebinFileName '+rebinFile
        print command
        if (not f_printOnly):
          os.system(command)


  ## data and MC .MJB_initial -> .DoubleMJB_initial ##
  if(doData and doMC):

    ## If we're doing bootstrap, we must also create a new nominal version for each systematic
    ## so that their binnings may be matched, and the division of histograms may be performed
    ## Get bootstrap result first
    if (doBootstrap):

      for mcType in mcTypes:
        if (doAverage):
          dataFile = glob.glob(args.workDir+'/hist.data.all.MJB_nominal.root')[0]
          mcFile = glob.glob(args.workDir+'/hist.mc.'+mcType+'.MJB_nominal.root')[0]
        if( doFit ):
          dataFile = glob.glob(args.workDir+'/hist.data.all.fit_MJB_nominal.root')[0]
          mcFile = glob.glob(args.workDir+'/hist.mc.'+mcType+'.fit_MJB_nominal.root')[0]

        print 'python MultijetBalance/scripts/plotting/calculateDoubleRatio.py --dataFile '+dataFile+' --mcFile '+mcFile
        if (not f_printOnly):
          calculateDoubleRatio.calculateDoubleRatio(dataFile, mcFile)


    ## Then get regular version
    for mcType in mcTypes:
      if (doAverage):
        dataFile = glob.glob(args.workDir+'/hist.data.all.MJB_initial.root')[0]
        mcFile = glob.glob(args.workDir+'/hist.mc.'+mcType+'.MJB_initial.root')[0]
      if( doFit ):
        dataFile = glob.glob(args.workDir+'/hist.data.all.fit_MJB_initial.root')[0]
        mcFile = glob.glob(args.workDir+'/hist.mc.'+mcType+'.fit_MJB_initial.root')[0]

      print 'python MultijetBalance/scripts/plotting/calculateDoubleRatio.py --dataFile '+dataFile+' --mcFile '+mcFile
      if (not f_printOnly):
        calculateDoubleRatio.calculateDoubleRatio(dataFile, mcFile)


    ## Next get systematic variations ##
    for mcType in mcTypes:
      if (doAverage):
        inFile = glob.glob(args.workDir+'/hist.combined.'+mcType+'.DoubleMJB_initial.root')[0]
      if( doFit ):
        inFile = glob.glob(args.workDir+'/hist.combined.'+mcType+'.Fit_DoubleMJB_initial.root')[0]

      bootstrapFile = ""
      if (doBootstrap):
        bootstrapFile = inFile.replace('DoubleMJB_initial','DoubleMJB_nominal')

      print 'python MultijetBalance/scripts/plotting/calculateDoubleRatioSys.py --inFile '+inFile
      if not f_printOnly:
        calculateDoubleRatioSys.calculateDoubleRatioSys(inFile, bootstrapFile)


  ## Needs recoilPt_center
  ## .(Double)MJB_initial -> .(Double)MJB_final ##
  if(doFinal):
    initialFiles = glob.glob(args.workDir+'/*_initial.root')
    for file in initialFiles:
      print 'python MultijetBalance/scripts/plotting/calculateFinalMJBGraphs.py --file '+file
      if (not f_printOnly):
        calculateFinalMJBGraphs.calculateFinalMJBGraphs(file)


if(f_plotRelevant):

  ## plotNominal.py ##
  files = glob.glob(args.workDir+'/*MJB_initial.root')+glob.glob(args.workDir+'/*DoubleMJB_initial.root')
  if(doFinal):
    files += glob.glob(args.workDir+'/*MJB_final.root')
    #files += glob.glob(args.workDir+'/*all.MJB_final.root')+glob.glob(args.workDir+'/*.DoubleMJB_final.root')
  for file in files:
    print 'python MultijetBalance/scripts/plotting/plotNominal.py -b --file '+file
    if (not f_printOnly):
      os.system('python MultijetBalance/scripts/plotting/plotNominal.py -b --file '+file)
    #f_plotSys = False;
    #f_addGagik = False
    #plotNominal.plotNominal(file, f_plotSys, f_addGagik);

  if(doMC and doJZSlices):
    files = sorted(glob.glob(args.workDir+'/*JZ*.scaled.root'))
    files = ','.join(files)
    if ( isValidation ):
      files += ' --validation'
    if (is2016):
      files += ' --is2016'
    print 'python MultijetBalance/scripts/plotting/combinedPlotNominal.py -b --files '+files
    if (not f_printOnly):
      os.system('python MultijetBalance/scripts/plotting/combinedPlotNominal.py -b --files '+files)
    print 'python MultijetBalance/scripts/plotting/combinedPlotNominal.py -b --normalize --files '+files
    if (not f_printOnly):
      os.system('python MultijetBalance/scripts/plotting/combinedPlotNominal.py -b --normalize --files '+files)

  if(doMC and doData):
    if( not doFit ):
      files = sorted(glob.glob(args.workDir+'/*.MJB_initial.root'))
      if len(files) > 0:
        files = ','.join(files)
        if ( isValidation ):
          files += ' --validation'
        if (is2016):
          files += ' --is2016'
        command = 'python MultijetBalance/scripts/plotting/combinedPlotNominal.py -b --ratio --files '+files
        print command
        if (not f_printOnly):
          os.system(command)

    if(doFit):
      files = sorted(glob.glob(args.workDir+'/*.fit_MJB_initial.root'))
      if len(files) > 0:
        files = ','.join(files)
        if ( isValidation ):
          files += ' --validation'
        if (is2016):
          files += ' --is2016'
        command = 'python MultijetBalance/scripts/plotting/combinedPlotNominal.py -b --ratio --files '+files
        print command
        if (not f_printOnly):
          os.system(command)

    if(doFinal):
      files = sorted(glob.glob(args.workDir+'/*.MJB_final.root'))
      if len(files) > 0:
        files = ','.join(files)
        if ( isValidation ):
          files += ' --validation'
        if (is2016):
          files += ' --is2016'
        command = 'python MultijetBalance/scripts/plotting/combinedPlotNominal.py -b --ratio --files '+files
        print command
        if (not f_printOnly):
          os.system(command)

###  ## plotMJBvEta.py ##
###  files = glob.glob(args.workDir+'/*.MJB_initial.root')+glob.glob(args.workDir+'/*.DoubleMJB_initial.root')
###  if(doFinal):
###    files += glob.glob(args.workDir+'/*.MJB_final.root')+glob.glob(args.workDir+'/*.DoubleMJB_final.root')
###  for file in files:
###    command = 'python MultijetBalance/scripts/plotting/plotMJBvEta.py -b --file '+file+' --binnings '+binnings
###    print command
###    os.system(command)
###    #plotMJBvEta.plotMJBvEta(file, binnings)
#
  if(doSys):
    ## plotSysRatios.py ##
    files = glob.glob(args.workDir+'/*.fit_MJB_initial.root')+glob.glob(args.workDir+'/*.Fit_DoubleMJB_initial.root')
    for file in files:
      command = 'python MultijetBalance/scripts/plotting/plotSysRatios.py -b --file '+file
      print command
      if (not f_printOnly):
        os.system(command)
      #plotSysRatios.plotSysRatios(file)

    ## plotActualSysRatios.py ##
    files = glob.glob(args.workDir+'/*.Fit_DoubleMJB_sys_initial.root')
    for file in files:
      command = 'python MultijetBalance/scripts/plotting/plotActualSysRatios.py -b --file '+file
      print command
      if (not f_printOnly):
        os.system(command)


if(f_plotAll):
  ## plotAll.py ##
  files = glob.glob(args.workDir+'/*.scaled.root') #+glob.glob(args.workDir+'/*MJB*.root')
  for file in files:
    command = 'python MultijetBalance/scripts/plotting/plotAll.py -b --file '+file
    print command
    if (not f_printOnly):
      os.system(command)
    ##plotAll.plotAll(file)

  files = glob.glob(args.workDir+'/*.scaled.root') #+glob.glob(args.workDir+'/*MJB*.root')
  if len(files) > 0:
    files = ','.join(files)
    command = 'python MultijetBalance/scripts/plotting/combinedPlotNominal.py -b --normalize --ratio --files '+files
    print command
    if (not f_printOnly):
      os.system(command)

#if(f_plotSL):
#  ## plotSL.py ##
#  files = glob.glob(args.workDir+'/*.appended.root')
#  for file in files:
#    print 'python MultijetBalance/scripts/plotting/plotSL.py -b --file '+file+' --binning '+binning
#    plotSL.plotSL(file, binning)
#
#  ## plotSLDataMCRatio.py ##
#  if(doData and doMC):
#    dataFile = glob.glob(args.workDir+'/*data.all.appended.root')[0]
#    mcFile = glob.glob(args.workDir+'/*mc.all.appended.root')[0]
#    print 'python MultijetBalance/scripts/plotting/plotSLDataMCRatio.py --mcFile '+mcFile+' --dataFile '+dataFile+' --binning '+binning
#    plotSLDataMCRatio.plotSLDataMCRatio(dataFile, mcFile, binning)
#
