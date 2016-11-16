import array, os, sys
import math
import time, glob
import argparse

sys.path.insert(0, '../plotting/')
import AtlasStyle
AtlasStyle.SetAtlasStyle()

parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("-b", dest='batchMode', action='store_true', default=False, help="Batch mode for PyRoot")
parser.add_argument("--fineHist", dest='getFinePtHist', action='store_true', default=False, help="Create histograms with 1 GeV binning")
parser.add_argument("--calcBin", dest='calcBinning', action='store_true', default=False, help="Create complete binning from relevant histogram")
parser.add_argument("--iterHist", dest='getIterativeHist', action='store_true', default=False, help="Create iterative binning histograms with subleading pt cuts")
parser.add_argument("--plotIter", dest='plotIter', action='store_true', default=False, help="Plot iterative binning plots")
parser.add_argument("--file", dest='fileName', default="", help="Input file name")
parser.add_argument("--treeName", dest='treeName', default="outTree_Nominal", help="Input TTree name")
parser.add_argument("--dir", dest='dir', default="", help="Directory containing input files, used if --file is not set.")
parser.add_argument("--outName", dest='outName', default="", help="Tag to append to output name.")
parser.add_argument("--tag", dest='tag', default="", help="Tag for selecting input files.")
args = parser.parse_args()

if args.outName:
  args.outName += '_'

import ROOT

####################### User defined variables ###############################
triggers = ["HLT_j380","HLT_j260","HLT_j175", "HLT_j110"]
trigEffs = [550       ,400       ,300       , 200]

## For calcBinning, the initial edges to use ##
#calcBinEdges = [300,1050,1100,1150,1200,1300,1500,1700,2000,2300]
calcBinEdges = [300, 1700]
numRequiredBins = 300 #bins = GeV
#The threshold nevents for accepting a bin threshold
#Errors & numEvents : 1% = 10000, 2% = 2500, 3% = 1111, 4% = 625, 5% = 400
eventThreshold = 100

## For getIterativeHist, the final binEdges and different subleading jet cutoffs to use ##
iterativeCutoffs = [950, 1200,1300,1500,1700,2000,2300,5000]
iterativeEdges = [300,350,400,450,500,550,600,650,700,750,800,850,900,950,1000,1050,1100,1150,1200,1300,1500,1700,2000,2300]


########################### getFinePtHist ##################################3
if (args.getFinePtHist):
  
  
  ### Make output file ###
  if args.dir:
    outDir = args.dir+'/binningPlots/'
  else:
    outDir = os.path.dirname( args.fileName )+"/binningPlots/"
  if not os.path.exists( outDir ):
    os.makedirs( outDir )

  outFile = ROOT.TFile.Open(outDir+args.outName+"fineBinningHist.root", "RECREATE")
  h_finept = ROOT.TH1F("finept","finept",5000,0,5000.)

  ### Find input files
  if len(args.fileName) > 0:
    fileNames = [args.file]
  else:
    fileNames = glob.glob( args.dir+'/*'+args.tag+'*.root' )


  totalEntries = 0
  totalCount = 0
  for fileName in fileNames:

    inFile = ROOT.TFile.Open(fileName, "READ")
    tree = inFile.Get(args.treeName)
    print fileName
    totalEntries += tree.GetEntries()
    inFile.Close()



  ### Loop over input files and connect branches ###
  startTime = time.time()
  for fileName in fileNames:
    inFile = ROOT.TFile.Open(fileName, "READ")
    tree = inFile.Get(args.treeName)
    
    numEntries = tree.GetEntries()
    tree.SetBranchStatus('*', 0)
    #jet_pt =   ROOT.std.vector('float')()
    #tree.SetBranchStatus( "jet_pt", 1)
    #tree.SetBranchAddress( "jet_pt", jet_pt)
    recoil_pt = array.array('f',[0])
    tree.SetBranchStatus( "recoilPt", 1)
    tree.SetBranchAddress( "recoilPt", recoil_pt)
    #weight = array.array('f',[0])
    #tree.SetBranchStatus( "weight", 1)
    #tree.SetBranchAddress( "weight", weight)
    passedTriggers =   ROOT.std.vector('string')()
    tree.SetBranchStatus( "passedTriggers", 1)
    tree.SetBranchAddress( "passedTriggers", passedTriggers)


    ## Loop over input TTree ##
    count = 0
    print "Running on " , numEntries, "entries"
    while tree.GetEntry(count):
      count += 1
      totalCount += 1
      if totalCount%1e5 == 0:  print "Event ", totalCount, ". It's been", (time.time() - startTime)/60. , "minutes.  Event rate is ", totalCount/(time.time()-startTime), " events per second.  We need about ", (totalEntries-totalCount)*(time.time()-startTime)/totalCount/60., " more minutes."


      # For each trigger we're using
      for iT, trigEff in enumerate(trigEffs):
        #If it passes the trigger efficiency cut
        if recoil_pt[0] > trigEff*1e3:
          #Check the one trigger for this pt range 
          if triggers[iT] in passedTriggers:
            h_finept.Fill( recoil_pt[0] / 1e3 )
          break #only check triggers once

    inFile.Close()

  outFile.Write()
  outFile.Close()

