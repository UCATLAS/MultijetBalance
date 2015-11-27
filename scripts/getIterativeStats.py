import ROOT, array
import math
import time
import argparse
import sys

#parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
#parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
#parser.add_argument("--dataFile", dest='dataFile', default="submitDir/hist-data12_8TeV.root",
#         help="Input file name")
#parser.add_argument("--mcFile", dest='mcFile', default="submitDir/hist-data12_8TeV.root",
#         help="Input file name")
#args = parser.parse_args()
sys.path.insert(0, '/home/jdandoy/Documents/Dijet/DijetFW/DijetHelpers/scripts/')
import AtlasStyle
AtlasStyle.SetAtlasStyle()

f_getHist = False
f_getBinning = True

fileName = "../../gridOutput/Trigger_VA_EM_Nov23/workarea/DataTrigger/allData.root"
treeName = "outTree_Nominal"
branchName = "recoilPt"
#1p = 10000, 3p = 1111, 5p = 400
eventThreshold = 300

triggers = ["HLT_j360","HLT_j260","HLT_j200"]
trigEffs = [480       ,360       ,300       ]

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

  binEdges = [300, 360, 420, 480, 540.0, 600.0, 660.0, 720.0, 780.0, 840.0, 900.0, 960.0, 1020.0, 1080.0, 1140.0, 1200.0, 1260.0, 1320.0, 1380.0, 1480.0, 1700.0, 2000.0, 2700.0]

  outFile = ROOT.TFile.Open("iterativeStats.root", "RECREATE")
  run1 = ROOT.TH1F("run1", "run1", len(binEdges)-1, array.array('f',binEdges) )
  run2 = ROOT.TH1F("run2", "run2", len(binEdges)-1, array.array('f',binEdges) )
  run3 = ROOT.TH1F("run3", "run3", len(binEdges)-1, array.array('f',binEdges) )



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
          if( jet_pt[1] <= 800):
            run1.Fill( b_pt[0] / 1e3 )
          if( jet_pt[1] <= 1260):
            run2.Fill( b_pt[0] / 1e3 )
          run3.Fill( b_pt[0] / 1e3 )

        break #only check triggers once


  outFile.Write()
  outFile.Close()

if (f_getBinning):

  inFile =  ROOT.TFile.Open("iterativeStats.root", "READ")
  run1 = inFile.Get("run1")
  run2 = inFile.Get("run2")
  run3 = inFile.Get("run3")


  run1.GetXaxis().SetRangeUser(200, 2800)
  run1.SetLineColor(ROOT.kBlue)
  run1.SetMarkerColor(ROOT.kBlue)
  run2.SetLineColor(ROOT.kRed)
  run2.SetMarkerColor(ROOT.kRed)
  run3.SetLineColor(ROOT.kBlack)
  run3.SetMarkerColor(ROOT.kBlack)


  c1 = ROOT.TCanvas()
  leg = ROOT.TLegend(0.6, 0.6, 0.9, 0.9)
  leg.AddEntry( run1, "800 GeV", "l")
  leg.AddEntry( run2, "1320 GeV", "l")
  leg.AddEntry( run3, "No Limit", "l")


  run3.Draw()
  run2.Draw("same")
  run1.Draw("same")
  leg.Draw("same")
  c1.SetLogy()
  c1.SaveAs("IterativeStats.png")
