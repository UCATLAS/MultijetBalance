## Jeff Dandoy ##

import glob, os
import argparse


parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
parser.add_argument("--dir", dest='workDir', default='gridOutput/combinedArea', help="Typically gridOutput")
args = parser.parse_args()


import getRecoilPtTree
import scaleHist
import calculateMJBHists
import calculateDoubleRatio
import calculateFinalMJBGraphs
import plotNominal
import plotSysRatios
import plotMJBvEta
import plotAll



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
f_calculateMJB = True # Calculate new histograms to be added to root file
f_plotRelevant = True # Most important Plots
f_plotAll = True # All plots
###f_plotSL = True  # Plot Sampling Layers

doData = True
doMC = True
doSys = False
doJZSlices = False

#binnings = "Fine,8TeV,Coarse"
binnings = "Fine"

if not os.path.exists(args.workDir+'/workarea/initialFiles'):
  os.makedirs(args.workDir+'/workarea/initialFiles')

args.workDir = args.workDir+'/workarea/'

if(f_getPtHist):
  treeDir = args.workDir+'/../tree/'
  if not os.path.exists( treeDir ):
    print "Warning, there is no tree directory in ", args.workDir+'/../'
    print "A pT histogram will not be created"
  else:
    print 'python multijetBalanceAlgo/scripts/plotting/getRecoilPtTree.py --pathToTrees '+treeDir
    getRecoilPtTree.getRecoilPtTree(treeDir, "outTree_Nominal")
    if (doData):
      os.system('hadd '+args.workDir+'/ptBinTree_data15.root '+treeDir+'/ptBinTree_*data15*.root')
    if (doMC):
      os.system('hadd '+args.workDir+'/ptBinTree_mc15.root '+treeDir+'/ptBinTree_*mc15*.root')



if(f_scale and doMC):
  ## input -> scaled ##
  files = glob.glob(args.workDir+'/../hist/*mc15*_hist.root')
  for file in files:
    print 'python MultijetBalanceAlgo/scripts/plotting/scaleHist.py --file '+file
    scaleHist.scaleHist(file)
  files = glob.glob(args.workDir+'/../hist/*scaled.root')
  for file in files:
    os.system('mv '+file+' '+args.workDir+'/initialFiles/')


if(f_combine):
  if( doMC ):
    ## scaled -> all*scaled ##
    print 'hadd '+args.workDir+'/hist.mc.all.scaled.root '+args.workDir+'/initialFiles/*.mc15*_hist.scaled.root'
    os.system('hadd '+args.workDir+'/hist.mc.all.scaled.root '+args.workDir+'/initialFiles/*.mc*_hist.scaled.root')
  if( doData ):
    print 'hadd '+args.workDir+'/hist.data.all.scaled.root '+args.workDir+'/../hist/*.data*_hist.root'
    os.system('hadd '+args.workDir+'/hist.data.all.scaled.root '+args.workDir+'/../hist/*.data*_hist.root')

  if(doJZSlices):
    files = glob.glob(args.workDir+'/initialFiles/*.mc15*_hist.scaled.root')
    for file in files:
      JZString = os.path.basename(file).split('.')[4].split('_')[-1]
      os.system('cp '+file+' '+args.workDir+'/hist.mc.'+JZString+'.scaled.root')


if(f_calculateMJB):
  ## scaled -> .appended and .MJB_initial ##
  files = glob.glob(args.workDir+'/hist.*.*.scaled.root')
  for file in files:
    print 'python MultijetBalanceAlgo/scripts/plotting/calculateMJBHists.py --file '+file+' --binnings '+binnings
    f_extraPlots = False
    calculateMJBHists.calculateMJBHists(file, binnings, f_extraPlots)
    #os.system('mv '+file+' '+args.workDir+'/initialFiles/')

  ## data and MC .MJB_initial -> .DoubleMJB_initial ##
  if(doData and doMC):
    dataFile = glob.glob(args.workDir+'/hist.data.all.MJB_initial.root')[0]
    mcFile = glob.glob(args.workDir+'/hist.mc.all.MJB_initial.root')[0]
    print 'python MultijetBalanceAlgo/scripts/plotting/calculateDoubleRatio.py --dataFile '+dataFile+' --mcFile '+mcFile
    calculateDoubleRatio.calculateDoubleRatio(dataFile, mcFile)

## Needs recoilPt_center

  ## .(Double)MJB_initial -> .(Double)MJB_final ##
  initialFiles = glob.glob(args.workDir+'/*_initial.root')
  for file in initialFiles:
    print 'python MultijetBalanceAlgo/scripts/plotting/calculateFinalMJBGraphs.py --file '+file+ ' --binnings '+binnings
    calculateFinalMJBGraphs.calculateFinalMJBGraphs(file, binnings)


