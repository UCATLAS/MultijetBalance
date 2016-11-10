import ROOT, array, sys, os
import math
import time
import glob

sys.path.insert(0, '../plotting/')
import AtlasStyle
AtlasStyle.SetAtlasStyle()

import argparse

parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
parser.add_argument("--calculate", dest='f_calculate', action='store_true', default=False, help="Calculate the efficiency numerator and denominators.")
parser.add_argument("--plot", dest='f_plot', action='store_true', default=False, help="Plot MC and Data curves together.")
parser.add_argument("--data", dest='data', action='store_true', default=False, help="Running on data.")
parser.add_argument("--mc", dest='mc', action='store_true', default=False, help="Running on mc.")
parser.add_argument("--file", dest='file', default="", help="Input file name.")
parser.add_argument("--outName", dest='outName', default="Study", help="Tag to append to output name.")
parser.add_argument("--nevents", dest='nevents', type=int, default = -1, help="Maximum number of events to run over.")
parser.add_argument("--dir", dest='dir', default="", help="Directory containing input files, used if --file is not set.")
args = parser.parse_args()

def checkTrigger():
  startTime = time.time()
  triggers = ["HLT_j380","HLT_j260","HLT_j175","HLT_j110"]
  types = ["unprescaled", "unbiased", "prescaled"]

  if args.f_calculate:
    if len(args.file) > 0:
      fileNames = [args.file]
    else:
      fileNames = glob.glob( args.dir+'/*.root' )
  
  
    treeName = "outTree_Nominal"
  
    outDir = os.path.dirname( fileNames[0] )+"/triggerPlots/"
    if not os.path.exists( outDir ):
      os.makedirs( outDir )
  
    outName = outDir+'TriggerHists_'
    if args.data:
      outName += '_Data_'+args.outName+'.root'
    elif args.mc:
      outName += '_MC_'+args.outName+'.root'
    else:
      print "Warning, must set --data or --mc"
      exit(1)
  
  
    # 2D lists of histograms - trigger:type
    h_recoilpt_numer = []
    h_recoilpt_denom = []
    h_jetpt_numer = []
    h_jetpt_denom = []
  
  
    outFile = ROOT.TFile.Open(outName, "RECREATE")
    #loop over each trigger
    for trigger in triggers:
      #If last trigger, skip
      if trigger == triggers[-1]:
        continue
      h_recoilpt_numer.append( [] )
      h_jetpt_numer.append( [] )
      h_recoilpt_denom.append( [] )
      h_jetpt_denom.append( [] )
  
      #Create histograms
      for thisType in types:
        # numerator for recoilpt
        h_recoilpt_numer[-1].append( ROOT.TH1F(thisType+"_recoilpt_numer_"+trigger, "h_recoilpt_numer", 300, 50, 2000) )
        # numerator jetpt
        h_jetpt_numer[-1].append( ROOT.TH1F(thisType+"_jetpt_numer_"+trigger, "h_jetpt_numer", 300, 50, 2000) )
        # denominator for recoilpt
        h_recoilpt_denom[-1].append( ROOT.TH1F(thisType+"_recoilpt_denom_"+trigger, "h_recoilpt_denom", 300, 50, 2000) )
        # denominator for jetpt
        h_jetpt_denom[-1].append( ROOT.TH1F(thisType+"_jetpt_denom_"+trigger, "h_jetpt_denom", 300, 50, 2000) )
  
    for fileName in fileNames:
  
      inFile = ROOT.TFile.Open(fileName, "READ")
      tree = inFile.Get(treeName)
      numEntries = tree.GetEntries()
  
  
      #Weight by cutflow if using MC
      cutflowWeight = 1
      if args.mc:
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
      while tree.GetEntry(count):
        count += 1
        if (args.nevents > -1) and (count > args.nevents):
          break
  
        if count%100000 == 0:
          print count
          print "It's been", (time.time() - startTime)/60. , "minutes.  Event rate is ", count/(time.time()-startTime), " events per second"
  
  # !!TODO add back asym cut?
  #      if( jet_pt[1]*1000/recoilPt[0] > 0.8):
  #        continue
  
        for iT, trigger in enumerate(triggers):
  
          #Skip last trigger
          if trigger == triggers[-1]:
            continue
  
          #Use previous trigger events for data
          if args.data:
            weight[0] = 1.
            cutflowWeight = 1.
  
  
          for iType, thisType in enumerate(types):
            if thisType == "prescaled": 
  
              # Unprescaled method - for when trigger in question is unprescaled
              # numerator is all events passing this trigger
              # denominator is all events passing this trigger or the next lowest trigger
              if trigger in passedTriggers or triggers[iT+1] in passedTriggers:
                h_recoilpt_denom[iT][iType].Fill( recoilPt[0]/1e3, weight[0]*cutflowWeight)
                h_jetpt_denom[iT][iType].Fill( jet_pt[0], weight[0]*cutflowWeight)
              if trigger in passedTriggers:
                h_recoilpt_numer[iT][iType].Fill( recoilPt[0]/1e3, weight[0]*cutflowWeight )
                h_jetpt_numer[iT][iType].Fill( jet_pt[0], weight[0]*cutflowWeight )
  
            elif thisType == "unbiased":
  
              ## unbiased - This is for MC when no requirement on trigger is made
              # numerator is all events passing this trigger
              # denominator is all events passing any trigger
              h_recoilpt_denom[iT][iType].Fill( recoilPt[0]/1e3, weight[0]*cutflowWeight )
              h_jetpt_denom[iT][iType].Fill( jet_pt[0], weight[0]*cutflowWeight )
              if trigger in passedTriggers:
                h_recoilpt_numer[iT][iType].Fill( recoilPt[0]/1e3, weight[0]*cutflowWeight )
                h_jetpt_numer[iT][iType].Fill( jet_pt[0], weight[0]*cutflowWeight )
  
            elif thisType == "unprescaled":
              ## Correct biased - for prescaled triggers, but less statistics than Unprescaled method
              # numerator is events passing this trigger and the next lowest trigger
              # denominator is events passing the next lowest trigger
              if triggers[iT+1] in passedTriggers:
                h_recoilpt_denom[iT][iType].Fill( recoilPt[0]/1e3, weight[0]*cutflowWeight)
                h_jetpt_denom[iT][iType].Fill( jet_pt[0], weight[0]*cutflowWeight)
              if trigger in passedTriggers and triggers[iT+1] in passedTriggers:
                h_recoilpt_numer[iT][iType].Fill( recoilPt[0]/1e3, weight[0]*cutflowWeight )
                h_jetpt_numer[iT][iType].Fill( jet_pt[0], weight[0]*cutflowWeight )
  
  
    outFile.Write()
    outFile.Close()
  
  
  
  if args.f_plot:
    if len(args.file) > 0:
      fileNames = [args.file]
      outDir = os.path.dirname(args.file)+'/'
    else:
      fileNames = glob.glob( args.dir+'/*.root' )
      outDir = args.dir+'/'

  
    fileLabels = []
    colors = [ROOT.kBlack, ROOT.kRed, ROOT.kBlue]
  
    for fileName in fileNames:
      fileLabels.append( os.path.basename(fileName).replace('TriggerHists_','').replace('.root','') )
  
  
    c1 = ROOT.TCanvas()
  
    types = ["unprescaled", "prescaled"]
    variables = ["recoilpt","jetpt"]
  
    files = []
    for fileName in fileNames:
      files.append( ROOT.TFile.Open( fileName, "READ") )
  
    for trigger in triggers:
      #Skip last trigger
      if trigger == triggers[-1]:
        continue
  
  
      for thisType in types:
        for thisVar in variables:
          h_numer, h_denom = [], []
    
          for iFile, thisFile in enumerate(files):
            
            h_numer.append( thisFile.Get(thisType+"_"+thisVar+"_numer_"+trigger ) )
            h_numer[-1].SetDirectory(0)
            h_numer[-1].SetTitle(fileLabels[iFile]+'_'+h_numer[-1].GetName())
            h_numer[-1].SetLineColor( colors[iFile] )
            h_denom.append( thisFile.Get(thisType+"_"+thisVar+"_denom_"+trigger ) )
            h_denom[-1].SetDirectory(0)
            h_denom[-1].SetTitle(fileLabels[iFile]+'_'+h_denom[-1].GetName())
    
    
          fullyEfficientPt = []
          ## Divide numerator and denominator to get efficiency turn-on curve
          for iT, trig in enumerate(h_numer):
            h_numer[iT].Divide( h_denom[iT] )
    
            thisEfficiencyPt = getEfficiencyPoint( h_numer[iT], h_denom[iT] )
            fullyEfficientPt.append( thisEfficiencyPt )
    
          leg = ROOT.TLegend(0.7, 0.7, 0.9, 0.9)
          for iFile, thisFile in enumerate(files):
            leg.AddEntry( h_numer[iFile], fileLabels[iFile], "l")

            h_numer[iFile].SetMaximum(2)
            if iFile == 0:
              h_numer[iFile].Draw()
            else:
              h_numer[iFile].Draw("same")
      
          AtlasStyle.ATLAS_LABEL(0.2,0.9, 1,"  Internal")
          AtlasStyle.myText(0.2,0.84,1, "#sqrt{s} = 13 TeV, 3.6 fb^{-1}")
          EffString = thisVar+" "
          for iFile, thisFile in enumerate(files):
            EffString += fileLabels[iFile]+" "+str(fullyEfficientPt[iFile] )

          AtlasStyle.myText(0.2,0.78,2, EffString ) 
      
          leg.Draw("same")
      
          plotName = outDir+thisType+'_'+thisVar+'_'+trigger+'.png'
          c1.SaveAs(plotName);
      
          c1.Clear();

def getEfficiencyPoint( thisRatio, thisDenom ):
 
  #Set bins of 0 after turn-on to 1.  These beens have 0 events in numerator and denominator
  for iBin in range(thisRatio.FindFirstBinAbove(0.5), thisRatio.GetNbinsX()+1 ):
    if thisDenom.GetBinContent(iBin) == 0:
      thisRatio.SetBinContent(iBin, 1)
  #Set all bin errors to 0    
  for iBin in range(1, thisRatio.GetNbinsX()+1 ):
    thisRatio.SetBinError(iBin, 0)

  ####### Calculate pt of full efficiency.   ####

  ptStart = int( thisRatio.GetName().split('_')[-1].replace('j','') )
  print ptStart
  minBin = 1+thisRatio.GetXaxis().FindBin( ptStart)

  effBin = minBin

  for iBin in range(minBin+1, thisRatio.GetNbinsX()):
    if thisRatio.GetBinContent( iBin ) > thisRatio.GetBinContent( effBin ):
      effBin = iBin
    else:
      break
    if thisRatio.GetBinContent( iBin ) > 0.99:
      break

  return thisRatio.GetXaxis().GetBinLowEdge( effBin )
  
if __name__ == "__main__":
  checkTrigger()

