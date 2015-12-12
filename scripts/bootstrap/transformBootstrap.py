## Jeff Dandoy ##

import glob, os
import argparse

parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
parser.add_argument("--dir", dest='workDir', default='', help="Typically gridOutput")
args = parser.parse_args()

f_printOnly = False


#First iteration will assume a bootstrap object as input.  Otherwise we will assume propogated histograms
f_firstIteration = False

f_combine = True
f_fit = False
f_rebin = False

#f_combine = True #Combine MC files
#f_fit = True

if not os.path.exists(args.workDir+'/workarea/initialFiles'):
  os.makedirs(args.workDir+'/workarea/initialFiles')

args.workDir = args.workDir+'/workarea/'


if (f_firstIteration):
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

else:
  if(f_combine):
      command = 'hadd '+args.workDir+'/hist.data.all.histOnlyFormat.root '+args.workDir+'/../hist/*.data*_hist.root'
      print command
      os.system(command)

      command = 'python MultijetBalanceAlgo/scripts/bootstrap/reformatHists.py --file '+_args.workDir+'/hist.data.all.scaled.root'
      print command
      os.system(command)

  if( f_fit ):
      command = 'python MultijetBalanceAlgo/scripts/bootstrap/runBootstrapFitting.py --file '+args.workDir+'/hist.data.all.scaled.root'
      if (f_rebin):
        command += ' -rebin'
      print command
      os.system(command)