########################### getIterativeHist ##################################
if (args.getIterativeHist):

  ## Make output histograms ##
  outDir = os.path.dirname( args.fileName )+"/binningPlots/"
  if not os.path.exists( outDir ):
    os.makedirs( outDir )

  outFile = ROOT.TFile.Open(outDir+"IterativeBinningHist.root", "RECREATE")

  vh_iterative = []
  for cutoff in iterativeCutoffs:
    vh_iterative.append( ROOT.TH1F("Iteration_"+str(cutoff),"Iteration_"+str(cutoff), len(iterativeEdges)-1, array.array('f',iterativeEdges)) )

  ### Loop over TTree ###
  count = 0
  print "Running on " , numEntries, "entries"
  startTime = time.time()
  while tree.GetEntry(count):
    count += 1
    if count%1e5 == 0:  print "Event ", count, ". It's been", (time.time() - startTime)/60. , "minutes.  Event rate is ", count/(time.time()-startTime), " events per second.  We need about ", (numEntries-count)*(time.time()-startTime)/count/60., " more minutes."

    # For each trigger we're using
    for iT, trigEff in enumerate(trigEffs):
      #If it passes the trigger efficiency cut
      if recoil_pt[0] > trigEff*1e3:
        #Check the one trigger for this pt range 
        if triggers[iT] in passedTriggers:
          # Fill iterative plots with subleading pt requirement
          for iI, h_iterative in enumerate(vh_iterative):
            if jet_pt[1] <= iterativeCutoffs[iI]:
              h_iterative.Fill( recoil_pt[0] / 1e3 )
        break #only check triggers once

  outFile.Write()
  outFile.Close()

########################### calcBinning ##############################
if (args.calcBinning):

  numEvents = []

  ### Get saved fineBinningHist ###
  inDir = os.path.dirname( args.fileName )+"/"
