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

def runBootstrapHistogrammer( dir ):

  pids = []
  logFiles = []
  NCORES = 3
  if not os.path.exists("parallelLogs/"):
    os.makedirs("parallelLogs/")

  outDir = dir+'/../SystToolHist/'
  if not os.path.exists(outDir):
    os.makedirs(outDir)

  #######   Setup the list of commands  ######

  files = glob.glob(dir+'/*_SystToolOutput.root')

  sysList = []

  thisFile = ROOT.TFile.Open(files[0])
  for key in thisFile.GetListOfKeys():
    if key.GetName().startswith("bootstrap"):
      sysList.append( key.GetName() )
  print sysList

  thisFile.Close()

  for iF, inFile in enumerate(files):
    for iS, inSys in enumerate(sysList):
      while len(pids) >= NCORES:
        wait_completion(pids, logFiles)

      logFile = "parallelLogs/runBootstrapHistogrammer_file"+str(iF)+"_sys"+str(iS)+'.txt'

      command = 'runBootstrapHistogrammer --file '+inFile+' --sysType '+inSys

      #print command
      res = submit_local_job( command, logFile )
      pids.append(res[0])
      logFiles.append(res[1])

  wait_all(pids, logFiles)

#!  ##### Combine seperate outputs #####
#!  command = 'hadd -f '+outDir+'/../workarea/bootstrap.data.bootstrap.initial.root '+outDir+'/*.root'
#!  print command
#!  os.system(command)

#### Code for running parallel jobs ###

def submit_local_job(exec_sequence, logfilename):
  output_f=open(logfilename, 'w')
  print exec_sequence
  pid = subprocess.Popen(exec_sequence, shell=True, stderr=output_f, stdout=output_f)
  #time.sleep(0.1)  #Wait to prevent opening / closing of several files

  return pid, output_f

def wait_completion(pids, logFiles):
  #print """Wait until the completion of one of the launched jobs"""
  while True:
    for pid in pids:
      if pid.poll() is not None:
        print "\nProcess", pid.pid, "has completed"
        logFiles.pop(pids.index(pid)).close()  #remove logfile from list and close it
        pids.remove(pid)

        return
    print ".",
    sys.stdout.flush()
    time.sleep(0.1) # wait before retrying

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
  parser.add_argument("--dir", dest='dir', default="", help="Input dir name")
  args = parser.parse_args()

  runBootstrapHistogrammer(args.dir) 
  print "Starting fitting at", beginTime
  print "Finished fitting at", datetime.datetime.now().time()


