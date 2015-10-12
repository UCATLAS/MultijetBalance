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
    newTree.Branch("weight", weight, "weight/F")
    newTree.Branch("recoilPt", recoilPt, "recoilPt/F")
    for entry in tree:
      weight[0] = entry.weight * scaleFactor
      recoilPt[0] = entry.recoilPt
      newTree.Fill()

    newTree.Write("", TObject.kOverwrite)
    outFile.Close()

if __name__ == "__main__":
  args = parser.parse_args()
  getRecoilPtTree(args.pathToTrees, args.treeName)
  print "Finished reweightTrees()"
