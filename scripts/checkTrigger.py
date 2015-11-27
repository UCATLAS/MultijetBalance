import ROOT, array, sys, os
import math
import time
import glob

sys.path.insert(0, '/home/jdandoy/Documents/Dijet/DijetFW/DijetHelpers/scripts/')
import AtlasStyle
AtlasStyle.SetAtlasStyle()

import argparse

parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
parser.add_argument("-run", dest='f_run', action='store_true', default=False, help="Running on data")
parser.add_argument("-plot", dest='f_plot', action='store_true', default=False, help="Plot MC and Data curves together.")
#parser.add_argument("-unbiased", dest='f_unbiased', action='store_true', default=False, help="Make MC denominator unbiased")
parser.add_argument("-data", dest='f_data', action='store_true', default=False, help="Running on data")
parser.add_argument("--file", dest='file', default="",
         help="Input file name")
parser.add_argument("--dir", dest='dir', default="",
         help="Directory containing input files")
args = parser.parse_args()

f_data = args.f_data

startTime = time.time()


triggers = ["HLT_j360","HLT_j260","HLT_j200","HLT_j175","HLT_j150","HLT_j110","HLT_j85"]
lastTrigger = "HLT_j85"
leadStart = [300, 200, 200, 150, 100, 100]

if args.f_run:
  if len(args.file) > 0:
    fileNames = [args.file]
  else:
    fileNames = glob.glob( args.dir+'/*.root' )


  treeName = "outTree_Nominal"

  outDir = os.path.dirname( fileNames[0] )+"/triggerPlots/"
  if not os.path.exists( outDir ):
    os.makedirs( outDir )

  outName = outDir+'TriggerHists'
  if f_data:
    outName += '_Data.root'
  else:
    outName += '_MC.root'


  h_triggers = []
  h_denom = []
  h_lead_triggers = []
  h_lead_denom = []

  types = ["wrong", "unbaised", "correct"]

  outFile = ROOT.TFile.Open(outName, "RECREATE")
  for trigger in triggers:
    if trigger == lastTrigger:
      continue
    h_triggers.append( [] )
    h_lead_triggers.append( [] )
    h_denom.append( [] )
    h_lead_denom.append( [] )

    for thisType in types:
      h_triggers[-1].append( ROOT.TH1F(thisType+"_"+trigger, "h_"+trigger, 300, 50, 2000) )
      h_lead_triggers[-1].append( ROOT.TH1F(thisType+"_lead_"+trigger, "h_lead_"+trigger, 300, 50, 2000) )
      h_denom[-1].append( ROOT.TH1F(thisType+"_denom_"+trigger, "h_full", 300, 50, 2000) )
      h_lead_denom[-1].append( ROOT.TH1F(thisType+"_lead_denom_"+trigger, "h_lead", 300, 50, 2000) )

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


    print "Running over ", tree.GetEntries(), "entries"
    count = 0
#    maxEvents = 500000
    while tree.GetEntry(count):
      count += 1
#      if count > maxEvents:
#        break

      if count%100000 == 0:
        print count
        print "It's been", (time.time() - startTime)/60. , "minutes.  Event rate is ", count/(time.time()-startTime), " events per second"

      if( jet_pt[1]*1000/recoilPt[0] > 0.8):
        continue

      for iT, trigger in enumerate(triggers):
        if trigger == lastTrigger:
          continue

        #Use previous trigger events for data
        if f_data:
          weight[0] = 1.
          cutflowWeight = 1.


        # Wrong method
        if trigger in passedTriggers or triggers[iT+1] in passedTriggers:
          h_denom[iT][0].Fill( recoilPt[0]/1e3, weight[0]*cutflowWeight)
          h_lead_denom[iT][0].Fill( jet_pt[0], weight[0]*cutflowWeight)
        if trigger in passedTriggers:
          h_triggers[iT][0].Fill( recoilPt[0]/1e3, weight[0]*cutflowWeight )
          h_lead_triggers[iT][0].Fill( jet_pt[0], weight[0]*cutflowWeight )

        ## unbiased
        h_denom[iT][1].Fill( recoilPt[0]/1e3, weight[0]*cutflowWeight )
        h_lead_denom[iT][1].Fill( jet_pt[0], weight[0]*cutflowWeight )
        if trigger in passedTriggers:
          h_triggers[iT][1].Fill( recoilPt[0]/1e3, weight[0]*cutflowWeight )
          h_lead_triggers[iT][1].Fill( jet_pt[0], weight[0]*cutflowWeight )

        ## Correct biased
        if triggers[iT+1] in passedTriggers:
          h_denom[iT][2].Fill( recoilPt[0]/1e3, weight[0]*cutflowWeight)
          h_lead_denom[iT][2].Fill( jet_pt[0], weight[0]*cutflowWeight)
        if trigger in passedTriggers and triggers[iT+1] in passedTriggers:
          h_triggers[iT][2].Fill( recoilPt[0]/1e3, weight[0]*cutflowWeight )
          h_lead_triggers[iT][2].Fill( jet_pt[0], weight[0]*cutflowWeight )


  outFile.Write()
  outFile.Close()


