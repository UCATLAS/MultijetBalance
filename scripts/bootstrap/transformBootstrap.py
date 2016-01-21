## Jeff Dandoy ##


import glob, os
import argparse

parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
parser.add_argument("--firstIter", dest='firstIter', action='store_true', default=False,
    help="Inputs are for the first iteration: bootstrap objects which will be transformed into MJB correction histograms")
parser.add_argument("--lastIter",  dest='lastIter',  action='store_true', default=False,
    help="Inputs are for the last iteration: Output histograms which must be combined to determine the final rebinning")
parser.add_argument("--fit",  dest='fit',  action='store_true', default=False,
    help="Systematic histograms will be fit, rather than using the average balance value.  This is not recommended and "\
    "will take a substantial amount of time.")
parser.add_argument("--rebin",  dest='rebin',  action='store_true', default=False,
    help="Perform rebinning at the last iteration")
parser.add_argument("--dir", dest='workDir', default='', help="Typically gridOutput")
args = parser.parse_args()

f_printOnly = True

if not args.firstIter and not args.lastIter:
  print "Error, neither --firstIter nor --lastIter was chosen.  The bootstrap procedure must be chosen"
  exit(1)

#First Iteration
f_combine = False
f_fit = True

#Last Iteration
f_combine = False
f_reformat = False
f_calculateRebin = False


args.workDir = args.workDir+'/workarea/'

if not os.path.exists(args.workDir+'/initialFiles'):
  os.makedirs(args.workDir+'/initialFiles')


if (args.firstIter):
  if( f_combine ):

    ##  Grab all bootstrap histograms ##
    ##  ~ 12 minutes
    command = 'hadd '+args.workDir+'/bootstrap.data.bootstrap.initial.root '+args.workDir+'/../SystToolOutput/*.data*_SystToolOutput.root'
    print command
    if not f_printOnly:
      os.system(command)

    ## Create regular 2D histograms from all bootstrap toys and from nominal
    # ~ 2 minutes, bootstrap.data.bootstrap.initial.root -> hist.data.bootstrap.scaled.root
    command = 'runBootstrapHistogrammer --file '+args.workDir+'/bootstrap.data.bootstrap.initial.root'
    print command
    if not f_printOnly:
      os.system(command)


    ## Grab all nominal histograms ##
    ##  !! This file can be directly copied from the nominal plotting results ##
    # ~ 6  minutes
    command = 'hadd '+args.workDir+'/hist.data.nominal.initial.root '+args.workDir+'/../hist/*.data*_hist.root'
    print command
    if not f_printOnly:
      os.system(command)

    ## Strip unnecessary histograms from the nominal ##
    ## ~ 0 minutes,  hist.data.nominal.initial.root -> hist.data.nominal.scaled.root
    command = 'cp '+args.workDir+'/hist.data.bootstrap.scaled.root '+args.workDir+'/hist.data.all.scaled.root'
    print command
    if not f_printOnly:
      os.system(command)

    command = 'python MultijetBalanceAlgo/scripts/bootstrap/addNominalToBootstrap.py'
    command += ' --nomFile '+args.workDir+'/hist.data.nominal.initial.root'
    command += ' --bsFile '+args.workDir+'/hist.data.all.scaled.root'
    print command
    if not f_printOnly:
      os.system(command)

  if( f_fit ):
    command = 'python MultijetBalanceAlgo/scripts/bootstrap/runBootstrapFitting.py --file '+args.workDir+'/hist.data.all.scaled.root'
    print command
    if not f_printOnly:
      os.system(command)

elif (args.lastIter):
  if(f_combine):
      command = 'hadd '+args.workDir+'/hist.data.all.histOnlyFormat.root '+args.workDir+'/../hist/*.data*_hist.root'
      print command
      if not f_printOnly:
        os.system(command)
        print "Finished hadding"

  if(f_reformat):
      command = 'python MultijetBalanceAlgo/scripts/bootstrap/reformatHists.py --file '+args.workDir+'/hist.data.all.histOnlyFormat.root'
      print command
      if not f_printOnly:
        os.system(command)

  if( f_calculateRebin ):
      command = 'python MultijetBalanceAlgo/scripts/bootstrap/runBootstrapFitting.py --file '+args.workDir+'/hist.data.all.scaled.root'
      if (args.fit):
        command += ' -fit'
      if (args.rebin):
        command += ' -rebin'
      print command
      if not f_printOnly:
        os.system(command)

