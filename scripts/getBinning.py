import ROOT, array
import math
import time
import argparse

#parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
#parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
#parser.add_argument("--dataFile", dest='dataFile', default="submitDir/hist-data12_8TeV.root",
#         help="Input file name")
#parser.add_argument("--mcFile", dest='mcFile', default="submitDir/hist-data12_8TeV.root",
#         help="Input file name")
#args = parser.parse_args()

f_getBinning = True

#f_getBinning = True
#binName = "3p"

fileName = "../../gridOutput/Trigger_VA_EM_Nov23/workarea/DataTrigger/allData.root"
treeName = "outTree_Nominal"
branchName = "recoilPt"
#1p = 10000, 3p = 1111, 5p = 400
eventThreshold = 1111

triggers = ["HLT_j360","HLT_j260","HLT_j200","HLT_j175"]
trigEffs = [500       ,400       ,350       ,300       ]
#triggers = ["HLT_j360","HLT_j260","HLT_j200","HLT_j175","HLT_j150","HLT_j110"]
#trigEffs = [500       ,400       ,350       ,300       ,250       ,200]

binName = "Fine"

inFile = ROOT.TFile.Open(fileName, "READ")
tree = inFile.Get(treeName)

numEntries = tree.GetEntries()
tree.SetBranchStatus('*', 0)
b_pt = array.array('f',[0])
tree.SetBranchStatus( branchName, 1)
tree.SetBranchAddress( branchName, b_pt)
#weight = array.array('f',[0])
#tree.SetBranchStatus( "weight", 1)
#tree.SetBranchAddress( "weight", weight)

passedTriggers =   ROOT.std.vector('string')()
tree.SetBranchStatus( "passedTriggers", 1)
tree.SetBranchAddress( "passedTriggers", passedTriggers)

### Calculate binning ###
if (f_getBinning):

  count = 0
  pts = []
  print "Running on " , numEntries, "entries"
  while tree.GetEntry(count):
    count += 1
#    if count > 100000:
#      continue
    if count%1e5 == 0:  print count

    for iT, trigEff in enumerate(trigEffs):
      if b_pt[0] > trigEff:
        if triggers[iT] in passedTriggers:
          pts.append( b_pt[0]/1e3 )
#          ptsWeighted.append( b_pt[0]/1e3, weight[0] )
        break #only check triggers once

  pts = sorted(pts)
  pts = [pt for pt in pts if pt >= trigEffs[-1] ]

  ## Get Settled Trigger Bins
  numEvents = []
  binEdges = [300, 350, 400, 450, 500]
  for iBin in range(1, len(binEdges) ):
    thisEdge = binEdges[iBin]

    iNBin = next(pt[0] for pt in enumerate(pts) if pt[1] > thisEdge)
    numEvents.append(  len(pts[0:iNBin]) )
    print pts[iNBin], len(pts[0:iNBin])
    pts = pts[iNBin:-1]

  ## Get new bins
  increase = 50
  while( len(pts) > 0 ):
    if not any( pt > binEdges[-1]+increase for pt in pts):
      iNBin = len(pts) - 1
    else:
      iNBin = next(pt[0] for pt in enumerate(pts) if pt[1] > binEdges[-1]+increase)
    print pts[iNBin], len(pts[0:iNBin])
    if len(pts[0:iNBin]) >= eventThreshold or iNBin == len(pts)-1:
    #if len(pts[0:iNBin]) >= 10000 or iNBin == len(pts)-1:
      binEdges.append( binEdges[-1]+increase )
      numEvents.append( len(pts[0:iNBin]) )
      print "----------continuing or done"
      pts = pts[iNBin:-1]
      continue
    else:
      increase += 50

### Draw Histogram ###

hist = ROOT.TH1F("ptHist", "ptHist", len(binEdges)-1, array.array('f', binEdges) )

count = 0
pts = []
while tree.GetEntry(count):
  hist.Fill(b_pt[0]/1e3)
  count += 1

print binName
print "Edges:", binEdges
print "Content:",
for iBin in range(1, hist.GetNbinsX()+1):
  print hist.GetBinContent(iBin),
print ""

c1 = ROOT.TCanvas()
hist.Draw()
c1.SetLogy()
c1.SaveAs("BinCalc_"+binName+".png")
