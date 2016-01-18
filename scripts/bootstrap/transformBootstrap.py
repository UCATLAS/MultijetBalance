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

f_printOnly = False

if not args.firstIter and not args.lastIter:
  print "Error, neither --firstIter nor --lastIter was chosen.  The bootstrap procedure must be chosen"
  exit(1)

f_combine = False
f_reformat = False
f_calculateRebin = True


args.workDir = args.workDir+'/workarea/'

if not os.path.exists(args.workDir+'/initialFiles'):
  os.makedirs(args.workDir+'/initialFiles')


if (args.firstIter):
  if( f_combine ):
    command = 'hadd '+args.workDir+'/bootstrap.data.all.initial.root '+args.workDir+'/../SystToolOutput/*.data*_SystToolOutput.root'
    print command
    os.system(command)

    command = 'runBootstrapHistogrammer --file '+args.workDir+'/bootstrap.data.all.initial.root'
    print command
    os.system(command)

  if( f_fit ):
    command = 'python MultijetBalanceAlgo/scripts/bootstrap/runBootstrapFitting.py --file '+args.workDir+'/hist.data.all.scaled.root'
    print command
    os.system(command)

elif (args.lastIter):
  if(f_combine):
      command = 'hadd '+args.workDir+'/hist.data.all.histOnlyFormat.root '+args.workDir+'/../hist/*.data*_hist.root'
      print command
      os.system(command)
      print "Finished hadding"

  if(f_reformat):
      command = 'python MultijetBalanceAlgo/scripts/bootstrap/reformatHists.py --file '+args.workDir+'/hist.data.all.histOnlyFormat.root'
      print command
      os.system(command)

  if( f_calculateRebin ):
      command = 'python MultijetBalanceAlgo/scripts/bootstrap/runBootstrapFitting.py --file '+args.workDir+'/hist.data.all.scaled.root'
      if (args.fit):
        command += ' -fit'
      if (args.rebin):
        command += ' -rebin'
      print command
      os.system(command)