#  c1 = ROOT.TCanvas()
#  if not args.f_plotBoth:
#    for iT, trigger in enumerate(triggers):
#      if trigger == lastTrigger:
#        continue
#
#      print trigger
#
#      ## Create Efficiency turn on curve
#      h_triggers[iT].Divide( h_denom[iT] )
#      for iBin in range(h_triggers[iT].FindFirstBinAbove(0.5), h_triggers[iT].GetNbinsX()+1 ):
#        if h_denom[iT].GetBinContent(iBin) == 0:
#          h_triggers[iT].SetBinContent(iBin, 1)
#      for iBin in range(1, h_triggers[iT].GetNbinsX()+1 ):
#        h_triggers[iT].SetBinError(iBin, 0)
#
#      ## Calculate Efficiency point
#      if f_data:
#        ## Find minimum bin below 400 GeV
#        minBin = 1
#        for iBin in range(1, 55):
#          binCont = h_triggers[iT].GetBinContent(iBin)
#          if binCont > 0 and binCont < h_triggers[iT].GetBinContent(minBin):
#            minBin = iBin
#        ## Find point at which efficiency dips - this is the turnoff point
#        minBin += 8
#        effBin = minBin
#        for iBin in range(minBin+1, h_triggers[iT].GetNbinsX()):
#          if h_triggers[iT].GetBinContent( iBin ) > h_triggers[iT].GetBinContent( effBin ):
#            effBin = iBin
#          else:
#            break
#          if h_triggers[iT].GetBinContent( iBin ) > 0.995:
#            break
#
#      else:
#        effBin = h_triggers[iT].FindFirstBinAbove(0.995)
#
#      effPt = h_triggers[iT].GetXaxis().GetBinLowEdge( effBin )
#
#      h_triggers[iT].Draw()
#
#      AtlasStyle.ATLAS_LABEL(0.5,0.58, 1,"  Internal")
#      AtlasStyle.myText(0.5,0.52,1, "#sqrt{s} = 13 TeV, 3.6 fb^{-1}")
#    #  typeText = "anti-k_{t} R = 0.4"
#    #  typeText += ", EM+JES (in-situ)"
#    #  AtlasStyle.myText(0.2,0.76,1, typeText)
#      AtlasStyle.myText(0.5,0.46,1, " Efficient at %.0f GeV" % effPt)
#
#      plotName = outDir
#      if f_data:
#        plotName += "Data"
#      else:
#        plotName += "MC"
#      plotName += "_recoil_"+trigger+'.png'
#      c1.SaveAs(plotName);
#
#
#      ## Create Efficiency turn on curve
#      h_lead_triggers[iT].Divide( h_lead_denom[iT] )
#      for iBin in range(h_lead_triggers[iT].FindFirstBinAbove(0.5), h_lead_triggers[iT].GetNbinsX()+1 ):
#        if h_lead_denom[iT].GetBinContent(iBin) == 0:
#          h_lead_triggers[iT].SetBinContent(iBin, 1)
#      for iBin in range(1, h_lead_triggers[iT].GetNbinsX()+1 ):
#        h_lead_triggers[iT].SetBinError(iBin, 0)
#
#      ## Calculate Efficiency point
#      if f_data:
#        ## Find point at which efficiency dips - this is the turnoff point
#        minBin = h_lead_triggers[iT].GetXaxis().FindBin(leadStart[iT])
#        effBin = minBin
#        for iBin in range(minBin+1, h_lead_triggers[iT].GetNbinsX()):
#          print h_lead_triggers[iT].GetBinContent( iBin ), h_lead_triggers[iT].GetBinContent( effBin ), iBin, effBin
#          if h_lead_triggers[iT].GetBinContent( iBin ) < 0.2:
#            effBin = iBin
#            continue
#          if h_lead_triggers[iT].GetBinContent( iBin ) > h_lead_triggers[iT].GetBinContent( effBin ):
#            effBin = iBin
#          else:
#            break
#          if h_lead_triggers[iT].GetBinContent( iBin ) > 0.995:
#            break
#
#      else:
#        effBin = h_lead_triggers[iT].FindFirstBinAbove(0.995)
#
#      effPt = h_lead_triggers[iT].GetXaxis().GetBinLowEdge( effBin )
#
#      h_lead_triggers[iT].Draw()
#      print effPt
#
#      AtlasStyle.ATLAS_LABEL(0.5,0.58, 1,"  Internal")
#      AtlasStyle.myText(0.5,0.52,1, "#sqrt{s} = 13 TeV, 3.6 fb^{-1}")
#    #  typeText = "anti-k_{t} R = 0.4"
#    #  typeText += ", EM+JES (in-situ)"
#    #  AtlasStyle.myText(0.2,0.76,1, typeText)
#      AtlasStyle.myText(0.5,0.46,1, " Efficient at %.0f GeV" % effPt)
#
#      plotName = outDir
#      if f_data:
#        plotName += "Data"
#      else:
#        plotName += "MC"
#      plotName += "_lead_"+trigger+'.png'
#      c1.SaveAs(plotName);

