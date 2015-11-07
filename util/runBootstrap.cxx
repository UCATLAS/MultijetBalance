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

#include "SystTool/SystTool.h"
#include "SystTool/SystContainer.h"

using namespace std;

vector<double> vectorize(string bins)
{
    istringstream stream(bins);
    string bin;

    vector<double> output;

    while (getline(stream, bin, ',')) {
        output.push_back( atof(bin.c_str()) );
    }

    return output;
}

vector<string> vectorizeStr(string bins)
{
    istringstream stream(bins);
    string bin;

    vector<string> output;

    while (getline(stream, bin, ',')) {
        while (bin[0] == ' ') bin.erase(bin.begin());
        output.push_back(bin);
    }

    return output;
}

/** Example program to get JES systematics */
int main(int argc, char *argv[])
{

//  std::string inFileName = "/home/jdandoy/Documents/Dijet/MultijetBalanceFW/gridOutput/Oct20_area/SystToolOutput/group.phys-exotics.data15_13TeV.00280231.physics_Main.ST_20151020_SystToolOutput.root";
  std::string inFileName = "/home/jdandoy/Documents/Dijet/MultijetBalanceFW/gridOutput/workarea/Nov5_bootstrap_area/SystToolOutput/allData.root";

  TCanvas* c1 = new TCanvas();

  // Get binning and systematics from SystToolOutput file //
  TFile *inFile = TFile::Open(inFileName.c_str(), "READ");
  TIter next(inFile->GetListOfKeys());
  TKey *key;

  std::vector<std::string> sysNames;
  std::vector<double> ptBins;
  std::string sysName = "";
  int count = 0;

  //Get Pt Bins //
  TH2DBootstrap* boot = (TH2DBootstrap*) inFile->Get("bootstrap_Nominal");
  TH1D* hist = (TH1D*) boot->GetNominal();
  for(int iBin=1; iBin < hist->GetNbinsX()+1; ++iBin){
    ptBins.push_back( hist->GetXaxis()->GetBinLowEdge(iBin) );
  }
  ptBins.push_back( hist->GetXaxis()->GetBinUpEdge(hist->GetNbinsX()) );

  // Get Systematics (Nominal first) //
  sysNames.push_back("Nominal");
  while ((key = (TKey*)next() )){
    sysName = key->GetName();
    if (sysName.find("Nominal") == std::string::npos){
      if( count < 5){
        sysName = sysName.substr(10, sysName.size());
        cout << "Adding Systematic " << sysName << endl;
        sysNames.push_back(sysName);
      }
      count++;
    }
  }
  inFile->Close();

  TFile *output = TFile::Open("bootstrap.data.all.MJB_initial.root", "RECREATE");
  SystContainer *sysData = new SystContainer(sysNames, ptBins, 100);
  sysData->readFromFile(inFileName);
  SystTool *sysTool = new SystTool(sysNames, ptBins);
  sysTool->setSystData(sysData);
  for (unsigned int iSys = 1; iSys < sysNames.size(); ++iSys) {
    std::cout << "Systematic " << iSys << "/" << sysNames.size() << std::endl;
    output->mkdir(("Iteration0_"+sysNames.at(iSys)).c_str() );
    TDirectory* thisDir = (TDirectory*) output->Get( ("Iteration0_"+sysNames.at(iSys)).c_str() );
    sysTool->runToysJES(sysNames.at(iSys));
    TH1D* h_sys = (TH1D*) sysTool->getSystHistData("MJB_Fine");
    h_sys->Draw();
    c1->Draw();
    h_sys->SetDirectory(thisDir);
    output->cd( ("Iteration0_"+sysNames.at(iSys)).c_str() );
    h_sys->Write();
    h_sys->Clear();
  }
  output->Close();


//    for (unsigned int iSyst = 1; iSyst <= 1; ++iSyst) { //loop over systematics
//        cout << systNames.at(iSyst) << endl;
//
//        sTool->runToysJES(systNames.at(iSyst));
//
//        TH1D* h_syst = sTool->getSystHist(jetAlgo.at(iAlg) + "_" + systNames.at(iSyst));
//
//        output->cd();
//        h_syst->Write();
//
//       // Additional histograms
//
//       // MC/Data systematics
//       TH1D *h_systMC = sTool->getSystHistMC(jetAlgo.at(iAlg) + "_" + systNames.at(iSyst));
//       TH1D *h_systData = sTool->getSystHistData(jetAlgo.at(iAlg) + "_" + systNames.at(iSyst));
//
//       output->cd();
//       h_systMC->Write();
//       h_systData->Write();
//
//       // Toys histograms
//       vector<TH1D*>* toys = sTool->getToysHist(jetAlgo.at(iAlg) + "_" + systNames.at(iSyst));
//       output->cd();
//       for (unsigned int i = 0; i < toys->size(); ++i) {
//           toys->at(i)->Write();
//       }
//
//       vector<TH1D*>* toysMC = sTool->getToysHistMC(jetAlgo.at(iAlg) + "_" + systNames.at(iSyst));
//       vector<TH1D*>* toysData = sTool->getToysHistData(jetAlgo.at(iAlg) + "_" + systNames.at(iSyst));
//
//       output->cd();
//       for (unsigned int i = 0; i < toysMC->size(); ++i) {
//           toysMC->at(i)->Write();
//           toysData->at(i)->Write();
//       }
//
//    }
//
//    delete sTool;
//    delete systData;
//
//    output->Close();
//
    return 0;
}
