import math
import time
import glob


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
parser.add_argument("--tag", dest='tag', default="", help="Tag for selecting input files.")
args = parser.parse_args()

import ROOT, array, sys, os
sys.path.insert(0, '../plotting/')
import AtlasStyle
AtlasStyle.SetAtlasStyle()

def checkTrigger():
  startTime = time.time()
  #triggers = ["HLT_j380","HLT_j260"]
  triggers = ["HLT_j380","HLT_j260","HLT_j175","HLT_j110", "HLT_j85"]
  types = ["unprescaled", "unbiased", "prescaled"]

  if args.f_calculate:
    if len(args.file) > 0:
      fileNames = [args.file]
    else:
      fileNames = glob.glob( args.dir+'/*'+args.tag+'*.root' )
  
  
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
  

    totalEntries = 0
    totalCount = 0
    for fileName in fileNames:
  
      inFile = ROOT.TFile.Open(fileName, "READ")
      tree = inFile.Get(treeName)
      print fileName
      totalEntries += tree.GetEntries()
      inFile.Close()

    print "Running over a total of ", totalEntries, "events"

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
        totalCount += 1
        if (args.nevents > -1) and (count > args.nevents):
          break
  
        if totalCount%1e5 == 0:  print "Event ", totalCount, ". It's been", (time.time() - startTime)/60. , "minutes.  Event rate is ", totalCount/(time.time()-startTime), " events per second.  We need about ", (totalEntries-totalCount)*(time.time()-startTime)/totalCount/60., " more minutes."
  
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
      inFile.Close()
  
  
    outFile.Write()
    outFile.Close()
  
  
  
  if args.f_plot:
    if len(args.file) > 0:
      fileNames = [args.file]
      outDir = os.path.dirname(args.file)+'/'
    else:
      fileNames = glob.glob( args.dir+'/*'+args.tag+'*.root' )
      outDir = args.dir+'/'

    if (args.outName):
      outDir += args.outName+'/'
      if not os.path.exists( outDir ):
        os.makedirs( outDir )
  
    fileLabels = []
    colors = [ROOT.kBlack, ROOT.kRed, ROOT.kBlue]
  
    for fileName in fileNames:
      fileLabels.append( os.path.basename(fileName).replace('TriggerHists_','').replace('.root','') )
  
  
  
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

          h_eff = []
    
          for iFile, thisFile in enumerate(files):
            
            h_numer.append( thisFile.Get(thisType+"_"+thisVar+"_numer_"+trigger ) )
            h_numer[-1].SetDirectory(0)
            h_numer[-1].SetTitle(fileLabels[iFile]+'_'+h_numer[-1].GetName())
            h_numer[-1].SetLineColor( colors[iFile] )
            h_denom.append( thisFile.Get(thisType+"_"+thisVar+"_denom_"+trigger ) )
            h_denom[-1].SetDirectory(0)
            h_denom[-1].SetTitle(fileLabels[iFile]+'_'+h_denom[-1].GetName())
    
    
          fullyEfficientPt = []
          fits = []
          ## Divide numerator and denominator to get efficiency turn-on curve
          for iT, trig in enumerate(h_numer):
            #h_numer[iT].Divide( h_denom[iT] )
            #h_eff.append( h_numer[iT] )

            minPt, triggerPt = findMinimum( h_numer[iT], h_denom[iT] )
            thisTEff = ROOT.TEfficiency(h_numer[iT], h_denom[iT])
            thisTEff.SetLineColor( colors[iT] )
            thisTEff.SetMarkerColor( colors[iT] )
            thisTEff.SetMarkerSize( 1 )
            h_eff.append( thisTEff )

            thisFit = ROOT.TF1( 'Fit_'+h_numer[iT].GetName(), EffFit, minPt, triggerPt+200, 3)
            thisFit.SetParameters(0.8, 300, 50) #From a low pt trigger turn-on, relatively meaningless
            thisFit.SetParLimits(0, 0., 1.)
            thisTEff.Fit(thisFit)
            thisFit.SetLineColor( colors[iT] )
            #thisFit.SetLineWidth( 1 )
            fits.append( thisFit)
            #fit1.Draw()
            #time.sleep(100)
    
            thisEfficiencyPt = getEfficiencyPoint( thisFit )
            fullyEfficientPt.append( thisEfficiencyPt )
    
          c1 = ROOT.TCanvas()
          leg = ROOT.TLegend(0.7, 0.7, 0.9, 0.9)
          for iFile, thisFile in enumerate(files):
            leg.AddEntry( h_eff[iFile], fileLabels[iFile], "l")

#            h_eff[iFile].SetMaximum(2)
            if iFile == 0:
              h_eff[iFile].Draw("")
              ROOT.gPad.Update()
              graph = h_eff[iFile].GetPaintedGraph()
              graph.SetMaximum(2)
              ROOT.gPad.Update()
            else:
              h_eff[iFile].Draw("same")

            fits[iFile].Draw("same")


      
          AtlasStyle.ATLAS_LABEL(0.2,0.9, 1,"    Internal ("+thisVar+")")
          AtlasStyle.myText(0.2,0.84,1, "#sqrt{s} = 13 TeV, 3.6 fb^{-1}")
          for iFile, thisFile in enumerate(files):
            EffString = fileLabels[iFile].replace('_',' ')+" "+str(fullyEfficientPt[iFile] )
            AtlasStyle.myText(0.2,0.78-(0.06*iFile),1, EffString ) 
      
#          leg.Draw("same")
      
          plotName = outDir+thisType+'_'+thisVar+'_'+trigger+'.png'
          c1.SaveAs(plotName);
      
          c1.Clear();

def findMinimum( numerHist, denomHist ):

  ratioHist = numerHist.Clone("tmpHist")
  ratioHist.Divide(denomHist)
  ratioHist.Draw()
  triggerPt = numerHist.GetName().split('_')[-1]
  #triggerPt = [thisStr for thisStr in numerHist.GetName().split('_') if thisStr.startswith('j')][0]
  triggerPt = int( triggerPt[1:])
  print "Found trigger value", triggerPt

  ratioHist.GetXaxis().SetRangeUser(3, triggerPt)
  minPt = ratioHist.GetXaxis().GetBinLowEdge( ratioHist.GetMinimumBin() )
  return minPt, triggerPt

def getEfficiencyPoint( EfficiencyFit):

  #hist = EfficiencyFit.GetHistogram()
  #hist.Draw()
 
  maxEff = EfficiencyFit.GetParameter(0)
  print "Maximum efficiency is at ", maxEff
  return int(math.ceil(EfficiencyFit.GetX( 0.99*maxEff )))

def EffFit(x, p):
  
  #return (1/2.)*(1+ROOT.TMath.Erf( (x[0]-p[0])/(math.sqrt(2)*p[1]) ) )
  return (p[0]/2.)*(1+ROOT.TMath.Erf( (x[0]-p[1])/(math.sqrt(2)*p[2]) ) )
  
if __name__ == "__main__":
  checkTrigger()

