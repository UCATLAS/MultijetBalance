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
         << "  --file            Path to a file ending in scaled" << std::endl
         << "  --upperEdge       Upper edge for final bin" << std::endl
         << "  --sysType         String tag for which sys to run" << std::endl
         << "  --rebinFileName   Path to rebin file.  No rebinning if this is not set" << std::endl
         << "  --fit             Fit the histograms, rather than retrieving their mean directly" << std::endl
         << std::endl;
    exit(1);
  }

  std::string sysType = "Iteration";
  std::string rebinFileName = "";
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
    } else if (options.at(iArg).compare("--sysType") == 0) {
       char tmpChar = options.at(iArg+1)[0];
       if (iArg+1 == argc || tmpChar == '-' ) {
         std::cout << " --sysType should be followed by a string" << std::endl;
         return 1;
       } else {
         sysType = options.at(iArg+1);
         iArg += 2;
       }
    } else if (options.at(iArg).compare("--upperEdge") == 0) {
       char tmpChar = options.at(iArg+1)[0];
       if (iArg+1 == argc || tmpChar == '-' ) {
         std::cout << " --upperEdge should be followed by a float" << std::endl;
         return 1;
       } else {
         upperEdge = std::stof(options.at(iArg+1));
         iArg += 2;
       }
    } else if (options.at(iArg).compare("--rebinFileName") == 0) {
       char tmpChar = options.at(iArg+1)[0];
       if (iArg+1 == argc || tmpChar == '-' ) {
         std::cout << " --rebinFileName should be followed by a file path" << std::endl;
         return 1;
       } else {
         rebinFileName = options.at(iArg+1);
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

  std::size_t pos = inFileName.find("scaled");

  if( pos == std::string::npos ){
    cout << "Only runs on \"scaled\" files " << endl;
    exit(1);
  }
  std::string outFileName = inFileName;
  if( f_fit )
    outFileName.replace(pos, 6, "fit_MJB_initial");
  else
    outFileName.replace(pos, 6, "mean_MJB_initial");

  if (sysType.size() > 0 && sysType.compare("Iteration") != 0)
    outFileName += ("."+sysType);

  TFile* rebinFile = NULL;
  if (rebinFileName.size() > 0){

    pos = rebinFileName.find("significant");
    if( pos == std::string::npos){
      cout << "Rebinning only accepts \"significant\" files " << endl;
      exit(1);
    }

    rebinFile = TFile::Open(rebinFileName.c_str(), "READ");
    if( rebinFile->IsZombie() ){
      cout << "Error, rebin file " << rebinFileName << " does not exist. Exiting..." << endl;
      exit(1);
    }
  }

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

  TFile *outFile = TFile::Open(outFileName.c_str(), "RECREATE");


  // Get Fitting Object
  double NsigmaForFit = 1.6;
  JES_BalanceFitter* m_BalFit = new JES_BalanceFitter(NsigmaForFit);

  TCanvas* c1 = new TCanvas("c1");
  TLatex *lt = new TLatex();
  lt->SetTextSize(0.04);
  lt->SetNDC();

  int keyCount = 0;
  while ((key = (TKey*)next() )){
    std::string sysName = key->GetName();
    if( sysName.find(sysType) == std::string::npos)
      continue;

    if( sysName.find("MJB_") != std::string::npos)
      continue;
    if( sysName.find("MCType") != std::string::npos)
      continue;

    std::string fitPlotsOutName = outFileName;
    fitPlotsOutName.erase(0, fitPlotsOutName.find_last_of("/"));
    fitPlotsOutName += "_"+sysName;

    TH2F* h_recoilPt_PtBal = (TH2F*) inFile->Get((sysName+"/recoilPt_PtBal").c_str());

    keyCount++;
    cout << "Systematic " << sysName << " (" << keyCount << "/" << nKeys << ")" << endl;

    // Get Binning of output histogram
    TArrayD* xBins = (TArrayD*) h_recoilPt_PtBal->GetXaxis()->GetXbins();
    Double_t* xBinsD = xBins->GetArray();
    int numBins = h_recoilPt_PtBal->GetNbinsX();

    while( upperEdge < xBinsD[numBins]){
      numBins--;
    }
    Double_t final_xBins[numBins];
    for(int iBin = 0; iBin <= numBins; ++iBin){
      if (xBinsD[iBin] < upperEdge)
        final_xBins[iBin] = xBinsD[iBin];
      else
        final_xBins[iBin] = upperEdge;
//cout << "iBin: " << iBin << " : " << final_xBins[iBin] << endl;
    }
    cout << "Setting last bin upper edge to " << final_xBins[numBins] << endl;

    TH1D* h_template = new TH1D("Template", "Template", numBins, final_xBins);

    TH1D* h_mean = (TH1D*) h_template->Clone("MJB");  h_mean->SetTitle("MJB");
    TH1D* h_error = (TH1D*) h_template->Clone("Error");  h_mean->SetTitle("Error");
    TH1D* h_redchi = (TH1D*) h_template->Clone("ReducedChi");  h_mean->SetTitle("ReducedChi");
    TH1D* h_median = (TH1D*) h_template->Clone("Median");  h_mean->SetTitle("Median");
    TH1D* h_width = (TH1D*) h_template->Clone("Width");  h_mean->SetTitle("Width");
    TH1D* h_medianHist = (TH1D*) h_template->Clone("MedianHist");  h_mean->SetTitle("MedianHist");

//    // Original Profile
//    TProfile* prof_MJBcorrection = (TProfile*) h_recoilPt_PtBal->ProfileX("prof_MJBcorrection", 1, -1, "");
//    TH1D* h_mean_prof = (TH1D*) prof_MJBcorrection->ProjectionX("h_mean_prof");

    vector<int> binsToCombine;
    if(rebinFile){
    //if(rebinFile && (sysName.find("MCType") == std::string::npos) ){
      std::string thisSysType;
      if (sysName.find("MCType") != std::string::npos){
        thisSysType = "significant_Nominal";
      }else{
        thisSysType = "significant_"+sysName.substr(11, sysName.size());
      }
      cout << "Rebin histogram is: " << thisSysType << endl;
      TH1D* rebinHist = (TH1D*) rebinFile->Get(thisSysType.c_str());

      // Get Binning of rebin histogram
      TArrayD* xBinsRebin = (TArrayD*) rebinHist->GetXaxis()->GetXbins();
      Double_t* xBinsDRebin = xBinsRebin->GetArray();
      int rebin_numBins = rebinHist->GetNbinsX();

      while( upperEdge < xBinsDRebin[rebin_numBins]){
        rebin_numBins--;
      }

      //Check that starting point is same between the two !!

      //Find mapping between actual hist and rebin hist
      //each index of binsToRebin will correspond to 1 fit (and to 1 bin of the rebin hist)
      //each element of binsToRebin will be the end bin to fit to, with the previous element being the starting bin
      int iRebin = 0;
      for(int iBin = 0; iBin <= numBins; ++iBin){
        if( xBinsDRebin[iRebin] == final_xBins[iBin]){
          binsToCombine.push_back( iBin );
          iRebin++;
        }
      }
//      for( int iBin = 0; iBin < binsToCombine.size(); ++iBin){
//        cout << iBin << " : " << binsToCombine.at(iBin) << endl;
//        cout << xBinsDRebin[iBin] << " : " << final_xBins[binsToCombine.at(iBin)] << endl;
//      }
//      exit(1);
    }else{
      for(int iBin = 0; iBin <= numBins; ++iBin){
        binsToCombine.push_back( iBin );
      }
//      for( int iBin = 0; iBin < binsToCombine.size(); ++iBin){
//        cout << iBin << " : " << binsToCombine.at(iBin) << endl;
//      }
//      exit(1);
    }
    // Loop over all projections, and fit
    //for( int iBin=1; iBin < h_recoilPt_PtBal->GetNbinsX()+1; ++iBin){
    for( unsigned int iRange=1; iRange < binsToCombine.size(); ++iRange){
      int iBin_start = binsToCombine.at(iRange-1)+1;
      int iBin_end = binsToCombine.at(iRange);
      cout << iRange << " : " << iBin_start << " : " << iBin_end << endl;
      TH1D* h_proj = h_recoilPt_PtBal->ProjectionY( "h_proj", iBin_start, iBin_end, "ed");
      if (h_proj->GetEntries() < 1)
        continue;

      float thisMean = 0., thisError = 0., thisRedChi = 0., thisMedian = 0., thisWidth = 0., thisMedianHist = 0.;

      if( f_fit ){
        m_BalFit->Fit(h_proj, 0); // Rebin histogram and fit
//Refit       m_BalFit->ReFit(h_proj, 0); // Rebin histogram and fit
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
    }

    h_mean->Draw();
    c1->Update();
    c1->SaveAs( (fitPlotsOutDir+fitPlotsOutName+"_preprofMJB.png").c_str() );



    // Create output directory
    outFile->mkdir(sysName.c_str());
    TDirectoryFile* sysDir = (TDirectoryFile*) outFile->Get(sysName.c_str());
    sysDir->cd();

    // Save Histograms
    h_recoilPt_PtBal->SetDirectory(sysDir); h_recoilPt_PtBal->Write();
//    h_recoilPt_center->SetDirectory(sysDir); h_recoilPt_center->Write();
    h_mean->SetDirectory(sysDir); h_mean->Write();
    if (f_fit){
      h_error->SetDirectory(sysDir); h_error->Write();
      h_redchi->SetDirectory(sysDir); h_redchi->Write();
      h_median->SetDirectory(sysDir); h_median->Write();
      h_width->SetDirectory(sysDir); h_width->Write();
      h_medianHist->SetDirectory(sysDir); h_medianHist->Write();
    }

    c1->Clear();
    h_template->Delete();
    h_recoilPt_PtBal->Delete();
    h_mean->Delete();
    h_error->Delete();
    h_redchi->Delete();
    h_median->Delete();
    h_width->Delete();
    h_medianHist->Delete();


  }

  outFile->Close();
  inFile->Close();


  std::cout << "Finished Fitting after " << (std::time(0) - initialTime) << " seconds" << std::endl;

    return 0;
}
