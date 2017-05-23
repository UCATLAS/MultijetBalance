## Jeff Dandoy ##

import glob, os
import argparse, itertools

import MJBSample, calcMJB
import plotter

def main():

  m_printOnly = False
  #m_workDir = "../../../gridOutput/MJBarea/Rel21_May19/"
  m_workDir = "gridOutput/MJBarea/LC0/"


  #make files dir  
  
  
  # Do not run commands, but print them in the order they are applied
  
  m_outputIntermediary = False
  
  ## Create samples & append them to the sample list
  sample_list = []
  
#  sample_list.append( MJBSample.DataSample(tag="Rel21") )
#  sample_list.append( MJBSample.DataSample(tag="Rel207") )

  sample_list.append( MJBSample.DataSample(tag="Main") )
  sample_list.append( MJBSample.MCSample(tag="Pythia") )
  sample_list.append( MJBSample.MCSample(tag="Sherpa") )


  ### Gather input files for each sample ###
  for sample in sample_list:
    sample.setInputDir( m_workDir )
    sample.gatherInput()

  ### Add MC systematic for MC samples - corresponds to difference b/w MC samples
  for nomSample, systSample in itertools.product(sample_list, sample_list):
    if nomSample != systSample and nomSample.isMC and systSample.isMC:
      MJBSample.addMCSyst(nomSample, systSample)


  for sample in sample_list:
    calcMJB.profileBalance(sample)
    calcMJB.fitBalance(sample)

#  plotter.plotHists(sample_list)


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
  parser.add_argument("--dir", dest='workDir', default='gridOutput/VF_EM_Nov15_workarea/', help="Typically gridOutput")
  args = parser.parse_args()

  main()
  print "Done"