if(f_plotRelevant):

  ## plotNominal.py ##
  files = glob.glob(args.workDir+'/*all.appended.root')+glob.glob(args.workDir+'/*all.MJB_initial.root')+glob.glob(args.workDir+'/*all.MJB_final.root')+glob.glob(args.workDir+'/DoubleMJB_initial.root')+glob.glob(args.workDir+'/DoubleMJB_final.root')
  for file in files:
    print 'python MultijetBalanceAlgo/scripts/plotting/plotNominal.py -b --file '+file
    os.system('python MultijetBalanceAlgo/scripts/plotting/plotNominal.py -b --file '+file)
    #f_plotSys = False;
    #f_addGagik = False
    #plotNominal.plotNominal(file, f_plotSys, f_addGagik);

  if(doMC and doJZSlices):
    files = sorted(glob.glob(args.workDir+'/*JZ*.appended.root'))
    files = ','.join(files)
    print 'python MultijetBalanceAlgo/scripts/plotting/combinedPlotNominal.py -b --files '+files
    os.system('python MultijetBalanceAlgo/scripts/plotting/combinedPlotNominal.py -b --files '+files)
    print 'python MultijetBalanceAlgo/scripts/plotting/combinedPlotNominal.py -b --normalize --files '+files
    os.system('python MultijetBalanceAlgo/scripts/plotting/combinedPlotNominal.py -b --normalize --files '+files)

  if(doMC and doData):
    files = sorted(glob.glob(args.workDir+'/*all.appended.root'))
    files = ','.join(files)
    print 'python MultijetBalanceAlgo/scripts/plotting/combinedPlotNominal.py -b --normalize --files '+files
    os.system('python MultijetBalanceAlgo/scripts/plotting/combinedPlotNominal.py -b --normalize --files '+files)

    files = sorted(glob.glob(args.workDir+'/*all.MJB_initial.root'))
    files = ','.join(files)
    print 'python MultijetBalanceAlgo/scripts/plotting/combinedPlotNominal.py -b --files '+files
    os.system('python MultijetBalanceAlgo/scripts/plotting/combinedPlotNominal.py -b --files '+files)

    files = sorted(glob.glob(args.workDir+'/*all.MJB_final.root'))
    files = ','.join(files)
    print 'python MultijetBalanceAlgo/scripts/plotting/combinedPlotNominal.py -b --files '+files
    os.system('python MultijetBalanceAlgo/scripts/plotting/combinedPlotNominal.py -b --files '+files)

  ## plotMJBvEta.py ##
  files = glob.glob(args.workDir+'/*all.MJB_initial.root')+glob.glob(args.workDir+'/*all.MJB_final.root')+glob.glob(args.workDir+'/DoubleMJB_initial.root')+glob.glob(args.workDir+'/DoubleMJB_final.root')
  for file in files:
    print 'python MultijetBalanceAlgo/scripts/plotting/plotMJBvEta.py -b --file '+file+' --binnings '+binnings
    os.system('python MultijetBalanceAlgo/scripts/plotting/plotMJBvEta.py -b --file '+file+' --binnings '+binnings)
    plotMJBvEta.plotMJBvEta(file, binnings)

  if(doSys):
    ## plotSysRatios.py ##
    files = glob.glob(args.workDir+'/*all.appended.root')+glob.glob(args.workDir+'/*all.MJB_initial.root')+glob.glob(args.workDir+'/DoubleMJB_initial.root')
    for file in files:
      print 'python MultijetBalanceAlgo/scripts/plotting/plotSysRatios.py -b --file '+file
      os.system('python MultijetBalanceAlgo/scripts/plotting/plotSysRatios.py -b --file '+file)
      plotSysRatios.plotSysRatios(file)


if(f_plotAll):
  ## plotAll.py ##
  files = glob.glob(args.workDir+'/*all.appended.root') #+glob.glob(args.workDir+'/*MJB*.root')
  for file in files:
    print 'python MultijetBalanceAlgo/scripts/plotting/plotAll.py -b --file '+file
    os.system('python MultijetBalanceAlgo/scripts/plotting/plotAll.py -b --file '+file)
#    plotAll.plotAll(file)

#if(f_plotSL):
#  ## plotSL.py ##
#  files = glob.glob(args.workDir+'/*.appended.root')
#  for file in files:
#    print 'python MultijetBalanceAlgo/scripts/plotting/plotSL.py -b --file '+file+' --binning '+binning
#    plotSL.plotSL(file, binning)
#
#  ## plotSLDataMCRatio.py ##
#  if(doData and doMC):
#    dataFile = glob.glob(args.workDir+'/*data.all.appended.root')[0]
#    mcFile = glob.glob(args.workDir+'/*mc.all.appended.root')[0]
#    print 'python MultijetBalanceAlgo/scripts/plotting/plotSLDataMCRatio.py --mcFile '+mcFile+' --dataFile '+dataFile+' --binning '+binning
#    plotSLDataMCRatio.plotSLDataMCRatio(dataFile, mcFile, binning)
#
