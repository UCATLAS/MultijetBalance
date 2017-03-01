//////////////////////////////////////////////////////////////////
// runBootstrapFit.cxx
//////////////////////////////////////////////////////////////////
// Run fits on histograms made from bootstrap toys.
// Allows rebinning based on RMS of bootstrap toy fits.
//////////////////////////////////////////////////////////////////
// jeff.dandoy@cern.ch
//////////////////////////////////////////////////////////////////

#include <vector>
#include <iostream>
#include <string>
#include <sys/time.h>
#include <sys/stat.h>

#include <TFile.h>
#include <TLatex.h>
#include <TCanvas.h>
#include <TKey.h>
#include <TH1.h>
#include <TH2.h>

#include "JES_ResponseFitter/JES_BalanceFitter.h"

using namespace std;

///// Function for plotting fitted distributions //////////////
int SaveCanvas(int startBin, int endBin, string cName, JES_BalanceFitter* m_BalFit){
  TCanvas c1;// = new TCanvas("c1");
  TLatex lt;// = new TLatex();
  lt.SetTextSize(0.04);
  lt.SetNDC();

  c1.cd();
  TF1* thisFit = (TF1*) m_BalFit->GetFit();
  TH1D* thisHisto = (TH1D*) m_BalFit->GetHisto();
  thisHisto->Draw();
  thisFit->Draw("same");

  double thisMean = 0., thisError = 0., thisRedChi = 0., thisMedian = 0., thisWidth = 0., thisMedianHist = 0.;

  thisMean = m_BalFit->GetMean();
  thisError = m_BalFit->GetMeanError();
  thisMedian = m_BalFit->GetMedian();
  thisRedChi = m_BalFit->GetChi2Ndof();
  thisWidth = m_BalFit->GetSigma();

  if (thisError < 0.5)
    thisMedianHist = m_BalFit->GetHistoMedian();
  else
    thisMedianHist = -99.;

  float ltx = 0.62;
  float lty = 0.80;

  char name[200];

  sprintf(name, "Bins: %i to %i", startBin, endBin);
  lt.DrawLatex(ltx,lty,name);
  lty -= 0.05;

  sprintf(name, "pT: %.0f to %.0f", thisHisto->GetXaxis()->GetBinLowEdge(startBin), thisHisto->GetXaxis()->GetBinUpEdge(endBin));
  lt.DrawLatex(ltx,lty,name);
  lty -= 0.05;

  sprintf(name,"Mean: %.3f", thisMean);
  lt.DrawLatex(ltx,lty,name);
  lty -= 0.05;

  sprintf(name,"Fit Median: %.3f", thisMedian);
  lt.DrawLatex(ltx,lty,name);
  lty -= 0.05;

  sprintf(name,"Hist Median: %.3f", thisMedianHist);
  lt.DrawLatex(ltx,lty,name);
  lty -= 0.05;

  sprintf(name,"Error: %.4f", thisError);
  lt.DrawLatex(ltx,lty,name);
  lty -= 0.05;

  sprintf(name,"RedChi2: %.2f", thisRedChi);
  lt.DrawLatex(ltx,lty,name);
  lty -= 0.05;

  sprintf(name,"Width: %.2f", thisWidth);
  lt.DrawLatex(ltx,lty,name);
  lty -= 0.05;

  c1.Update();
  c1.SaveAs( cName.c_str() );

return 0;
}

inline bool isInteger(const std::string & s){
  if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

  char * p ;
  strtol(s.c_str(), &p, 10) ;

  return (*p == 0) ;
}