if args.f_plot:
  c1 = ROOT.TCanvas()
  upDir = "/home/jdandoy/Documents/Dijet/MultijetBalanceFW/gridOutput/Trigger_VA_EM_Nov23/workarea/"
  fileNames = [upDir+'/DataTrigger/triggerPlots/TriggerHists_Data.root',
      upDir+'/MCTrigger/triggerPlots/TriggerHists_MC.root',
      upDir+'/MCTrigger/triggerPlots/TriggerHists_MC.root']
  fileLabels = ['Data', 'MC Unbiased', 'MC Biased']
  colors = [ROOT.kBlack, ROOT.kRed, ROOT.kBlue]
  histName = ['correct', 'unbaised', 'correct']
#  histName = ['wrong', 'unbaised', 'wrong']

  files = []
  for fileName in fileNames:
    files.append( ROOT.TFile.Open( fileName, "READ") )

  for trigger in triggers:
    if trigger == lastTrigger:
      continue

    h_triggers, h_denom = [], []

    for iFile, thisFile in enumerate(files):
      h_triggers.append( thisFile.Get(histName[iFile]+"_"+trigger ) )
      h_triggers[-1].SetDirectory(0)
      h_triggers[-1].SetTitle(fileLabels[iFile]+'_'+h_triggers[-1].GetName())
      h_denom.append( thisFile.Get(histName[iFile]+"_denom_"+trigger ) )
      h_denom[-1].SetDirectory(0)
      h_denom[-1].SetTitle(fileLabels[iFile]+'_'+h_denom[-1].GetName())
      h_triggers[-1].SetLineColor( colors[iFile] )

      h_triggers.append( thisFile.Get(histName[iFile]+"_lead_"+trigger ) )
      h_triggers[-1].SetDirectory(0)
      h_triggers[-1].SetTitle(fileLabels[iFile]+'_'+h_triggers[-1].GetName())
      h_denom.append( thisFile.Get(histName[iFile]+"_lead_denom_"+trigger ) )
      h_denom[-1].SetDirectory(0)
      h_denom[-1].SetTitle(fileLabels[iFile]+'_'+h_denom[-1].GetName())
      h_triggers[-1].SetLineColor( colors[iFile] )


    effPt = []
    ## Create Efficiency turn on curve
    for iT, trig in enumerate(h_triggers):
      h_triggers[iT].Divide( h_denom[iT] )

      for iBin in range(h_triggers[iT].FindFirstBinAbove(0.5), h_triggers[iT].GetNbinsX()+1 ):
        if h_denom[iT].GetBinContent(iBin) == 0:
          h_triggers[iT].SetBinContent(iBin, 1)
      for iBin in range(1, h_triggers[iT].GetNbinsX()+1 ):
        h_triggers[iT].SetBinError(iBin, 0)

      ## Calculate Efficiency point
      minBin = 1
      checkNum = 5
      for iBin in range(minBin, 60):
        onlyIncreasing = True
        for iCheck in range(iBin, iBin+checkNum):
          print h_triggers[iT].GetBinContent(iCheck+1) ,'<', h_triggers[iT].GetBinContent(iCheck)
          if h_triggers[iT].GetBinContent(iCheck+1) <= h_triggers[iT].GetBinContent(iCheck):
            print "False"
            onlyIncreasing = False
            break

        if onlyIncreasing:
          print "Chose ", iBin
          minBin = iBin
          break

