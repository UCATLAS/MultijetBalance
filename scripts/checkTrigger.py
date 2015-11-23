import ROOT, array, sys
import math
import time

sys.path.insert(0, '/home/jdandoy/Documents/Dijet/DijetFW/DijetHelpers/scripts/')
import AtlasStyle
AtlasStyle.SetAtlasStyle()

#f_data = True
f_data = False

triggers = ["HLT_j360","HLT_j260","HLT_j200","HLT_j175","HLT_j150","HLT_j110","HLT_j85"]
lastTrigger = "HLT_j85"

#fileName = "../../submitDir/data-tree/data15_13TeV.00283270.physics_Main.merge.DAOD_EXOT2.f640_m1511_p2425.root"
fileNames = ["../../submitDir/data-tree/mc15_13TeV.361023.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ3W.merge.DAOD_EXOT2.e3668_s2576_s2132_r6765_r6282_p2432.root"]
treeName = "outTree_Nominal"

outName = "triggerPlots/TriggerHists"
if f_data:
  outName += '_Data.root'
else:
  outName += '_MC.root'

outFile = ROOT.TFile.Open(outName, "RECREATE")
h_triggers = []
h_denom = []
h_lead_triggers = []
h_lead_denom = []
for trigger in triggers:
  if trigger == lastTrigger:
    continue
  h_triggers.append( ROOT.TH1F("h_"+trigger, "h_"+trigger, 300, 50, 2000) )
  h_lead_triggers.append( ROOT.TH1F("h_lead_"+trigger, "h_lead_"+trigger, 300, 50, 2000) )
  h_denom.append( ROOT.TH1F("h_denom_"+trigger, "h_full", 300, 50, 2000) )
  h_lead_denom.append( ROOT.TH1F("h_lead_denom_"+trigger, "h_lead", 300, 50, 2000) )

for fileName in fileNames:

  inFile = ROOT.TFile.Open(fileName, "READ")
  tree = inFile.Get(treeName)
  numEntries = tree.GetEntries()


  cutflowWeight = 1
  if not f_data:
    for key in inFile.GetListOfKeys():
      if "cutflow" in key.GetName() and not "weight" in key.GetName():
        hist = inFile.Get(key.GetName())
        cutflowWeight = 1./hist.GetBinContent(1)
  print "Cutflow weight is ", cutflowWeight


  tree.SetBranchStatus('*', 0)
  jet_pt =   ROOT.std.vector('float')()
  tree.SetBranchStatus( "jet_pt", 1)
  tree.SetBranchAddress( "jet_pt", jet_pt)
  recoilPt = array.array('f',[0])
  tree.SetBranchStatus( "recoilPt", 1)
  tree.SetBranchAddress( "recoilPt", recoilPt)
  passedTriggers =   ROOT.std.vector('string')()
  tree.SetBranchStatus( "passedTriggers", 1)
  tree.SetBranchAddress( "passedTriggers", passedTriggers)
  weight =   array.array('f', [0])
  tree.SetBranchStatus( "weight", 1)
  tree.SetBranchAddress( "weight", weight)
  #triggerPrescales =   ROOT.std.vector('float')()
  #tree.SetBranchStatus( "triggerPrescales", 1)
  #tree.SetBranchAddress( "triggerPrescales", triggerPrescales)



  count = 0
  while tree.GetEntry(count):
    count += 1

    if count%1000 == 0:
      print count

    for iT, trigger in enumerate(triggers):
      if trigger == lastTrigger:
        continue

      #Use all events for MC
      if not f_data:
        h_denom[iT].Fill( recoilPt[0]/1e3, weight[0]*cutflowWeight )
        h_lead_denom[iT].Fill( jet_pt[0], weight[0]*cutflowWeight )

        if trigger in passedTriggers:
          h_triggers[iT].Fill( recoilPt[0]/1e3, weight[0]*cutflowWeight )
          h_lead_triggers[iT].Fill( jet_pt[0], weight[0]*cutflowWeight )

      #Use previous trigger events for data
      else:
        if trigger in passedTriggers or triggers[iT+1] in passedTriggers:
          h_denom[iT].Fill( recoilPt[0]/1e3)
          h_lead_denom[iT].Fill( jet_pt[0])

        if trigger in passedTriggers:
          h_triggers[iT].Fill( recoilPt[0]/1e3 )
          h_lead_triggers[iT].Fill( jet_pt[0] )

outFile.Write()

c1 = ROOT.TCanvas()
for iT, trigger in enumerate(triggers):
  if trigger == lastTrigger:
    continue

  ## Create Efficiency turn on curve
  h_triggers[iT].Divide( h_denom[iT] )
  for iBin in range(h_triggers[iT].FindFirstBinAbove(0.5), h_triggers[iT].GetNbinsX()+1 ):
    if h_denom[iT].GetBinContent(iBin) == 0:
      h_triggers[iT].SetBinContent(iBin, 1)
  for iBin in range(1, h_triggers[iT].GetNbinsX()+1 ):
    h_triggers[iT].SetBinError(iBin, 0)

  ## Calculate Efficiency point
  effBin = h_triggers[iT].FindFirstBinAbove(0.995)
  effPt = h_triggers[iT].GetXaxis().GetBinLowEdge( effBin )

  h_triggers[iT].Draw()
  print effPt

  AtlasStyle.ATLAS_LABEL(0.5,0.58, 1,"  Internal")
  AtlasStyle.myText(0.5,0.52,1, "#sqrt{s} = 13 TeV, 3.6 fb^{-1}")
#  typeText = "anti-k_{t} R = 0.4"
#  typeText += ", EM+JES (in-situ)"
#  AtlasStyle.myText(0.2,0.76,1, typeText)
  AtlasStyle.myText(0.5,0.46,1, " Efficient at %.0f GeV" % effPt)



  plotName = "triggerPlots/"
  if f_data:
    plotName += "Data"
  else:
    plotName += "MC"
  plotName += "_"+trigger+'.png'
  c1.SaveAs(plotName);


outFile.Close()