//// This is for when the nominal was rebinned.  No longer needed
TH2F* initialRebin( TH2F* inputHist ){


  std::string histName = inputHist->GetName();
  inputHist->SetName( ("tmp_"+histName).c_str() );
//  double binArray[] = {300, 360, 420, 480, 540, 600, 660, 720, 780, 840, 900, 960, 1020, 1140, 1260, 1480, 2000};
//  int nBins = 16;
//  Double_t ptBalBins[501];
//  int numPtBalBins = 500;
//  for(int i=0; i < numPtBalBins+1; ++i){
//    ptBalBins[i] = i/100.;
//  }
//
//  TH2F* newHist = new TH2F( histName.c_str(), inputHist->GetTitle(), nBins, binArray, numPtBalBins, ptBalBins);
//  newHist->Sumw2();
//  for(int iBinX=1; iBinX < inputHist->GetNbinsX()+1; ++iBinX){
//    for(int iBinY=1; iBinY < inputHist->GetNbinsY()+1; ++iBinY){
//      newHist->Fill( inputHist->GetXaxis()->GetBinLowEdge(iBinX)+0.0001, inputHist->GetYaxis()->GetBinLowEdge(iBinY)+0.0001, inputHist->GetBinContent(iBinX, iBinY) );
//    }
//  }
  TH2F* newHist = (TH2F*) inputHist->Clone( histName.c_str() );
  return newHist;
}


