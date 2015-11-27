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

f_getHist = False
f_getBinning = True
#binName = "3p"

fileName = "../../gridOutput/Trigger_VA_EM_Nov23/workarea/DataTrigger/allData.root"
treeName = "outTree_Nominal"
branchName = "recoilPt"
#1p = 10000, 3p = 1111, 5p = 400
eventThreshold = 300

triggers = ["HLT_j360","HLT_j260","HLT_j200"]
trigEffs = [480       ,360       ,300       ]
#triggers = ["HLT_j360","HLT_j260","HLT_j200","HLT_j175"]
#trigEffs = [500       ,400       ,350       ,300       ]
#triggers = ["HLT_j360","HLT_j260","HLT_j200","HLT_j175","HLT_j150","HLT_j110"]
#trigEffs = [500       ,400       ,350       ,300       ,250       ,200]

binName = "Fine"

inFile = ROOT.TFile.Open(fileName, "READ")
tree = inFile.Get(treeName)

numEntries = tree.GetEntries()
tree.SetBranchStatus('*', 0)
jet_pt =   ROOT.std.vector('float')()
tree.SetBranchStatus( "jet_pt", 1)
tree.SetBranchAddress( "jet_pt", jet_pt)
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
if (f_getHist):

  outFile = ROOT.TFile.Open("fineBinningHist.root", "RECREATE")
  finept = ROOT.TH1F("finept","finept",400,0,4000.)

  count = 0
  print "Running on " , numEntries, "entries"
  while tree.GetEntry(count):
    count += 1
#    if count > 100000:
#      continue
    if count%1e5 == 0:  print count

    if( jet_pt[1]*1000/b_pt[0] > 0.8):
      continue

    for iT, trigEff in enumerate(trigEffs):
      if b_pt[0] > trigEff*1e3:
        if triggers[iT] in passedTriggers:
          finept.Fill( b_pt[0] / 1e3 )
#          ptsWeighted.append( b_pt[0]/1e3, weight[0] )
        break #only check triggers once
  outFile.Write()
  outFile.Close()

if (f_getBinning):

  inFile =  ROOT.TFile.Open("fineBinningHist.root", "READ")
  finept = inFile.Get("finept")
  ## Get Settled Trigger Bins
  numEvents = []
#  binEdges = [300, 360, 420, 480]
  binEdges = [300, 360, 420, 480, 540.0, 600.0, 660.0, 720.0, 780.0, 840.0, 900.0, 960.0, 1020.0, 1080.0, 1140.0, 1200.0, 1260.0, 1320.0, 1380.0, 1480.0, 1700.0, 2000.0, 2700.0]
  #binEdges = [300, 360, 420, 480, 540.0, 600.0, 660.0, 720.0, 780.0, 840.0, 900.0, 960.0, 1020.0, 1080.0, 1140.0, 1200.0, 1260.0, 1320.0, 1380.0, 1480.0, 1700.0, 2000.0, 2700.0]
  for iBin in range(1, len(binEdges) ):
    thisLowEdge = binEdges[iBin-1]
    thisUpEdge = binEdges[iBin-1]
    thisLowBin = finept.GetXaxis().FindBin( thisLowEdge+1)
    thisUpBin = finept.GetXaxis().FindBin( thisUpEdge-1 )
    thisNumEvents = 0.
    for iB in range(thisLowBin, thisUpBin+1):
      thisNumEvents += finept.GetBinContent(iB)
    numEvents.append( thisNumEvents )

  firstBin = finept.GetXaxis().FindBin( binEdges[-1]+1 )
  lastBin = finept.FindLastBinAbove(0)
  numRequiredBins = 6 #bins

#  ## Get new bins
#  thisNumEvents = 0.
#  currentBin = firstBin
#  numBinsAdded = 0
#  while( currentBin <= lastBin ):
#    thisNumEvents += finept.GetBinContent( currentBin )
#    numBinsAdded += 1
#
#    if numBinsAdded < numRequiredBins:
#      currentBin += 1
#      continue
#
#    if (thisNumEvents >= eventThreshold) or (currentBin == lastBin):
#      binEdges.append( finept.GetXaxis().GetBinUpEdge( currentBin ) )
#      numEvents.append( thisNumEvents )
#      thisNumEvents = 0.
#      numBinsAdded = 0
#      currentBin += 1
#      continue
#
#    if numBinsAdded > numRequiredBins:
#      numRequiredBins += 1
#      currentBin += 1
#

  ### Draw Histogram ###

  hist = ROOT.TH1F("ptHist", "ptHist", len(binEdges)-1, array.array('f', binEdges) )
  for iBin in range(1, finept.GetNbinsX()+1):
    hist.Fill( finept.GetXaxis().GetBinLowEdge(iBin)+1, finept.GetBinContent(iBin) )

  print binName
  print "Edges:", binEdges
  print "Content:",
  for iBin in range(1, hist.GetNbinsX()+1):
    print hist.GetBinContent(iBin),
    hist.SetBinError(iBin, math.sqrt(hist.GetBinContent(iBin)))
  print ""
  hist.GetXaxis().SetRangeUser(200, 2800)
  hist.SetLineColor(ROOT.kRed)
  hist.SetMarkerColor(ROOT.kRed)
  hist.SetBinContent(0, 0)
  hist.SetBinError(0, 0)

  c1 = ROOT.TCanvas()
  hist.Draw()
  c1.SetLogy()
  c1.SaveAs("BinCalc_"+binName+".png")
