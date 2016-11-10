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


Full Instructions
^^^^^^^^^^^^^^^^^

You need a new JESUncertainties config and root file from Kate Pachal. The JetUncertainties package must be downloaded
and these files placed in the corret place.

A few edits need to be made to the config file:
change "UncertaintyRootFile" to point to the new ROOT file.
Change the JESComponent numbering so that there are no numbers skipped!  We need this because we use the JESComponent number
to get an index.
Previously this only required chaning 100, 101, and 102 to lower numbers.
Also the name "LAr_Esmear" should be switched to "Gjet_GamEsmear".

First stage of running:
Run MC stages 0 and 1, because each stage is independent of the previous results. Switch stages with only m_MJBIteration.
Run data stage 0 and bootstrap stage 0.  Bootstrap only requires changing m_bootstrap and m_systTool_nToys. 

Submit jobs to grid using runGridSubmission.py
Download jobs with downloadAndMerge.py
Place all output into same directory, with hist\, tree\, SystToolOutput, etc.
Run plotting code on this directory, i.e. python MultijetBalance/scripts/plotting/transform.py --dir path/to/files/
mv double MJB file, hist.combined.Pythia.Fit_DoubleMJB_initial.root , to MultijetBalance/data/ to use as input for next iteration
Just set in config file "m_MJBIteration" : 1 and "m_MJBCorrectionFile" 


Trigger Efficiency Cutoff
-------------------------

Each trigger should be used only in a region of full efficiency wrt recoil pt.
These values are set via the m_triggerAndPt config variable, as described above.
The cutoff files can be calculated using MultijetBalance/scripts/binning/checkTrigger.py.
This calculates the efficiency of a trigger wrt a reference supporting trigger, and find the point at which it's 99.5% efficient.

This code first creates histograms of the efficiencies using::
  
  python checkTrigger.py --calculate --data --file path/to/file.root

Use the ``--mc`` option if running on MC.
Use the ``--nevents`` option if you'd only like to run on a subset of the events (10 million should be plenty).
An optional output tag can be added with the ``--outName`` option.
Use the ``-dir`` option to run over a directory with several files.
Before using this file, the first few lines of checkTrigger() should be edited with the triggers to use, in descending order.
This code will create an output directory called ``triggerPlots`` in the same directory as the input file.

After calculating the efficiencies with ``--calculate``, the efficiencies may be plotted with::

  python checkTrigger --plot --file path/to/file.root

Trigger are calculated using one of three methods.
Here the trigger in question is ``higher``, and ``lower`` is the next lowest trigger.
Unprescaled method should be used only for the first unprescaled trigger, and uses the logic ``(higher && lower) / lower``.
This should not be used for prescaled triggers, as there is no guarantee an event triggering ``higher`` will also trigger ``lower``.
For the prescaled method, the logic is instead ``higher / (higher || lower)``.
A third method, unbiased, is only to be used for MC when no trigger requirement is made on the TTree events.
The method is ``higher/nevents``, calculating the number of all events that pass the ``higher`` trigger.
All methods are calculated by default.

Binning
-------

The binning in recoil pt should be chosen so that enough statistics are in each bin.
In general, 10,000 events should be used in each bin, though at higher recoil pt this will be lower.

First a histogram with fine binning (1 GeV) must be created.
This is done with ::

  python getBinning.py --fineHist --file path/to/file.root

This file is stored in a new directory, ``binningPlots/``, stored in the same directory as the input file.
The binning can then be calculated using::

  python getBinning.py --calcBin --file path/to/file.root

Here the beginning of ``getBinning.py`` should first be edited with the ``eventThreshold``, ``numRequiredBins``, and ``calcBinEdges``.
``eventThreshold`` is the number of required events in the bin, nominally set to 10,000.
``numRequiredBins`` is the minimum width of the bins, in GeV.
Once not enough events are in a bin, this value is doubled permanently for all subsequent bins.
``calcBinEdges`` is a list containing at least the first bin edge, as well as other bin edges the user is requiring.
New bins will only be calculated after the last entry in ``calcBinEdges``.

The new bin edges will be printed to screen, and an output plot will be made as BinningCalculation.png.


Iterative thresholds
--------------------

The MJB is an iterative procedure, and at each stage events can only be used if the subleading jet pt is below a certain threshold.
This first threshold is set entirely by the reach of the input V+jet calibrations, but the subsequent thresholds 
are set by the statistics.
The number of events in each bin after requring a specific subleading jet pt cut can be found with::
  
  python getBinning.py --iterHist --file path/to/file.root

The beginning of this file should first be edited with the ``iterativeEdges`` and ``iterativeCutoffs``.
Here ``iterativeEdges`` are the bin edges determined in the previous step,
and ``iterativeCutoffs`` are the subleading jet pt cutoffs to test.
These cutoffs should be identical to one of the bin edges, as it defines the point at which the previous calibration
can be used for the next iteartion.
These results are saved to IterativeBinningHist.root in the ``binningPlots/`` directory.

To plot the distributions after various pt cuts, use::

  python getBinning.py --plotIter --file path/to/file.root

The statistics in each bin after a subleading jet pt cut can be seen, and the iterative cuts can be determined.



