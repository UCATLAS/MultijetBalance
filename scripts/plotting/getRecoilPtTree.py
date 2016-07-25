import glob, array, argparse, os

#put argparse before ROOT call.  This allows for argparse help options to be printed properly (otherwise pyroot hijacks --help) and allows -b option to be forwarded to pyroot
parser = argparse.ArgumentParser(description="%prog [options]", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--pathToTrees", dest='pathToTrees', default="./gridOutput/tree", help="Path to the input trees")
parser.add_argument("--treeName", dest='treeName', default="outTree_Nominal", help="Name of trees to be reweighted")


from ROOT import *

def getRecoilPtTree(pathToTrees, treeName):
  files = glob.glob(pathToTrees+"/*.root")
  files = [file for file in files if "ptBinTree" not in file] #don't run over previous ptBinTree files
  treeName = "outTree_Nominal"

  for file in files:

    print "Creating pT Tree file ", file

    inFile = TFile.Open(file, "UPDATE")
    tree = inFile.Get( treeName )
    outFile = TFile.Open(os.path.dirname(file)+"/ptBinTree_"+os.path.basename(file), "RECREATE")

    if "data" in file:
      scaleFactor = 1.
    else:
      for key in inFile.GetListOfKeys():
        if 'cutflow' in key.GetName() and 'weight' not in key.GetName():
          cutFlow = inFile.Get( key.GetName() )
      if not cutFlow:
        print "ERROR, no cutflow file found"
        exit(1)

      scaleFactor = 1./cutFlow.GetBinContent(1)

    newTree = TTree("ptBinTree", "ptBinTree")

    weight = array.array('f', [0])
    recoilPt = array.array('f', [0])
    leadJetPt = array.array('f', [0])
    newTree.Branch("weight", weight, "weight/F")
    newTree.Branch("recoilPt", recoilPt, "recoilPt/F")
    newTree.Branch("leadJetPt", leadJetPt, "leadJetPt/F")

    b_weight = array.array('f', [0])
    b_recoilPt = array.array('f', [0])
    b_jet_pt = std.vector('float')()

    tree.SetBranchStatus('*',0)
    tree.SetBranchStatus( "weight", 1)
    tree.SetBranchStatus( "recoilPt", 1)
    tree.SetBranchStatus( "jet_pt", 1)

    tree.SetBranchAddress("weight", b_weight)
    tree.SetBranchAddress("recoilPt", b_recoilPt)
    tree.SetBranchAddress("jet_pt", b_jet_pt)

    numEv = tree.GetEntries()
    iE = 0;
    while( iE < numEv):
      tree.GetEntry(iE)
      weight[0] = b_weight[0]
      recoilPt[0] = b_recoilPt[0]
      leadJetPt[0] = b_jet_pt[0]
      newTree.Fill()
      iE += 1
#    for entry in tree:
#      print "yes"
#      weight[0] = entry.weight * scaleFactor
#      recoilPt[0] = entry.recoilPt
#      leadJetPt[0] = entry.jet_pt[0]
#      newTree.Fill()

    newTree.Write("", TObject.kOverwrite)
    outFile.Close()

if __name__ == "__main__":
  args = parser.parse_args()
  getRecoilPtTree(args.pathToTrees, args.treeName)
  print "Finished reweightTrees()"
