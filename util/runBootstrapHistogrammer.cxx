#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <unistd.h>

#include <TFile.h>
#include <TCanvas.h>
#include <TKey.h>
#include <TH1.h>
#include <TH2D.h>

#include "BootstrapGenerator/BootstrapGenerator.h"
#include "BootstrapGenerator/TH2DBootstrap.h"

using namespace std;

/** Example program to get JES systematics */
int main(int argc, char *argv[])
{

//  std::string inFileName = "/home/jdandoy/Documents/Dijet/MultijetBalanceFW/gridOutput/Oct20_area/SystToolOutput/group.phys-exotics.data15_13TeV.00280231.physics_Main.ST_20151020_SystToolOutput.root";
  std::string inFileName = ""; // /home/jdandoy/Documents/Dijet/MultijetBalanceFW/gridOutput/workarea/Nov5_bootstrap_area/SystToolOutput/allData.root";
  unsigned int nToys = 100;

  /////////// Retrieve getBootstrap's arguments //////////////////////////
  std::vector< std::string> options;
  for(int ii=1; ii < argc; ++ii){
    options.push_back( argv[ii] );
  }

  if (argc > 1 && options.at(0).compare("-h") == 0) {
    std::cout << std::endl
         << " runDijetResonance : DijetResonance job submission" << std::endl
         << std::endl
         << " Optional arguments:" << std::endl
         << "  -h                Prints this menu" << std::endl
         << "  --file            Path to a bootstrap file" << std::endl
//         << "  --nominal         Only Nominal Hists" << std::endl
         << std::endl;
    exit(1);
  }

  int iArg = 0;
  while(iArg < argc-1) {
    if (options.at(iArg).compare("-h") == 0) {
       // Ignore if not first argument
       ++iArg;
    } else if (options.at(iArg).compare("--file") == 0) {
       char tmpChar = options.at(iArg+1)[0];
       if (iArg+1 == argc || tmpChar == '-' ) {
         std::cout << " --file should be followed by a file or folder" << std::endl;
         return 1;
       } else {
         inFileName = options.at(iArg+1);
         iArg += 2;
       }
    }else{
      std::cout << "Couldn't understand argument " << options.at(iArg) << std::endl;
      return 1;
    }
  }//while arguments


  std::string fileDir = inFileName.substr(0, inFileName.find_last_of("/") );
  cout << "saving to fileDir " << fileDir << endl;

  // Get binning and systematics from SystToolOutput file //
  TFile *inFile = TFile::Open(inFileName.c_str(), "READ");
  TIter next(inFile->GetListOfKeys());
  TKey *key;

  std::vector<std::string> sysNames;
  std::vector<double> ptBins;
  std::string sysName = "";

  // Get Systematics (Nominal first) //
  sysNames.push_back("Nominal");
  while ((key = (TKey*)next() )){
    sysName = key->GetName();
    if (sysName.find("Nominal") == std::string::npos){
      sysName = sysName.substr(10, sysName.size());
      cout << "Adding Systematic " << sysName << endl;
      sysNames.push_back(sysName);
    }
  }

  TFile *output = TFile::Open((fileDir+"/hist.data.bootstrap.scaled.root").c_str(), "RECREATE");
  for (unsigned int iSys = 0; iSys < sysNames.size(); ++iSys) {
    std::cout << "Systematic " << sysNames.at(iSys) << ": " << iSys << "/" << sysNames.size() << std::endl;

    TH2DBootstrap* bootStrap = (TH2DBootstrap*) inFile->Get( ("bootstrap_"+sysNames.at(iSys)).c_str());
    for( unsigned int iT = 0; iT < nToys; ++iT){
      std::string dirName = "Iteration0_"+sysNames.at(iSys)+"_"+to_string(iT);
      output->mkdir( dirName.c_str() );
      TDirectory* thisDir = (TDirectory*) output->Get( dirName.c_str() );
      TH2D* thisHist = (TH2D*) bootStrap->GetReplica(iT);
      thisHist->SetName("recoilPt_PtBal_Fine");
      thisHist->SetTitle("recoilPt_PtBal_Fine");
      thisHist->SetDirectory(thisDir);
      output->cd( dirName.c_str() );
      thisHist->Write();
      thisHist->Clear();

    }
  }
  output->Close();
  inFile->Close();

  return 0;
}
