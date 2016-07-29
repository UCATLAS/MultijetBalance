# MultijetBalance

Bootstrap Functionality



Final iteration:
MultijetBalanceAlgo/scripts/bootstrap/transformBootstrap.py

--lastIter:
The output of the last iteration will be histograms without a TDirectory structure
Including a TDirectory structure greatly increases hadd time, so it is not used.

The first step will hadd the seperate files together
The second step will reformat the file so that the TDirectory structure is included.
The third step will find the mean and RMS of all the toys for each systematic using runBootstrapFitting.py
This calls runBootstrapRebin on each systematic individually to allow parallelization and to speed things up.
The output is saved to "hist.*.*.RMS.root"

python MultijetBalanceAlgo/scripts/bootstrap/transformBootstrap.py --rebin --lastIter --dir gridOutput/

