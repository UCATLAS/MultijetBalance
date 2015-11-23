#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <unistd.h>

#include <TFile.h>
#include <TProfile.h>
#include <TLatex.h>
#include <TCanvas.h>
#include <TKey.h>
#include <TH1.h>
#include <TH2.h>
#include <TROOT.h>
#include <sys/time.h>


//#include "/home/jdandoy/Documents/Dijet/MultijetBalanceFW/JES_ResponseFitter/JES_ResponseFitter/JES_BalanceFitter.h"
#include "JES_ResponseFitter/JES_BalanceFitter.h"

using namespace std;

int main(int argc, char *argv[])
//int runFit()
{
  std::time_t initialTime = std::time(0);
  gErrorIgnoreLevel = 2000;
  std::string inFileName = "";
  bool f_nominal = false;
  /////////// Retrieve runFit's arguments //////////////////////////
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
         << "  --file            Path to a file ending in appended" << std::endl
         << "  --nominal         Only Nominal Hists" << std::endl
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
    } else if (options.at(iArg).compare("--nominal") == 0) {
      f_nominal = true;
      iArg++;
    }else{
      std::cout << "Couldn't understand argument " << options.at(iArg) << std::endl;
      return 1;
    }
  }//while arguments


//  std::string inFileName = "/home/jdandoy/Documents/Dijet/MultijetBalanceFW/gridOutput/Oct20_area/SystToolOutput/group.phys-exotics.data15_13TeV.00280231.physics_Main.ST_20151020_SystToolOutput.root";
//  std::string inFileName = "/home/jdandoy/Documents/Dijet/MultijetBalanceFW/gridOutput/workarea/Oct20_area/workarea/hist.data.all.appended.root";
//
cout << "here" << endl;
  if ( inFileName.size() == 0){
    cout << "No input file given " << endl;
    exit(1);
  }

  std::size_t pos = inFileName.find("appended");

  if( pos == std::string::npos ){
    cout << "Only runs on appended files " << endl;
    exit(1);
  }
  std::string outFileName = inFileName;
  outFileName.replace(pos, 8, "fit_MJB_initial");
  cout << "Creating Output File " << outFileName << endl;

  std::string fitPlotsOutName = outFileName;
  fitPlotsOutName.erase(0, fitPlotsOutName.find_last_of("/"));

  // Get binning and systematics from SystToolOutput file //
  TFile *inFile = TFile::Open(inFileName.c_str(), "READ");
  TIter next(inFile->GetListOfKeys());
  TKey *key;

  TFile *outFile = TFile::Open(outFileName.c_str(), "RECREATE");


  // Get Fitting Object
  double NsigmaForFit = 1.6;
  JES_BalanceFitter* m_BalFit = new JES_BalanceFitter(NsigmaForFit);

  TCanvas* c1 = new TCanvas("c1");
  while ((key = (TKey*)next() )){
    std::string sysName = key->GetName();
    if( f_nominal && sysName.find("Nominal") == std::string::npos)
      continue;
    // Get Relevant Histograms
    TH2F* h_recoilPt_PtBal = (TH2F*) inFile->Get((sysName+"/recoilPt_PtBal_Fine").c_str());
    TH1F* h_recoilPt_center = (TH1F*) inFile->Get((sysName+"/recoilPt_center").c_str());

    // Get Binning of output histogram
    TArrayD* xBins = (TArrayD*) h_recoilPt_PtBal->GetXaxis()->GetXbins();
    Double_t* xBinsD = xBins->GetArray();
    int numBins = h_recoilPt_PtBal->GetNbinsX();
    xBinsD[numBins] = 3100;
    cout << "last bin " << xBinsD[numBins] << endl;
    TH1D* h_template = new TH1D("Template", "Template", numBins, xBinsD);
    for( int iBin=1; iBin < h_template->GetNbinsX()+1; ++iBin){
      cout << "Bin edge " << iBin << " is " << h_template->GetXaxis()->GetBinLowEdge(iBin) << endl;
    }

//    TH1D* h_template = h_recoilPt_PtBal->ProjectionX();
//    h_template->SetName("Template"); h_template->SetTitle("Template");
//    for( int iBin=1; iBin < h_template->GetNbinsX()+1; ++iBin){
//      h_template->SetBinContent(iBin, 0); h_template->SetBinError(iBin, 0);
//    }

    TH1D* h_mean = (TH1D*) h_template->Clone("MJB_Fine");  h_mean->SetTitle("MJB_Fine");
    TH1D* h_error = (TH1D*) h_template->Clone("Error");  h_mean->SetTitle("Error");
    TH1D* h_redchi = (TH1D*) h_template->Clone("ReducedChi");  h_mean->SetTitle("ReducedChi");
    TH1D* h_median = (TH1D*) h_template->Clone("Median");  h_mean->SetTitle("Median");
    TH1D* h_width = (TH1D*) h_template->Clone("Width");  h_mean->SetTitle("Width");
    TH1D* h_medianHist = (TH1D*) h_template->Clone("MedianHist");  h_mean->SetTitle("MedianHist");

//    // Original Profile
//    TProfile* prof_MJBcorrection = (TProfile*) h_recoilPt_PtBal->ProfileX("prof_MJBcorrection", 1, -1, "");
//    TH1D* h_mean_prof = (TH1D*) prof_MJBcorrection->ProjectionX("h_mean_prof");


    // Loop over all projections, and fit
    for( int iBin=1; iBin < h_recoilPt_PtBal->GetNbinsX()+1; ++iBin){
      TH1D* h_proj = h_recoilPt_PtBal->ProjectionY( "h_proj", iBin, iBin, "ed");
      m_BalFit->Fit(h_proj, 0); // Rebin histogram and fit
      m_BalFit->ReFit(h_proj, 0); // Rebin histogram and fit
      TF1* thisFit = (TF1*) m_BalFit->GetFit();
      TH1D* thisHisto = (TH1D*) m_BalFit->GetHisto();
      thisHisto->Draw();
      thisFit->Draw("same");
//      cout << "iBin " << iBin << endl;
//      usleep(100000);


      float thisMean = 0., thisError = 0., thisRedChi = 0., thisMedian = 0., thisWidth = 0., thisMedianHist = 0.;

      thisMean = m_BalFit->GetMean();
      thisError = m_BalFit->GetMeanError();
      h_mean->SetBinContent( iBin, thisMean );
      h_mean->SetBinError( iBin, thisError );
      h_error->SetBinContent(iBin, thisError );

      thisMedian = m_BalFit->GetMedian();
      h_median->SetBinContent(iBin, thisMedian);
      thisRedChi = m_BalFit->GetChi2Ndof();
      h_redchi->SetBinContent(iBin, thisRedChi);
      thisWidth = m_BalFit->GetSigma();
      h_width->SetBinContent(iBin, thisWidth);

      if (thisError < 0.5){
        thisMedianHist = m_BalFit->GetHistoMedian();
        h_medianHist->SetBinContent(iBin, thisMedianHist);
      }else
        h_medianHist->SetBinContent(iBin, -1.);


//      if (thisError <= 0.001 && thisRedChi < 3){
//        h_mean->SetBinContent( iBin, thisMean );
//        h_mean->SetBinError( iBin, thisError );
//      } else {
//        h_mean->SetBinContent( iBin, h_proj->GetMean() );
//        h_mean->SetBinError( iBin, h_proj->GetMeanError() );
//      }

      TLatex *lt = new TLatex();
      lt->SetTextSize(0.04);
      lt->SetNDC();

      float ltx = 0.62;
      float lty = 0.80;

      char name[200];

      sprintf(name, "Bin: %i", iBin);
      lt->DrawLatex(ltx,lty,name);
      lty -= 0.05;

      sprintf(name, "pT: %.0f %.0f", h_recoilPt_PtBal->GetXaxis()->GetBinLowEdge(iBin), h_recoilPt_PtBal->GetXaxis()->GetBinUpEdge(iBin));
      lt->DrawLatex(ltx,lty,name);
      lty -= 0.05;

      sprintf(name,"Mean: %.3f", thisMean);
      lt->DrawLatex(ltx,lty,name);
      lty -= 0.05;

      sprintf(name,"Fit Median: %.3f", thisMedian);
      lt->DrawLatex(ltx,lty,name);
      lty -= 0.05;

      if (thisError < 0.5){
        sprintf(name,"Hist Median: %.3f", thisMedianHist);
        lt->DrawLatex(ltx,lty,name);
        lty -= 0.05;
      }

      sprintf(name,"Error: %.4f", thisError);
      lt->DrawLatex(ltx,lty,name);
      lty -= 0.05;

      sprintf(name,"RedChi2: %.2f", thisRedChi);
      lt->DrawLatex(ltx,lty,name);
      lty -= 0.05;

      sprintf(name,"Width: %.2f", thisWidth);
      lt->DrawLatex(ltx,lty,name);
      lty -= 0.05;

      sprintf(name,"Projection Mean: %.3f", h_proj->GetMean());
      lt->DrawLatex(ltx,lty,name);
      lty -= 0.05;

      sprintf(name,"Projection Error: %.4f", h_proj->GetMeanError());
      lt->DrawLatex(ltx,lty,name);
      lty -= 0.05;

      c1->Update();
      c1->SaveAs( ("fits/"+fitPlotsOutName+"_"+to_string(iBin)+".png").c_str() );
    }

    h_mean->Draw();
    c1->Update();
    c1->SaveAs( ("fits/"+fitPlotsOutName+"_preprofMJB.png").c_str() );



    // Create output directory
    outFile->mkdir(sysName.c_str());
    TDirectoryFile* sysDir = (TDirectoryFile*) outFile->Get(sysName.c_str());
    sysDir->cd();

    // Save Histograms
    h_recoilPt_PtBal->SetDirectory(sysDir); h_recoilPt_PtBal->Write();
    h_recoilPt_center->SetDirectory(sysDir); h_recoilPt_center->Write();
    h_mean->SetDirectory(sysDir); h_mean->Write();
    h_error->SetDirectory(sysDir); h_error->Write();
    h_redchi->SetDirectory(sysDir); h_redchi->Write();
    h_median->SetDirectory(sysDir); h_median->Write();
    h_width->SetDirectory(sysDir); h_width->Write();
    h_medianHist->SetDirectory(sysDir); h_medianHist->Write();

    c1->Clear();
  }

  outFile->Close();
  inFile->Close();


  std::cout << "Finished Fitting after " << (std::time(0) - initialTime) << " seconds" << std::endl;

    return 0;
}