#      minBin = h_triggers[iT].GetXaxis().FindBin(300)
      effBin = minBin

      for iBin in range(minBin+1, h_triggers[iT].GetNbinsX()):
        if h_triggers[iT].GetBinContent( iBin ) > h_triggers[iT].GetBinContent( effBin ):
          effBin = iBin
        else:
          break
        if h_triggers[iT].GetBinContent( iBin ) > 0.990:
          break

      effPt.append( h_triggers[iT].GetXaxis().GetBinLowEdge( effBin ) )


#    ## Find minimum bin below 400 GeV
#    minBin = 1
#      binCont = h_data_triggers[iT].GetBinContent(iBin)
#      if binCont > 0 and binCont < h_data_triggers[iT].GetBinContent(minBin):
#        minBin = iBin
#    ## Find point at which efficiency dips - this is the turnoff point
#    minBin += 8
#    effBin = minBin
#    for iBin in range(minBin+1, h_data_triggers[iT].GetNbinsX()):
#      if h_data_triggers[iT].GetBinContent( iBin ) > h_data_triggers[iT].GetBinContent( effBin ):
#        effBin = iBin
#      else:
#        break
#      if h_data_triggers[iT].GetBinContent( iBin ) > 0.995:
#        break
#
#    mc_effBin = h_mc_triggers[iT].FindFirstBinAbove(0.995)
####
#
#    effPt = h_triggers[iT].GetXaxis().GetBinLowEdge( effBin )


    leg = ROOT.TLegend(0.6, 0.4, 0.9, 0.9)
    leg.AddEntry( h_triggers[0], "Data "+trigger, "l")
    leg.AddEntry( h_triggers[2], "MC", "l")
    leg.AddEntry( h_triggers[4], "Biased MC", "l")

    h_triggers[0].Draw()
    h_triggers[2].Draw("same")
    h_triggers[4].Draw("same")


    AtlasStyle.ATLAS_LABEL(0.3,0.34, 1,"  Internal")
    AtlasStyle.myText(0.3,0.28,1, "#sqrt{s} = 13 TeV, 3.6 fb^{-1}")
#    AtlasStyle.myText(0.3,0.22,2, "99.5\% Efficient: %.0f GeV" %(effPt[0]) )
    AtlasStyle.myText(0.3,0.22,2, "Data: %.0f, MC: %.0f, MCbiased: %.0f GeV" %(effPt[0], effPt[2], effPt[4]) )

    leg.Draw("same")

    plotName = upDir+'/combinedPlots/'+histName[0]+'_Recoil_'+trigger+'.png'
#    plotName += h_data_triggers[iT].GetName()+'.png'
    c1.SaveAs(plotName);

    c1.Clear();

    h_triggers[1].Draw()
    h_triggers[3].Draw("same")
    h_triggers[5].Draw("same")

    AtlasStyle.ATLAS_LABEL(0.3,0.34, 1,"Internal")
    AtlasStyle.myText(0.3,0.28,1, "#sqrt{s} = 13 TeV, 3.6 fb^{-1}")
    AtlasStyle.myText(0.3,0.22,2, "Data: %.0f, MC: %.0f, MCbiased: %.0f GeV" %(effPt[1], effPt[3], effPt[5]) )
    leg.Draw("same")

    plotName = upDir+'/combinedPlots/'+histName[0]+'_Lead_'+trigger+'.png'
#    plotName += h_data_triggers[iT].GetName()+'.png'
    c1.SaveAs(plotName);



