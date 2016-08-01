.. _Instructions:

Instructions
=======================

Once the environment is set up (:ref:`Installing`), the code is run through the ``xAODAnaHelpers/scripts/xAH_run.py`` command.
xAH_run.py allows for many run options, and requires a driver option at the end of the command. Those of primary interest for the MJB are:

 * --files : A list of files to run over
 * --config : A configuration file for all algorithms
 * --submitDir : Output and submission directory
 * --force : Replace an existing output directory
 * direct : The driver which allows for local running

For example::

    ./xAODAnaHelpers/scripts/xAH_run.py -f --files path/to/input/file.root --config MultijetBalance/data/config_MJB.py direct

Configuration File
------------------
An example configuration file can be found at MultijetBalance/data/config_MJB.py.
It firsts sets the options for BasicEventSelection, and then for MultijetBalanceAlgo.
Some MJB configurations are also used for setting the jet calibration, cleaning, uncertainty, and JVT options.
Relevant MJB Configurations are:

 * m_binning : The binning of the recoil pt
 * m_MJBIteration : Choose which iteration of MJB you want to run
 * m_MJBIterationThreshold : Choose a list of subleading jet pt cutoffs for each iteration
 * m_MJBCorrectionFile : Choose the file which has the results from the previous MJB iteration (Nothing needed for Iteration 0)
 * m_triggerAndPt : A list of triggers and their recoil system pt cutoffs
 * m_sysVariation : The systematic variations to run (Nominal for none, AllSystematics for all)
 * m_writeNominalTree : Output the TTree for the nominal result only

There are also several modes.
The ``Validation mode`` allows you to apply the insitu calibrations to the leading jet, which is normally calibrated only up to etaJES.
``Validation mode`` allows you to check the performance of the jet calibration for high-pt jets.
This should only be done for a single iteration (0).
This mode requires the following options:

 * m_leadingInsitu : Apply the insitu calibration to the leading jet
 * m_noLimtJESPt : Apply calibrations to jets of any pt
 * m_MJBIterationThreshold : Set to a very large number (i.e. "9999" GeV)

The ``Bootstrap mode`` allows you to save toys to calculate the statistical correlations between systematic uncertainties, and is complemented by the scripts in MultijetBalance/scripts/bootstrap/ .
Toys are saved in a unique output file called SystToolOutput.
For further iterations of MJB in ``bootstrap mode``, histograms will be created without the normal TDirectory structure to allow for proper hadding of the many output histograms.
This mode requires the following options:

 * m_bootstrap : Turn on Bootstrap Mode
 * m_sysTool_nToys : Set the number of toys saved for each systematic

Output
------
In general the Multijet Balance will use output histograms for observable calculation and performance checks.
Minitrees may also be written for further checks.
Histograms may be directly created during the analysis using MultijetHists.cxx, and will be found in hist-FILENAME.root in the output directory (submitDir).
Minitree creation and variables are defined in MiniTree.cxx

Analysis Cutflow
----------------

Two cutflow histograms (one weighted, one unweighted) exist across all algorithms.
In a given algorithm it may be retrieved in the histInitialize() function, and it may be found in the output directory submitDir/hist-cutflow/.
For each selection a bin is added using the FindBin(NAME) function, which will automatically append the NAME and return the bin position, which is saved to the m_cutflowBins vector.
During the event flow the respective bin is incremented after the selection, and a single integer iterator (iCutflow) keeps track of the bin position.

Grid Submission
---------------
Grid submission is run by directly calling xAH_run.py on a .txt file including a list of DQ2 containers.
This must be run from the top directory.
It will run over all container names listed in the text file (i.e. see MultijetBalance/scripts/list_sample_grid.txt).

This process may be simplified for running over several containers through the use of the submission script MultijetBalance/scripts/runGridSubmission.py.
As each grid submission must have a unique name, the extraTag string located at the top of run_grid.py must be varied for each attempted run.
The driver should be chosen to be the 'grid', and production_name should be set to an empty string.
Several text files may be chosen by the files list, as well as unique outputTags names.

Plotting Scripts
----------------

The output of MultijetBalanceAlgo may be manipulated to create final histograms through the scripts in MultijetBalance/scripts/plotting/.
All scripts are called in order through transformMJBHists.py.
The only option for transformMJBHists.py is

 * --dir : Directory where relevant files from downloadAndMerge.py can be found, and where output will be placed.

The steps are to be run in order, and it is recommended that each first be run independently to ensure they are working properly.
The steps include:

 * f_getPtHist : (Optional) Derive a histogram of average leadingJetPt for each recoilPt bin, which is used to set the x-axis of the "final" histograms
 * f_scale : Scale the MC JZ* slices to their proper cross sectino
 * f_combine : combine all scaled MC into one file, and combine all data into one file
 * f_calculateMJB : Calculate the MJB balance plot, as well as extra observables
 * f_plotRelevant : Plot the most relevant observables
 * f_plotAll : Plot all observables
 * f_plotSL : Plot extra information on the energy deposited in each sampling layer

Several options may also be set:

 * doData : Run on data
 * doMC : Run on MC
 * mcTypes : A list of different MC generators to run over, and which all will be plotted. For final balance results the first MC is used as default, the second MC is used as a systematic variation, and all subsequent MC's are ignored.
 * doSys : Also calculate the systematic histograms (set False for just nominal)
 * doJZSlices : Plot distributions of individual JZ slices compared against each other
 * doAverage : Calculate the average balance in each recoilPt bin based on a mean. This is faster but less accurate than fitting
 * doFit : Calculate the avearge balance based on fits. This can be slower, but is required for final results
 * doNominalOnly : Fit only the nominal distribution
 * endPt : Require the last bin to end at this pt
 * doFinal : Create the final TGraphErrors based on x-axis from f_getPtHist
 * doBootstrap : Combine bins into statistically relevant binning based on previous results from the bootstrap mode
 * rebinFile : The file with the bootstrap determined rebinning to use.

Bootstrap Plotting
^^^^^^^^^^^^^^^^^^
Bootstrap procedes similar to the regular plotting scripts, with scripts found in MultijetBalance/scripts/boostrap
and driven by transformBootstrap.py.
Two different kinds of iterations can be performed.

During the first iterations the calibration results are saved in a special SystOutput data format, and must be
transformed into calibration histograms with the --firstIter option.
The number of calibration histograms is proportional to the number of toys used in the bootstrap procedure.
Output histograms are therefore not stored in TDirectories but directly to the ROOT TFile, facilitating the merging of files.

During subsequent iterations the calibration results are saved in output histograms
that may be manipulated with the --lastIter option.
Again the TDirectory structure is not used to faciliate file merging.
Several steps are performed:

 * The first step will hadd the seperate files together
 * The second step will reformat the file so that the TDirectory structure is included.
 * The third step will find the mean and RMS of all the toys for each systematic using runBootstrapFitting.py.  This calls runBootstrapRebin on each systematic individually to allow parallelization and to speed things up.


The final output is saved to "hist.*.*.RMS.root"

::
  python MultijetBalanceAlgo/scripts/bootstrap/transformBootstrap.py --rebin --lastIter --dir gridOutput/