int main(int argc, char *argv[])
{
  std::time_t initialTime = std::time(0);
  gErrorIgnoreLevel = 2000;
  std::string inFileName = "";
  float upperEdge = 999999;

  /////////// Retrieve arguments //////////////////////////
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
         << "  --upperEdge       Upper edge for final bin" << std::endl
         << "  --sysType         String tag for which sys to run" << std::endl
         << "  --threshold       Threshold value to determine rebinning (default 2 sigma)" << std::endl
         << "  --fit             Perform fits rather than a mean" << std::endl
         << std::endl;
    exit(1);
  }

  std::string sysType = "";
  double threshold = 2.; //sigma
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
    } else if (options.at(iArg).compare("--threshold") == 0) {
       char tmpChar = options.at(iArg+1)[0];
       if (iArg+1 == argc || tmpChar == '-' ) {
         std::cout << " --threshold should be followed by a float" << std::endl;
         return 1;
       } else {
         threshold = std::stof(options.at(iArg+1));
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
    cout << "Only runs on scaled files " << endl;
    exit(1);
  }

  /// Get output name ///
  std::string outFileName = inFileName;
  outFileName.replace(pos, 6, "significant");

  if (sysType.size() == 0){
    cout << "Error, runBootstrapRebin requires a systematic name!  Exiting..." << endl;
    exit(1);
  }
  outFileName += ("."+sysType);
  cout << "Creating Output File " << outFileName << endl;

  std::string fitPlotsOutDir = outFileName;
  fitPlotsOutDir.erase(fitPlotsOutDir.find_last_of("/"));
  fitPlotsOutDir += "/RMSFits/";
  mkdir(fitPlotsOutDir.c_str(), 0777);

  std::string fitPlotsOutName = outFileName;
  fitPlotsOutName.erase(0, fitPlotsOutName.find_last_of("/"));


  // Get relevant systematics //
  TFile *inFile = TFile::Open(inFileName.c_str(), "READ");
  TIter next(inFile->GetListOfKeys());
  TKey *key;

  std::vector<TH2F*> h_2D_sys, h_2D_nominal;
  TH2F *h_full_recoilPt_PtBal = NULL, *h_full_recoilPt_PtBal_nominal = NULL;
  while ((key = (TKey*)next() )){
    std::string sysName = key->GetName();
    if( sysName.find(sysType) == std::string::npos)
      continue;

    //sysName formats are like: Iteration1_Zjet_Stat1_neg_97
    //nominal format is like: Iteration1_Zjet_Stat1_neg
    std::string iteration = sysName.substr(0, sysName.find_first_of('_'));
    std::string toyNum = sysName.substr(sysName.find_last_of('_')+1, sysName.size());
    //if last field is an integer, then it's a toy.  Otherwise it's the full (non-bootstrap) result
    if( isInteger(toyNum) ){
      std::string nominalName = iteration+"_Nominal_"+toyNum;

      TH2F* h_recoilPt_PtBal = (TH2F*) inFile->Get((sysName+"/recoilPt_PtBal").c_str());
      TH2F* rebin_recoilPt_PtBal = initialRebin( h_recoilPt_PtBal );
      h_2D_sys.push_back( rebin_recoilPt_PtBal );
      TH2F* h_recoilPt_PtBal_nominal = (TH2F*) inFile->Get((nominalName+"/recoilPt_PtBal").c_str());
      TH2F* rebin_recoilPt_PtBal_nominal = initialRebin( h_recoilPt_PtBal_nominal );
      h_2D_nominal.push_back( rebin_recoilPt_PtBal_nominal );

    }else{
      std::string nominalName = iteration+"_Nominal";
      TH2F* h_tmp_recoilPt_PtBal = (TH2F*) inFile->Get((sysName+"/recoilPt_PtBal").c_str());
      h_full_recoilPt_PtBal = initialRebin( h_tmp_recoilPt_PtBal );
      TH2F* h_tmp_recoilPt_PtBal_nominal = (TH2F*) inFile->Get((nominalName+"/recoilPt_PtBal").c_str());
      h_full_recoilPt_PtBal_nominal = initialRebin( h_tmp_recoilPt_PtBal_nominal );
    }

  }
  cout << "numToys in sys is " << h_2D_sys.size() << " and in nominal is " << h_2D_nominal.size() << endl;

  if(  h_2D_sys.size() < 1 || !(h_full_recoilPt_PtBal) || !(h_full_recoilPt_PtBal_nominal) ||
      h_full_recoilPt_PtBal->IsZombie() || h_full_recoilPt_PtBal_nominal->IsZombie() ){
    cout << "Error getting toys or nominal histogram.  Exiting..." << endl;
    exit(1);
  }

  // Get Fitting Object
  double NsigmaForFit = 1.6;
  JES_BalanceFitter* m_BalFit = new JES_BalanceFitter(NsigmaForFit);


  //Ignore any bins above upperEdge
  int largestBin = h_2D_sys.at(0)->GetNbinsX();
  while( h_2D_sys.at(0)->GetXaxis()->GetBinLowEdge(largestBin) >= upperEdge){
    largestBin--;
  }

  vector<int> reverseBinEdges; //we start from upper end
  reverseBinEdges.push_back( largestBin );
  vector<double> values_significant;


  // Loop over all bins //
  for( int iBin=reverseBinEdges.at(reverseBinEdges.size()-1); iBin > 0; --iBin){

    cout << "Combining bins " << iBin << " to " << reverseBinEdges.at(reverseBinEdges.size()-1) << endl;

    //Get fits of this iBin projection for full (non-bootstrap)
    TH1D* h_full_proj_sys = h_full_recoilPt_PtBal->ProjectionY("h_full_proj_sys", iBin, reverseBinEdges.at(reverseBinEdges.size()-1), "ed" );
    TH1D* h_full_proj_nominal = h_full_recoilPt_PtBal_nominal->ProjectionY("h_full_proj_nominal", iBin, reverseBinEdges.at(reverseBinEdges.size()-1), "ed" );
    double full_sysVal = -1., full_nominalVal = -1.;

    if(f_fit){
      m_BalFit->Fit(h_full_proj_nominal, 0); // Rebin histogram and fit
      full_nominalVal = m_BalFit->GetMean();
      m_BalFit->Fit(h_full_proj_sys, 0); // Rebin histogram and fit
      full_sysVal = m_BalFit->GetMean();
    }else{
      full_nominalVal = h_full_proj_nominal->GetMean();
      full_sysVal = h_full_proj_sys->GetMean();
    }

    vector<double> meanValues;
    // Loop over all toys //
    for(unsigned int iH = 0; iH < h_2D_sys.size(); ++iH){
      TH1D* h_proj_sys = h_2D_sys.at(iH)->ProjectionY("h_proj_sys", iBin, reverseBinEdges.at(reverseBinEdges.size()-1), "ed" );
      TH1D* h_proj_nominal = h_2D_nominal.at(iH)->ProjectionY("h_proj_nominal", iBin, reverseBinEdges.at(reverseBinEdges.size()-1), "ed" );
      double sysVal = -1., nominalVal = -1.;

      if(f_fit){
        m_BalFit->Fit(h_proj_nominal, 0); // Rebin histogram and fit
        nominalVal = m_BalFit->GetMean();
        m_BalFit->Fit(h_proj_sys, 0); // Rebin histogram and fit
        sysVal = m_BalFit->GetMean();
      }else{
        nominalVal = h_proj_nominal->GetMean();
        sysVal = h_proj_sys->GetMean();
      }
      //!! Need to check here if fit failed, and otherwise give the projection?
      meanValues.push_back(   nominalVal == 0 ? 0 : ((sysVal/nominalVal)-1.)  );
      
      //cout << std::setprecision(20) << (sysVal-nominalVal)/nominalVal << " sys: " << sysVal << " nominalVal: " << nominalVal << endl;

      // Draw this fit for the first toy //
      if( f_fit && iH == 0){
        string cName = fitPlotsOutDir+fitPlotsOutName+"_"+to_string(iBin)+"_"+to_string( reverseBinEdges.at(reverseBinEdges.size()-1) )+".png";
        SaveCanvas(iBin, reverseBinEdges.at(reverseBinEdges.size()-1), cName, m_BalFit);
      }
    }

    // Get mean value from full (non-bootstrap) results //
    double mean = (full_nominalVal == 0 ? 0 : ((full_sysVal/full_nominalVal)-1.)  );

    // Get RMS value from toys //
    double RMS =  TMath::RMS(meanValues.size(), &meanValues[0]);

    double mu = mean / RMS / RMS;
    double sig = 1.0/ RMS / RMS;

    // If RMS is below threshold, then save this bin as an edge. //
    // If above RMS, then this bin will be added with the next bin //
    //if( RMS < thresholdRMS){
    cout << "mean: " << mean << " and RMS: " << RMS << endl;
    cout << "sig: " << sig << " and mean/RMS " << fabs(mu)/sqrt(sig) << " and reverse " << sqrt(sig)/fabs(mu) << endl;
//    if (RMS==0 || ( (fabs(mu)/sqrt(sig) > threshold)) ){ // 3 sigma sig + <30% error
    //if (RMS==0 || ( (fabs(mu)/sqrt(sig) > threshold) && ( sqrt(sig)/fabs(mu) < 1.0/sqrt(10.0) )) ){ // 3 sigma sig + <30% error
    if (RMS==0 ||  (fabs(mu)/sqrt(sig) > threshold)  ){ // 3 sigma sig + <30% error
      reverseBinEdges.push_back(iBin-1);
      if (RMS == 0)
        values_significant.push_back( 0 );
      else
        values_significant.push_back( fabs(mu)/sqrt(sig) );
    }


  }//for all bins

  // Create histograms of the significance values with the final binning //
  int numBins = reverseBinEdges.size()-1;
  Double_t newXbins[numBins];
  for(unsigned int i=0; i < reverseBinEdges.size(); ++i){
    newXbins[numBins-i] = h_2D_sys.at(0)->GetXaxis()->GetBinUpEdge(reverseBinEdges.at(i));
    if (newXbins[numBins-i] > upperEdge)
      newXbins[numBins-i] = upperEdge;
    cout << "!! " << newXbins[numBins-i] << endl;
  }
  TH1D* h_significant = new TH1D( ("significant_"+sysType).c_str(), ("significant_"+sysType).c_str(), numBins, newXbins);
  for(int iBin=1; iBin < h_significant->GetNbinsX()+1; ++iBin){
    h_significant->SetBinContent(iBin, values_significant.at(numBins-iBin) );
  }
  TFile *outFile = TFile::Open(outFileName.c_str(), "RECREATE");
  h_significant->Write("", TObject::kOverwrite);
  outFile->Close();
  inFile->Close();

  std::cout << "Finished Fitting after " << (std::time(0) - initialTime) << " seconds" << std::endl;

  return 0;
}


