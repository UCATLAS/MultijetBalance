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
#include <sys/stat.h>


//#include "/home/jdandoy/Documents/Dijet/MultijetBalanceFW/JES_ResponseFitter/JES_ResponseFitter/JES_BalanceFitter.h"
#include "JES_ResponseFitter/JES_BalanceFitter.h"

using namespace std;

int main(int argc, char *argv[])
{
  std::time_t initialTime = std::time(0);
  gErrorIgnoreLevel = 2000;
  std::string inFileName = "";
  std::string histName = "";
  float upperEdge = 0;

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
         << "  --file            Path to a file" << std::endl
         << "  --hist            Histogram name" << std::endl
         << "  --fit             Fit the histograms, rather than retrieving their mean directly" << std::endl
         << std::endl;
    exit(1);
  }

  bool f_fit = false;

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
    } else if (options.at(iArg).compare("--hist") == 0) {
       char tmpChar = options.at(iArg+1)[0];
       if (iArg+1 == argc || tmpChar == '-' ) {
         std::cout << " --hist should be followed by a histogram name" << std::endl;
         return 1;
       } else {
         histName = options.at(iArg+1);
         iArg += 2;
       }
    } else if (options.at(iArg).compare("--fit") == 0) {
      f_fit = true;
      ++iArg;
    }else{
      std::cout << "Couldn't understand argument " << options.at(iArg) << std::endl;
      return 1;
    }
  }//while arguments


  if ( inFileName.size() == 0){
    cout << "No input file given " << endl;
    exit(1);
  }

  std::size_t pos = inFileName.find(".root");

  if( pos == std::string::npos ){
    cout << "Only runs on \"root\" files " << endl;
    exit(1);
  }
  std::string outFileName = inFileName;
  if( f_fit )
    outFileName.replace(pos, 5, ".fit_MJB_initial.root");
  else
    outFileName.replace(pos, 5, ".mean_MJB_initial.root");


  cout << "Creating Output File " << outFileName << endl;

  std::string fitPlotsOutDir = outFileName;
  fitPlotsOutDir.erase(fitPlotsOutDir.find_last_of("/"));
  fitPlotsOutDir += "/fits/";
  mkdir(fitPlotsOutDir.c_str(), 0777);


  // Get binning and systematics from SystToolOutput file //
  TFile *inFile = TFile::Open(inFileName.c_str(), "READ");
  TIter next(inFile->GetListOfKeys());
  TKey *key;
  int nKeys = inFile->GetNkeys();

  TFile *outFile = TFile::Open(outFileName.c_str(), "UPDATE");
  outFile->mkdir("Nominal");
  outFile->cd("Nominal");


  // Get Fitting Object
  double NsigmaForFit = 1.6;
  JES_BalanceFitter* m_BalFit = new JES_BalanceFitter(NsigmaForFit);

  TCanvas* c1 = new TCanvas("c1");
  TLatex *lt = new TLatex();
  lt->SetTextSize(0.04);
  lt->SetNDC();


  std::string fitPlotsOutName = outFileName;
  fitPlotsOutName.erase(0, fitPlotsOutName.find_last_of("/"));
  fitPlotsOutName += "_"+histName;

  TH2F* h_recoilPt_PtBal = (TH2F*) inFile->Get((histName).c_str());

  // Original Profile
  TProfile* prof_MJBcorrection = (TProfile*) h_recoilPt_PtBal->ProfileY("prof_MJBcorrection", 1, -1, "");
  TH1D* h_mean_prof = (TH1D*) prof_MJBcorrection->ProjectionX("h_mean_prof");

  TH1D* h_mean = (TH1D*) h_mean_prof->Clone(       ("MJB_"+histName).c_str() );  h_mean->SetTitle("MJB");
  TH1D* h_error = (TH1D*) h_mean_prof->Clone(      ("Error_"+histName).c_str());  h_mean->SetTitle("Error");
  TH1D* h_redchi = (TH1D*) h_mean_prof->Clone(     ("ReducedChi_"+histName).c_str());  h_mean->SetTitle("ReducedChi");
  TH1D* h_median = (TH1D*) h_mean_prof->Clone(     ("Median_"+histName).c_str());  h_mean->SetTitle("Median");
  TH1D* h_width = (TH1D*) h_mean_prof->Clone(      ("Width_"+histName).c_str());  h_mean->SetTitle("Width");
  TH1D* h_medianHist = (TH1D*) h_mean_prof->Clone( ("MedianHist_"+histName).c_str());  h_mean->SetTitle("MedianHist");


  // Loop over all projections, and fit
  for( unsigned int iRange=1; iRange < h_mean_prof->GetNbinsX()+1; ++iRange){
    int iBin_start = iRange;
    int iBin_end = iRange;
    TH1D* h_proj = h_recoilPt_PtBal->ProjectionX( "h_proj", iBin_start, iBin_end, "ed");
    if (h_proj->GetEntries() < 1)
      continue;

    float thisMean = 0., thisError = 0., thisRedChi = 0., thisMedian = 0., thisWidth = 0., thisMedianHist = 0.;

    if( f_fit ){
      m_BalFit->Fit(h_proj, 0); // Rebin histogram and fit
      thisMean = m_BalFit->GetMean();
      thisError = m_BalFit->GetMeanError();
    } else {
      thisMean = h_proj->GetMean();
      thisError = h_proj->GetMeanError();
    }

    //Save nominal results
    for(int iBin = iBin_start; iBin <= iBin_end; ++iBin){
      h_mean->SetBinContent( iBin, thisMean );
      h_mean->SetBinError( iBin, thisError );
    }

    //Save extra fit information
    if( f_fit ){

      TF1* thisFit = (TF1*) m_BalFit->GetFit();
      TH1D* thisHisto = (TH1D*) m_BalFit->GetHisto();
      thisHisto->Draw();
      thisFit->Draw("same");


      thisMedian = m_BalFit->GetMedian();
      thisRedChi = m_BalFit->GetChi2Ndof();
      thisWidth = m_BalFit->GetSigma();
      if (thisError < 0.5)
        thisMedianHist = m_BalFit->GetHistoMedian();
      else
        thisMedianHist = -99.0;

      //output histogram will have the same binning as the final result
      for(int iBin = iBin_start; iBin <= iBin_end; ++iBin){
        h_error->SetBinContent(iBin, thisError );
        h_median->SetBinContent(iBin, thisMedian);
        h_redchi->SetBinContent(iBin, thisRedChi);
        h_width->SetBinContent(iBin, thisWidth);
        h_medianHist->SetBinContent(iBin, thisMedianHist);
      }

      float ltx = 0.62;
      float lty = 0.80;

      char name[200];

      sprintf(name, "Bins: %i to %i", iBin_start, iBin_end);
      lt->DrawLatex(ltx,lty,name);
      lty -= 0.05;

      sprintf(name, "pT: %.0f %.0f", h_recoilPt_PtBal->GetXaxis()->GetBinLowEdge(iBin_start), h_recoilPt_PtBal->GetXaxis()->GetBinUpEdge(iBin_end));
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
      c1->SaveAs( (fitPlotsOutDir+fitPlotsOutName+"_"+to_string(iRange)+".png").c_str() );
    }// if f_fit
    h_proj->Delete();
  }//loop over bins

  h_mean->Draw();
  c1->Update();
  c1->SaveAs( (fitPlotsOutDir+fitPlotsOutName+"_preprofMJB.png").c_str() );


  h_recoilPt_PtBal->Write();

  // Save Histograms
  h_recoilPt_PtBal->Write();
  h_mean->Write();
  if (f_fit){
    h_error->Write();
    h_redchi->Write();
    h_median->Write();
    h_width->Write();
    h_medianHist->Write();
  }

  c1->Clear();
  h_recoilPt_PtBal->Delete();
  h_mean->Delete();
  h_error->Delete();
  h_redchi->Delete();
  h_median->Delete();
  h_width->Delete();
  h_medianHist->Delete();


  outFile->Close();
  inFile->Close();


  std::cout << "Finished Fitting after " << (std::time(0) - initialTime) << " seconds" << std::endl;

    return 0;
}
