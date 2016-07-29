.. _Installing:
Installing
==========

The MJB package may be retrieved with::

    git clone https://github.com/UCATLAS/MultijetBalance


Specifc tags or commit can be retrieved through::

    cd MultijetBalance/
    git checkout tags/XX-YY-ZZ
    ( or git checkout ####### )
    cd ../

A full list of tags be found through ``git tag``.

.. note::

    The https version of MultijetBalance is only used to run the code, and may not be used for developement.
    To retrieve the development version requires a github account `with generated ssh-key pairs <https://help.github.com/articles/generating-ssh-keys/>`.
    You can then clone over ssh instead of https::

      git clone git@github.com:UCATLAS/MultijetBalance

Dependencies
------------

The MJB package requires the following dependencies::

    git clone https://github.com/UCATLAS/xAODAnaHelpers
    svn co svn+ssh://svn.cern.ch/reps/atlasphys-sm/Physics/StandardModel/Common/BootstrapGenerator/tags/BootstrapGenerator-01-10-00 BootstrapGenerator
    svn co svn+ssh://svn.cern.ch/reps/atlas-gulefebv/SystTool/tags/SystTool-00-01-06 SystTool
    svn co svn+ssh://svn.cern.ch/reps/atlasperf/CombPerf/JetETMiss/JetCalibrationTools/JES_ResponseFitter/tags/JES_ResponseFitter-00-02-00

Specific versions of the JetCalibTools and JetUncertainties packages may also be required, depending upon the version of the ASG's Analysis Base.

Patches to SystTool and JES_ResponseFitter are also required, and may be applied with the checkoutASGtags tool::
    python MultijetBalance/scripts/checkoutASGtags.py $ABV

where $ABV is the current Analysis Base version.

Environment
-----------

To set up the environment requires::
    setupATLAS
    rcSetup Base,X.Y.Z
    rc find_packages

If the environment was previously set, it may be retrived instead with::
    setupATLAS
    rcSetup
    rc find_packages

After making any changes, recompile from the top directory with::
    rc compile

The framework may then be run by calling::
    ./xAODAnaHelpers/scripts/xAH_run.py
with the appropriate options

Updating changes
----------------

To update new changes that exist on the ``remote`` location, pull changes with::

    git pull --rebase

