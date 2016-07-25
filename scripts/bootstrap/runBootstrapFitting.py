#!/usr/bin/env python

#######################################################################################
# runBootstrapFitting.py
#######################################################################################
# This is a script for running runFit or runBootstrapFit on MJB files.
# It runs the fits in parallel on each systematic variation.
# It also combines the output into one file.
#######################################################################################
# jeff.dandoy@cern.ch
#######################################################################################


import subprocess, os, time, sys, glob
import datetime, argparse
import ROOT

def runBootstrapFitting(inFile, f_rebin, f_fit, sysType):

  pids = []
  logFiles = []
  NCORES = 6
  if not os.path.exists("parallelLogs/"):
    os.makedirs("parallelLogs/")

  #######   Setup the list of commands  ######

  sysList = []

  thisFile = ROOT.TFile.Open(inFile)
  for key in thisFile.GetListOfKeys():
    if key.GetName().endswith("_1"):
      sysList.append( '_'.join( key.GetName().split('_')[1:-1] ) )

  if len(sysType) > 0:
    sysList = [sys for sys in sysList if sysType in sys]

  ## For each systematic ##
  for iS, sys in enumerate(sysList):
    while len(pids) >= NCORES:
      wait_completion(pids, logFiles)

    logFile = "parallelLogs/runLog_"+str(iS)+'.txt'

    if (f_rebin):
      command = "runBootstrapRebin --file "+inFile+" --sysType "+sys
      command += " --upperEdge 2000 --threshold 2"
      if( f_fit ):
        command += ' --fit'
    else:
      command = "runFit --file "+inFile+" --sysType "+sys
      command += " --upperEdge 2000"
      if( f_fit ):
        command += ' --fit'

    print command
    res = submit_local_job( command, logFile )
    pids.append(res[0])
    logFiles.append(res[1])

  wait_all(pids, logFiles)

  ##### Combine seperate outputs #####
  if (f_rebin):
    histType = "significant"
  else:
    histType = "mean_MJB_initial"

  fitDir = os.path.dirname(inFile)+'/'
  if not os.path.exists(fitDir+'initialFits/'):
    os.makedirs(fitDir+'initialFits/')
  os.system('mv '+fitDir+'hist*'+histType+'*root.* '+fitDir+'initialFits/')
  ## Hadd output together into 1 file
  toHadd = glob.glob(fitDir+'initialFits/hist*'+histType+'*root.*')
  os.system('hadd -f '+fitDir+'hist.data.all.'+histType+'.root '+' '.join(toHadd) )

#### Code for running parallel jobs ###

def submit_local_job(exec_sequence, logfilename):
  output_f=open(logfilename, 'w')
  print "Executing ", exec_sequence
  pid = subprocess.Popen(exec_sequence, shell=True, stderr=output_f, stdout=output_f)
  time.sleep(0.1)  #Wait to prevent opening / closing of several files

  return pid, output_f

def wait_completion(pids, logFiles):
  print """Wait until the completion of one of the launched jobs"""
  while True:
    for pid in pids:
      if pid.poll() is not None:
        print "\nProcess", pid.pid, "has completed"
        logFiles.pop(pids.index(pid)).close()  #remove logfile from list and close it
        pids.remove(pid)

        return
    print ".",
    sys.stdout.flush()
    time.sleep(3) # wait before retrying

def wait_all(pids, logFiles):
  print """Wait until the completion of all remaining launched jobs"""
  while len(pids)>0:
    wait_completion(pids, logFiles)
  print "All jobs finished!"

#### Main #####
if __name__ == "__main__":
  beginTime = datetime.datetime.now().time()

  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("-rebin", dest='rebin', action='store_true', default=False, help="Rebin the histograms, for the final stage")
  parser.add_argument("-fit", dest='fit', action='store_true', default=False, help="Fit for the balance, rather than taking the average")
  parser.add_argument("--file", dest='file', default="", help="Input file name")
  parser.add_argument("--sysType", dest='sysType', default="", help="Run only on the given sysTypes")
  args = parser.parse_args()

  runBootstrapFitting(args.file, args.rebin, args.fit, args.sysType)
  print "Starting fitting at", beginTime
  print "Finished fitting at", datetime.datetime.now().time()


