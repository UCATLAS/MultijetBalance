import ROOT, array
import math
import time

f_getBinning = False

#f_getBinning = True
#binName = "3p"

fileName = "allPt_data.root"
treeName = "ptBinTree"
branchName = "recoilPt"

####binEdges = [200.,300.,400.,500.,600.,700.,800.,900.,1100.,1300.,1600.,1900.,2400.,3000.] #8TeV
#binEdges = [500.,600.,700.,800.,900.,1100.,1300.,1600.,1900.,2400.,3000.] #8TeV
#binName = "8TeV"

##ptBins_Fine = [15. ,20. ,25. ,35. ,45. ,55. ,70. ,85. ,100. ,116. ,134. ,152. ,172. ,194. ,216. ,240. ,264. ,290. ,318. ,346.,376.,408.,442.,478.,516.,556.,598.,642.,688.,736.,786.,838.,894.,952.,1012.,1076.,1162.,1310.,1530.,1992.,2500., 3000., 3500., 4500.]
binEdges = [478., 516.,556.,598.,642.,688.,736.,786.,838.,894.,952.,1012.,1076.,1162.,1310.,1530.,1992.,2500., 3000., 3500., 4500.] #Fine binning
binName = "Fine"

inFile = ROOT.TFile.Open(fileName, "READ")
tree = inFile.Get(treeName)

numEntries = tree.GetEntries()
b_pt = array.array('f',[0])
tree.SetBranchStatus( branchName, 1)
tree.SetBranchAddress( branchName, b_pt)

### Calculate binning ###
if (f_getBinning):

  count = 0
  pts = []
  while tree.GetEntry(count):
    pts.append( b_pt[0]/1e3 )
    count += 1

  pts = sorted(pts)
  pts = [pt for pt in pts if pt >= 500]

  ##print pts
  numEvents = []
  binEdges = [500]
  increase = 50
  while( len(pts) > 0 ):
    iNBin = next(pt[0] for pt in enumerate(pts) if pt[1] > binEdges[-1]+increase)
    print pts[iNBin], len(pts[0:iNBin])
#1p = 10000, 3p = 1111, 5p = 500
    if len(pts[0:iNBin]) >= 277 or iNBin == len(pts)-1:
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
  hist.Fill(b_pt[0]/1e3,4)
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
