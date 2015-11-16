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
#include <TH2.h>

#include "/home/jdandoy/Documents/Dijet/MultijetBalanceFW/JES_ResponseFitter/JES_ResponseFitter/JES_BalanceFitter.h"
//#include "JES_ResponseFitter/JES_BalanceFitter.h"

using namespace std;

/** Example program to get JES systematics */
//int main(int argc, char *argv[])
int runFit()
{


//  std::string inFileName = "/home/jdandoy/Documents/Dijet/MultijetBalanceFW/gridOutput/Oct20_area/SystToolOutput/group.phys-exotics.data15_13TeV.00280231.physics_Main.ST_20151020_SystToolOutput.root";
  std::string inFileName = "/home/jdandoy/Documents/Dijet/MultijetBalanceFW/gridOutput/workarea/Oct20_area/workarea/hist.mc.all.appended.root";

  std::size_t pos = inFileName.find("appended");
  std::string outFileName = inFileName;
  outFileName.replace(pos, 8, "fit");
  cout << "Creating Output File " << outFileName << endl;


  // Get binning and systematics from SystToolOutput file //
  TFile *inFile = TFile::Open(inFileName.c_str(), "READ");
  TIter next(inFile->GetListOfKeys());
  TKey *key;

  TFile *outFile = TFile::Open(outFileName.c_str(), "RECREATE");


  // Get Fitting Object
  double NsigmaForFit = 1.6;
  JES_BalanceFitter* m_BalFit = new JES_BalanceFitter(NsigmaForFit);

  while ((key = (TKey*)next() )){
    std::string sysName = key->GetName();
    if( sysName.find("Nominal") == std::string::npos)
      continue;
    // Get Relevant Histograms
    TH2F* h_recoilPt_PtBal = (TH2F*) inFile->Get((sysName+"/recoilPt_PtBal_Fine").c_str());
    TH1F* h_recoilPt_center = (TH1F*) inFile->Get((sysName+"/recoilPt_center").c_str());

    // Get Binning of output histogram
    TH1D* h_template = h_recoilPt_PtBal->ProjectionX();
    h_template->SetName("Template"); h_template->SetTitle("Template");
    for( int iBin=1; iBin < h_template->GetNbinsX()+1; ++iBin){
      h_template->SetBinContent(iBin, 0); h_template->SetBinError(iBin, 0);
    }

    TH1D* h_mean = (TH1D*) h_template->Clone("MJB_Fine");  h_mean->SetTitle("MJB_Fine");

    // Loop over all projections, and fit
    TCanvas* c1 = new TCanvas("c1");
    for( int iBin=1; iBin < h_recoilPt_PtBal->GetNbinsX()+1; ++iBin){
      TH1D* h_proj = h_recoilPt_PtBal->ProjectionY( "h_proj", iBin, iBin, "ed");
      h_proj->Draw();
      c1->Update();
      cout << "iBin " << iBin << endl;
      usleep(100000);

      h_mean->SetBinContent( iBin, h_proj->GetMean() );
      h_mean->SetBinError( iBin, h_proj->GetMeanError() );
    }

    h_mean->Draw();
    sleep(100);



    // Create output directory
    outFile->mkdir(sysName.c_str());
    TDirectoryFile* sysDir = (TDirectoryFile*) outFile->Get(sysName.c_str());
    sysDir->cd();

    // Save Histograms
    h_recoilPt_PtBal->SetDirectory(sysDir); h_recoilPt_PtBal->Write();
    h_recoilPt_center->SetDirectory(sysDir); h_recoilPt_center->Write();
    h_mean->SetDirectory(sysDir); h_mean->Write();
  }

  outFile->Close();
  inFile->Close();


//  std::vector<double> ptBins;
//  std::string sysName = "";
//  int count = 0;
//
//  //Get Pt Bins //
//  TH2DBootstrap* boot = (TH2DBootstrap*) inFile->Get("bootstrap_Nominal");
//  TH1D* hist = (TH1D*) boot->GetNominal();
//  for(int iBin=1; iBin < hist->GetNbinsX()+1; ++iBin){
//    ptBins.push_back( hist->GetXaxis()->GetBinLowEdge(iBin) );
//  }
//  ptBins.push_back( hist->GetXaxis()->GetBinUpEdge(hist->GetNbinsX()) );
//
//  // Get Systematics (Nominal first) //
//  sysNames.push_back("Nominal");
//  while ((key = (TKey*)next() )){
//    sysName = key->GetName();
//    if (sysName.find("Nominal") == std::string::npos){
//      if( count < 5){
//        sysName = sysName.substr(10, sysName.size());
//        cout << "Adding Systematic " << sysName << endl;
//        sysNames.push_back(sysName);
//      }
//      count++;
//    }
//  }
//  inFile->Close();
//
//  TFile *output = TFile::Open("bootstrap.data.all.MJB_initial.root", "RECREATE");
//  SystContainer *sysData = new SystContainer(sysNames, ptBins, 100);
//  sysData->readFromFile(inFileName);
//  SystTool *sysTool = new SystTool(sysNames, ptBins);
//  sysTool->setSystData(sysData);
//  for (unsigned int iSys = 1; iSys < sysNames.size(); ++iSys) {
//    std::cout << "Systematic " << iSys << "/" << sysNames.size() << std::endl;
//    output->mkdir(("Iteration0_"+sysNames.at(iSys)).c_str() );
//    TDirectory* thisDir = (TDirectory*) output->Get( ("Iteration0_"+sysNames.at(iSys)).c_str() );
//    sysTool->runToysJES(sysNames.at(iSys));
//    TH1D* h_sys = (TH1D*) sysTool->getSystHistData("MJB_Fine");
//    h_sys->Draw();
//    c1->Draw();
//    h_sys->SetDirectory(thisDir);
//    output->cd( ("Iteration0_"+sysNames.at(iSys)).c_str() );
//    h_sys->Write();
//    h_sys->Clear();
//  }
//  output->Close();


    return 0;
}
