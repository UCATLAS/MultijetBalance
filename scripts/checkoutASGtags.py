#!/usr/bin/python
#
# *******************************************************************************
# script to checkout and patch extra boostrap and fitting packages
#
# Usage (from the directory where ASG release has been set up):
#  python MultijetBalance/scripts/checkoutASGtags.py RELEASE_NUMBER [X.Y.Z]
# *******************************************************************************

import os, subprocess
import argparse

rc_env = os.environ.copy()

parser = argparse.ArgumentParser(description='Checkout packages and apply patches', usage='%(prog)s version')
parser.add_argument('version', help='the ASG release you have set up')
args = parser.parse_args()

print "Using ASG version {0}".format(args.version)

dict_pkg = {
            '2.4.7': [],
            '2.4.8': [],
            '2.4.17': [],
            '2.4.18': [],
            '2.4.22': []
           }

try:
  packages_to_checkout = dict_pkg[args.version]
except KeyError:
  print "Warning: that version isn't supported! This may not be a problem if you're using a new ASG release."
  import sys
  sys.exit(0)


packages_to_checkout.append("atlas-gulefebv/SystTool/tags/SystTool-00-01-06")
packages_to_checkout.append("atlasphys-sm/Physics/StandardModel/Common/BootstrapGenerator/tags/BootstrapGenerator-01-10-00")
packages_to_checkout.append("atlasperf/CombPerf/JetETMiss/JetCalibrationTools/JES_ResponseFitter/tags/JES_ResponseFitter-00-02-00")
packages_to_checkout.append("atlasoff/Reconstruction/Jet/JetAnalysisTools/JetTileCorrection/tags/JetTileCorrection-00-00-05/")



if len(packages_to_checkout) == 0:
  print "No packages needed for version ", args.version
else:
  for pkg in packages_to_checkout:
    print "Checking out package: {0}".format(pkg)
    subprocess.Popen(['cd $ROOTCOREBIN/.. && pwd && rc checkout_pkg {0}'.format(pkg) ], env=rc_env, shell=True).wait()

###  Apply Local Patches ###
packages_to_patch = ["JES_ResponseFitter", "SystTool"]
print "Applying svn patches..."
for pkg in packages_to_patch:
  if any(pkg in pkgs for pkgs in packages_to_checkout):
    print "  patching {0}".format(pkg)
    subprocess.Popen(['cd $ROOTCOREBIN/../{0} && pwd && patch -p0 -i $ROOTCOREBIN/../MultijetBalance/data/{0}.diff && cd - && pwd'.format(pkg)], env=rc_env, shell=True ).wait()
  else:
    print "  no patches to be applied!"