#  if not "binningPlots" in inDir:
#    inDir += "binningPlots/"
  inFile =  ROOT.TFile.Open( args.fileName, "READ" )
  finept = inFile.Get("finept")
  finept.GetXaxis().SetTitle("Recoil p_{T} (GeV)")
  finept.GetYaxis().SetTitle("Events")

  ## Get nevents in bins defined by initial calcBinEdges ##
  if len(calcBinEdges) > 1:
    for iBin in range(1, len(calcBinEdges) ):
      thisLowEdge = calcBinEdges[iBin-1]
      thisUpEdge = calcBinEdges[iBin-1]
      thisLowBin = finept.GetXaxis().FindBin( thisLowEdge+1)
      thisUpBin = finept.GetXaxis().FindBin( thisUpEdge-1 )
      thisNumEvents = 0.
      for iB in range(thisLowBin, thisUpBin+1):
        thisNumEvents += finept.GetBinContent(iB)
      numEvents.append( thisNumEvents )


  ## Calculate the new bin edges ##
  thisNumEvents = 0. # nevents currently in this bin
  currentBin = finept.GetXaxis().FindBin( calcBinEdges[-1] ) #First bin to include
  lastBin = finept.FindLastBinAbove(0)
  numBinsAdded = 0  # number of bins added, must be a multiple of numRequiredBins

  # while we haven't passed the last bin
  while( currentBin <= lastBin ):
    thisNumEvents += finept.GetBinContent( currentBin )
    numBinsAdded += 1

    ## If we're at the last bin, add it to the edge
    if currentBin == lastBin:
      calcBinEdges.append( finept.GetXaxis().GetBinUpEdge( currentBin ) )
      numEvents.append( thisNumEvents )
      thisNumEvents = 0.
      numBinsAdded = 0
      currentBin += 1

    #if the bin width is wider than the maximum, permanently double the numRequiredBins:
    elif (numBinsAdded > numRequiredBins):
      numRequiredBins = numRequiredBins*2
      currentBin += 1

    # if we have enough events in this bin, and the bin width is appropriate
    elif (thisNumEvents >= eventThreshold and numBinsAdded == numRequiredBins ):
      calcBinEdges.append( finept.GetXaxis().GetBinUpEdge( currentBin ) )
      numEvents.append( thisNumEvents )
      thisNumEvents = 0.
      numBinsAdded = 0
      currentBin += 1
    
    #if we don't have enough events yet
    else:
      currentBin += 1

  ######## Plot Histogram of bin decisions##########

  hist = ROOT.TH1F("ptHist", "ptHist", len(calcBinEdges)-1, array.array('f', calcBinEdges) )
  for iBin in range(1, finept.GetNbinsX()+1):
    hist.Fill( finept.GetXaxis().GetBinLowEdge(iBin)+1, finept.GetBinContent(iBin) )

  print "Edges:", calcBinEdges
  print "Content:",
  for iBin in range(1, hist.GetNbinsX()+1):
    print "Edge ", hist.GetXaxis().GetBinLowEdge(iBin), "  -- ", hist.GetBinContent(iBin)
    hist.SetBinError(iBin, math.sqrt(hist.GetBinContent(iBin)))
  print ""

  hist.GetXaxis().SetRangeUser(200, 2800)
  hist.SetBinContent(0, 0)
  hist.SetBinError(0, 0)

  c1 = ROOT.TCanvas()
  hist.Draw()
  c1.SetLogy()
  c1.SaveAs(inDir+"BinningCalculation.png")

######################## plotIter ##################################
if (args.plotIter):

  ## Get Histograms ##
  inDir = os.path.dirname( args.fileName )+"/"
  if not "binningPlots" in inDir:
    inDir += "binningPlots/"
  inFile =  ROOT.TFile.Open( inDir+"IterativeBinningHist.root", "READ" )

  vh_iterative = []
  for cutoff in iterativeCutoffs:
    vh_iterative.append( inFile.Get("Iteration_"+str(cutoff)) )

  ### Draw Histograms ###
  colors = [ROOT.kRed, ROOT.kBlue, ROOT.kGreen, ROOT.kCyan, ROOT.kOrange, ROOT.kViolet, ROOT.kPink-7, ROOT.kSpring-7]

  c1 = ROOT.TCanvas()
  leg = ROOT.TLegend(0.6, 0.6, 0.88, 0.9)
  for iH, hist in enumerate(vh_iterative):
    hist.GetXaxis().SetTitle("Recoil p_{T}")
    hist.GetYaxis().SetTitle("Events")
    hist.SetLineColor(colors[iH])
    hist.SetMaximum( hist.GetMaximum()**(3./2) )
    if iH == 0:
      hist.Draw()
    else:
      hist.Draw("same")
    leg.AddEntry( hist, hist.GetTitle(), "l")


  AtlasStyle.ATLAS_LABEL(0.2,0.9, 1,"  Internal")
  AtlasStyle.myText(0.2,0.84,1, "#sqrt{s} = 13 TeV, ~27 fb^{-1}")
  
  leg.Draw("same")
 
  c1.SetLogy()
  c1.SaveAs(inDir+"IterativeBinning.png")
