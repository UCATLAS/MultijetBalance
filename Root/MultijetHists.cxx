#include <MultijetBalanceAlgo/MultijetHists.h>
#include <sstream>

using namespace std;

MultijetHists :: MultijetHists (std::string name, std::string detailStr) :
  JetHists(name, detailStr)
{

  if( detailStr.find( "extraMJB" ) != std::string::npos)
    f_extraMJBHists = true;
  else
    f_extraMJBHists = false;

  m_debug = false;
}

StatusCode MultijetHists::initialize(std::string binning) {

  JetHists::initialize();
  //details for Multijet Balance
  if ( m_debug ) Info("MultijetHists::initialize()", "adding multijet balance plots");
  stringstream ss;


  //which sampling layers to save, there are 24 total
  m_samplingLayers.push_back(0.);  m_samplingLayers.push_back(1.);  m_samplingLayers.push_back(2.);
  m_samplingLayers.push_back(3.);  m_samplingLayers.push_back(10.);  m_samplingLayers.push_back(11.);
  m_samplingLayers.push_back(12.);  m_samplingLayers.push_back(13.);

  //The leading jets for which to save kinematic information
  m_numSavedJets = 6;

  //////Setup Binnings to use ///
  // if binning is empty, then do all binnings
  Double_t binArray[3][100];
  std::vector<std::string> binningNames;
  std::vector<int> binningSizes;

  //From 8 TeV
  if( binning.size() == 0 || binning.find("8TeV") != std::string::npos){
    Double_t ptBins_8TeV[] = {200.,300.,400.,500.,600.,700.,800.,900.,1100.,1300.,1600.,1900.,2400.,3000.};
    binningNames.push_back("_8TeV");
    int thisBinning = binningNames.size()-1;
    binningSizes.push_back( sizeof(ptBins_8TeV)/sizeof(ptBins_8TeV[0])-1 );
    for( int iBin=0; iBin < binningSizes.at(thisBinning)+1; ++iBin){
      binArray[thisBinning][iBin] = ptBins_8TeV[iBin];
    }
  }

  // Caterina's Coarse
  if( binning.size() == 0 || binning.find("Coarse") != std::string::npos){
    Double_t ptBins_Coarse[] = {20., 55., 100., 152., 240., 408., 598., 1012., 2500.};
    //Double_t ptBins_Coarse[] = {100., 152., 240., 408., 598., 1012., 2500.};
    binningNames.push_back("_Coarse");
    int thisBinning = binningNames.size()-1;
    binningSizes.push_back( sizeof(ptBins_Coarse)/sizeof(ptBins_Coarse[0])-1 );
    for( int iBin=0; iBin < binningSizes.at(thisBinning)+1; ++iBin){
      binArray[thisBinning][iBin] = ptBins_Coarse[iBin];
    }
  }

  // Inclusive Fine
  if( binning.size() == 0 || binning.find("Fine") != std::string::npos){
    //Double_t ptBins_Fine[] = {15. ,20. ,25. ,35. ,45. ,55. ,70. ,85. ,100. ,116. ,134. ,152. ,172. ,194. ,216. ,240. ,264. ,290. ,318. ,346.,376.,408.,442.,478.,516.,556.,598.,642.,688.,736.,786.,838.,894.,952.,1012.,1076.,1162.,1310.,1530.,1992.,2500., 3000., 3500., 4500.};
    //Double_t ptBins_Fine[] = {125, 150, 175, 200, 225, 250, 275, 300, 325, 350, 375, 400, 425, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000, 1050, 1100, 1150, 1200, 1300, 1500, 2000, 5000.};
    Double_t ptBins_Fine[] = {300, 360, 420, 480, 540, 600, 660, 720, 780, 840, 900, 960, 1020, 1080, 1140, 1200, 1260, 1320, 1380, 1480, 1700, 2000, 2700};

    binningNames.push_back("_Fine");
    int thisBinning = binningNames.size()-1;
    binningSizes.push_back( sizeof(ptBins_Fine)/sizeof(ptBins_Fine[0])-1 );
    for( int iBin=0; iBin < binningSizes.at(thisBinning)+1; ++iBin){
      binArray[thisBinning][iBin] = ptBins_Fine[iBin];
    }
  }

  m_numPtBinnings = binningNames.size();

  vector<TH1F*> tmpVec;
  vector<TH2F*> tmpVec2;
  vector<TH3F*> tmpVec3;

  for(int iJet=0; iJet < m_numSavedJets; ++iJet){
    ss << iJet;

//    m_MJBNjetsPt.push_back(tmpVec);
//    for(int iB = 0; iB < m_numPtBinnings; ++iB){
//      m_MJBNjetsPt.at(iJet).push_back( book(m_name, ("jetPt_jet"+ss.str()+binningNames.at(iB)),       "jet p_{T} [GeV]", binningSizes.at(iB), binArray[iB]) );
//    }

    m_MJBNjetsPt.push_back( book(m_name, ("jetPt_jet"+ss.str()),       "jet p_{T} [GeV]", 400, 0, 4000. ) );
    m_MJBNjetsEta.push_back(      book(m_name, ("jetEta_jet"+ss.str()),      ("jet_{"+ss.str()+"} #eta"), 80, -4., 4.) );
    m_MJBNjetsPhi.push_back(      book(m_name, ("jetPhi_jet"+ss.str()),      ("jet_{"+ss.str()+"} #phi"),60, -TMath::Pi(), TMath::Pi() ) );
    m_MJBNjetsM.push_back(        book(m_name, ("jetMass_jet"+ss.str()),     ("jet_{"+ss.str()+"} Mass [GeV]"), 80, 0, 400.) );
    m_MJBNjetsE.push_back(        book(m_name, ("jetEnergy_jet"+ss.str()),   ("jet_{"+ss.str()+"} Energy [GeV]"), 100, 0, 3000.) );
    m_MJBNjetsRapidity.push_back( book(m_name, ("jetRapidity_jet"+ss.str()), "jet Rapidity",80, -4., 4.) );
    m_MJBNjetsBeta.push_back(     book(m_name, ("jetBeta_jet"+ss.str()),     ("jet_{"+ss.str()+"} #beta"), 90, 1.6, 3.15) );
    ss.str("");
  }//for iJet
//  int iLayer = 0;
//  for(int iLVec=0; iLVec < m_samplingLayers.size(); ++iLVec){
//    iLayer = m_samplingLayers.at(iLVec);
//    ss << iLayer;
//
//    m_recoilPt_samplingLayer.push_back(tmpVec2);
//    m_recoilPt_samplingLayerPercent.push_back(tmpVec2);
////    m_leadJetPt_samplingLayer.push_back(tmpVec2);
//    m_leadJetPt_samplingLayerPercent.push_back(tmpVec2);
//    m_leadJetPt_trueSamplingLayerPercent.push_back(tmpVec2);
//    for(int iB = 0; iB < m_numPtBinnings; ++iB){
//
//
//      m_recoilPt_samplingLayer.at(iLayer).push_back(   book(m_name, ("recoilPt_SamplingLayer"+ss.str()+binningNames.at(iB)),
//            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
//            ("Energy in Layer "+ss.str()), binningSizes.at(iB), binArray[iB] ) );
//      m_recoilPt_samplingLayerPercent.at(iLayer).push_back(   book(m_name, ("recoilPt_SamplingLayerPercent"+ss.str()+binningNames.at(iB)),
//            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
//            ("\% of Recoil System Energy in Layer "+ss.str()), 110, 0., 1.1 ) );
////      m_leadJetPt_samplingLayer.at(iLayer).push_back(   book(m_name, ("leadJetPt_SamplingLayer"+ss.str()+binningNames.at(iB)),
////            "Leading Jet p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
////            ("Energy in Layer "+ss.str()), binningSizes.at(iB), binArray[iB] ) );
//      m_leadJetPt_samplingLayerPercent.at(iLayer).push_back(   book(m_name, ("leadJetPt_SamplingLayerPercent"+ss.str()+binningNames.at(iB)),
//            "Leading Jet p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
//            ("\% of Leading Jet Energy in Layer "+ss.str()), 110, 0., 1.1 ) );
//      m_leadJetPt_trueSamplingLayerPercent.at(iLayer).push_back(   book(m_name, ("leadJetPt_TrueSamplingLayerPercent"+ss.str()+binningNames.at(iB)),
//            "Leading Jet p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
//            ("\% of Leading Jet Sampling Energy in Layer "+ss.str()), 110, 0., 1.1 ) );
//    }//for iB
//
//    ss.str("");
//  }

  m_avgBeta = book(m_name, "avgBeta", "Average Beta Angle", 90, 1.6, 3.15);
  m_alpha = book(m_name, "alpha", "Alpha Angle", 80, 2.74, 3.15);
  m_njet = book(m_name, "njet", "Number of Jets", 12, 0., 12.);
  m_ptAsym = book(m_name, "ptAsym", "p_{T} Asymmetry", 90, 0., 0.9);
  m_ptBal = book(m_name, "ptBal", " p_{T} Balance", 80, 0., 4.);
  m_ptAsym_njet = book(m_name, "ptAsym_njet",
          "p_{T} Asymmetry", 90, 0., 0.9,
          "Number of Jets", 12, 0., 12.);

  m_recoilEta = book(m_name, "recoilEta", "Recoil System #eta", 80, -4., 4.);
  m_recoilPhi = book(m_name, "recoilPhi", "Recoil System #phi", 60, -TMath::Pi(), TMath::Pi());
  m_recoilM = book(m_name, "recoilM", "Recoil System Mass (GeV)", 100, 0, 3000.);
  m_recoilE = book(m_name, "recoilE", "Recoil System Energy (GeV)", 100, 0., 3000.);
  m_subOverRecoilPt = book( m_name, "subOverRecoilPt", "Subleading Jet p_{T} / Recoil System p_{T}", 100, 0., 1.);
  m_recoilPt_center = book(m_name, "recoilPt_center", "Recoil System p_{T}", 2000, 0, 4000.);

  Double_t ptBalBins[501];
  int numPtBalBins = 500;
  for(int i=0; i < numPtBalBins+1; ++i){
    ptBalBins[i] = i/100.;
  }

  for(int iB = 0; iB < m_numPtBinnings; ++iB){
    m_recoilPt.push_back( book(m_name, ("recoilPt"+binningNames.at(iB)), "Recoil System p_{T} (GeV)", binningSizes.at(iB), binArray[iB]) );

    m_recoilPt_jet1Pt.push_back( book(m_name, ("recoilPt_jet1Pt"+binningNames.at(iB)),
            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
            "sub Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB]) );
    m_recoilPt_avgBeta.push_back( book(m_name, ("recoilPt_avgBeta"+binningNames.at(iB)),
            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
            "Average #beta", 90, 1.6, 3.15) );
    m_recoilPt_alpha.push_back( book(m_name, ("recoilPt_alpha"+binningNames.at(iB)),
            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
            "Alpha", 80, 2.74, 3.15) );
    m_recoilPt_njet.push_back( book(m_name, ("recoilPt_njet"+binningNames.at(iB)),
            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
            "Number of Jets", 12, 0., 12.) );
//    m_recoilPt_averageIPC.push_back( book(m_name, ("recoilPt_averageIPC"+binningNames.at(iB)),
//            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
//            "Average Interactions Per Crossing", 60., 0., 60.) );
//    m_recoilPt_actualIPC.push_back( book(m_name, ("recoilPt_actualIPC"+binningNames.at(iB)),
//            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
//            "Actual Interactions Per Crossing", 60., 0., 60.) );

    m_leadJetPt_jet1Pt.push_back( book(m_name, ("leadJetPt_jet1Pt"+binningNames.at(iB)),
//            "Leading Jet p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
            "Leading Jet p_{T} [GeV]", 400, 0, 4000.,
            "Subleading Jet p_{T} [GeV]", binningSizes.at(iB), binArray[iB]) );
    m_leadJetPt_avgBeta.push_back( book(m_name, ("leadJetPt_avgBeta"+binningNames.at(iB)),
            "Leading Jet p_{T} [GeV]", 400, 0, 4000.,
            "Average #beta", 90, 1.6, 3.15) );
    m_leadJetPt_alpha.push_back( book(m_name, ("leadJetPt_alpha"+binningNames.at(iB)),
            "Leading Jet p_{T} [GeV]", 400, 0, 4000.,
            "Alpha", 80, 2.74, 3.15) );
    m_leadJetPt_njet.push_back( book(m_name, ("leadJetPt_njet"+binningNames.at(iB)),
            "Leading Jet p_{T} [GeV]", 400, 0, 4000.,
            "Number of Jets", 12, 0., 12.) );

    /////////////////jetPt vs correction ///////////////////////
    m_leadJetPt_ptBal.push_back( book(m_name, ("leadJetPt_PtBal"+binningNames.at(iB)),
            "Leading Jet p_{T} [GeV]", 400, 0, 4000.,
            "p_{T} Balance",  numPtBalBins, ptBalBins) );
    m_recoilPt_ptBal.push_back( book(m_name, ("recoilPt_PtBal"+binningNames.at(iB)),
            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
            "p_{T} Balance", numPtBalBins, ptBalBins) );

    m_recoilPt_ptBal_eta1.push_back( book(m_name, ("recoilPt_PtBal_eta1"+binningNames.at(iB)),
            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
            "p_{T} Balance", numPtBalBins, ptBalBins) );
    m_recoilPt_ptBal_eta2.push_back( book(m_name, ("recoilPt_PtBal_eta2"+binningNames.at(iB)),
            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
            "Inverse  p_{T} Balance", numPtBalBins, ptBalBins) );
    m_recoilPt_ptBal_eta3.push_back( book(m_name, ("recoilPt_PtBal_eta3"+binningNames.at(iB)),
            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
            "p_{T} Balance", numPtBalBins, ptBalBins) );


    //////////////// jetPt vs sampling Pt /////////////////////////////////
    m_leadJetPt_EMFrac.push_back( book(m_name, ("leadJetPt_EMFrac"+binningNames.at(iB)),
            "Leading Jet p_{T} [GeV]", 400, 0, 4000.,
            "EMFrac", 100, 0., 1.) );
    m_recoilPt_EMFrac.push_back( book(m_name, ("recoilPt_EMFrac"+binningNames.at(iB)),
            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
            "EMFrac", 100, 0., 1.) );
    m_leadJetPt_HECFrac.push_back( book(m_name, ("leadJetPt_HECFrac"+binningNames.at(iB)),
            "Leading Jet p_{T} [GeV]", 400, 0, 4000.,
            "HECFrac", 100, 0., 1.) );
    m_recoilPt_HECFrac.push_back( book(m_name, ("recoilPt_HECFrac"+binningNames.at(iB)),
            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
            "HECFrac", 100, 0., 1.) );
    m_leadJetPt_TileFrac.push_back( book( m_name, ("leadJetPt_TileFrac"+binningNames.at(iB)),
            "Leading Jet p_{T} [GeV]", 400, 0, 4000.,
            "TileFrac", 100, 0., 1.) );
    m_recoilPt_TileFrac.push_back( book( m_name, ("recoilPt_TileFrac"+binningNames.at(iB)),
            "Recoil System p_{T} [GeV]", binningSizes.at(iB), binArray[iB],
            "TileFrac", 100, 0., 1.) );
  } //for iB

  return StatusCode::SUCCESS;
}

StatusCode MultijetHists::execute( std::vector< xAOD::Jet* >* jets, const xAOD::EventInfo* eventInfo, int pvLoc ) {


  //////// Grab Accessors and commonly accessed values ////////////////
  static SG::AuxElement::ConstAccessor<float> avgBeta ("avgBeta");
  static SG::AuxElement::ConstAccessor<float> alpha ("alpha");
  static SG::AuxElement::ConstAccessor<int> njet ("njet");
  static SG::AuxElement::ConstAccessor<float> ptAsym ("ptAsym");
  static SG::AuxElement::ConstAccessor<float> ptBal ("ptBal");
  static SG::AuxElement::ConstAccessor<float> ptBal2 ("ptBal2");
  static SG::AuxElement::ConstAccessor<float> recoilPt ("recoilPt");
  static SG::AuxElement::ConstAccessor<float> recoilEta ("recoilEta");
  static SG::AuxElement::ConstAccessor<float> recoilPhi ("recoilPhi");
  static SG::AuxElement::ConstAccessor<float> recoilM ("recoilM");
  static SG::AuxElement::ConstAccessor<float> recoilE ("recoilE");
  static SG::AuxElement::ConstAccessor<float> weight ("weight");
  static SG::AuxElement::ConstAccessor<float> detEta ("detEta");
  static SG::AuxElement::ConstAccessor<float> beta ("beta");

  float eventWeight = weight( *eventInfo );
  float leadJetPt = jets->at(0)->pt()/1e3;
  float recoilJetPt = recoilPt( *eventInfo )/1e3;
  float thisPtBal = ptBal( *eventInfo );

  for(unsigned int iJet=0; iJet < jets->size(); ++iJet){
    JetHists::execute( jets->at(iJet), eventWeight, pvLoc);
  }

//  float totalEnergy = 0.;
//  for(int iLayer=0; iLayer < 24; ++iLayer){
//    totalEnergy += (jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(iLayer)/1e3);
//  }

  /////////////////// Fill individual Jet Hists //////////////////////////
  int numJets = std::min( m_numSavedJets, (int)jets->size() );
  for(int iJet=0; iJet < numJets; ++iJet){
//    for(int iB = 0; iB < m_numPtBinnings; ++iB){
//      m_MJBNjetsPt.at(iJet).at(iB)->        Fill( jets->at(iJet)->pt()/1e3,   eventWeight);
//    }
    m_MJBNjetsPt.at(iJet)->        Fill( jets->at(iJet)->pt()/1e3,   eventWeight);
    m_MJBNjetsEta.at(iJet)->       Fill( jets->at(iJet)->eta(),      eventWeight);
    m_MJBNjetsPhi.at(iJet)->       Fill( jets->at(iJet)->phi(),      eventWeight);
    m_MJBNjetsM.at(iJet)->         Fill( jets->at(iJet)->m()/1e3,    eventWeight);
    m_MJBNjetsE.at(iJet)->         Fill( jets->at(iJet)->e()/1e3,    eventWeight);
    m_MJBNjetsBeta.at(iJet)->      Fill( beta(*jets->at(iJet)), eventWeight);
  }

  m_recoilEta->Fill( recoilEta( *eventInfo ), eventWeight);
  m_recoilPhi->Fill( recoilPhi( *eventInfo ), eventWeight);
  m_recoilM->Fill( recoilM( *eventInfo )/1e3, eventWeight);
  m_recoilE->Fill( recoilE( *eventInfo )/1e3, eventWeight);
  m_recoilPt_center->Fill( recoilJetPt, eventWeight);
  m_subOverRecoilPt->Fill( (jets->at(1)->pt()/1e3)/recoilJetPt, eventWeight);

  m_avgBeta->Fill(avgBeta( *eventInfo ), eventWeight);
  m_alpha->Fill(alpha( *eventInfo ), eventWeight);
  m_njet->Fill(njet( *eventInfo ), eventWeight);
  m_ptAsym->Fill(ptAsym( *eventInfo ), eventWeight);
  m_ptBal->Fill(thisPtBal, eventWeight);

  m_ptAsym_njet->Fill(ptAsym( *eventInfo ), njet( *eventInfo ), eventWeight);


  ///// For Pt Binned Histograms //////
  for(int iB = 0; iB < m_numPtBinnings; ++iB){

    m_recoilPt.at(iB)->Fill( recoilJetPt, eventWeight);

    m_recoilPt_jet1Pt.at(iB)      ->Fill(recoilJetPt, jets->at(1)->pt()/1e3, eventWeight);
    m_recoilPt_avgBeta.at(iB)     ->Fill(recoilJetPt, avgBeta( *eventInfo ), eventWeight);
    m_recoilPt_alpha.at(iB)       ->Fill(recoilJetPt, alpha( *eventInfo ), eventWeight);
    m_recoilPt_njet.at(iB)        ->Fill(recoilJetPt, njet( *eventInfo ), eventWeight);
//    m_recoilPt_averageIPC.at(iB)  ->Fill(recoilJetPt, eventInfo->averageInteractionsPerCrossing(), eventWeight);
//    m_recoilPt_actualIPC.at(iB)   ->Fill(recoilJetPt, eventInfo->actualInteractionsPerCrossing(), eventWeight);


    m_leadJetPt_jet1Pt.at(iB)      ->Fill(leadJetPt, jets->at(1)->pt()/1e3, eventWeight);
    m_leadJetPt_avgBeta.at(iB)     ->Fill(leadJetPt, avgBeta( *eventInfo ), eventWeight);
    m_leadJetPt_alpha.at(iB)       ->Fill(leadJetPt, alpha( *eventInfo ), eventWeight);
    m_leadJetPt_njet.at(iB)        ->Fill(leadJetPt, njet( *eventInfo ), eventWeight);
//    m_leadJetPt_averageIPC.at(iB)  ->Fill(leadJetPt, averageIPC( *eventInfo ), eventWeight);
//    m_leadJetPt_actualIPC.at(iB)   ->Fill(leadJetPt, actualIPC( *eventInfo ), eventWeight);



    m_recoilPt_ptBal.at(iB) ->Fill(recoilJetPt, thisPtBal, eventWeight);
    m_leadJetPt_ptBal.at(iB) ->Fill(leadJetPt, thisPtBal, eventWeight);


    if( fabs( detEta(*jets->at(0))) <= 0.4 ) {
      m_recoilPt_ptBal_eta1.at(iB)->Fill(recoilJetPt, thisPtBal, eventWeight);
    }else if( (fabs( detEta(*jets->at(0))) > 0.4) && (fabs(detEta(*jets->at(0))) <= 0.8) ) {
      m_recoilPt_ptBal_eta2.at(iB)->Fill(recoilJetPt, thisPtBal, eventWeight);
    }else if( fabs(detEta(*jets->at(0))) <= 1.2 ) {
      m_recoilPt_ptBal_eta3.at(iB)->Fill(recoilJetPt, thisPtBal, eventWeight);
    }

    m_recoilPt_EMFrac.at(iB)   ->Fill( recoilJetPt, jets->at(0)->auxdata<float>("EMFrac"), eventWeight );
    m_recoilPt_HECFrac.at(iB)  ->Fill( recoilJetPt, jets->at(0)->auxdata<float>("HECFrac"), eventWeight );
    m_recoilPt_TileFrac.at(iB) ->Fill( recoilJetPt, jets->at(0)->auxdecor<float>("TileFrac"), eventWeight);
//    m_recoilPt_TileFrac.at(iB) ->Fill( recoilJetPt, ((jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(12)+jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(13)+jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(14))/1e3/totalEnergy), eventWeight);

    m_leadJetPt_EMFrac.at(iB)     ->Fill( leadJetPt, jets->at(0)->auxdata<float>("EMFrac"), eventWeight );
    m_leadJetPt_HECFrac.at(iB)    ->Fill( leadJetPt, jets->at(0)->auxdata<float>("HECFrac"), eventWeight );
    m_leadJetPt_TileFrac.at(iB) ->Fill( leadJetPt, jets->at(0)->auxdecor<float>("TileFrac"), eventWeight);
//    m_leadJetPt_TileFrac.at(iB)   ->Fill( leadJetPt, ((jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(12)+jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(13)+jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(14))/1e3/totalEnergy), eventWeight);

//    int iLayer;
//    for(int iLVec=0; iLVec < m_samplingLayers.size(); ++iLVec){
//      iLayer = m_samplingLayers.at(iLVec);
//
//      m_recoilPt_samplingLayer.at(iLayer).at(iB)        ->Fill( recoilJetPt, jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(iLayer)/1e3, eventWeight);
//      m_recoilPt_samplingLayerPercent.at(iLayer).at(iB) ->Fill( recoilJetPt, jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(iLayer)/recoilE(*eventInfo), eventWeight);
////      m_leadJetPt_samplingLayer.at(iLayer).at(iB)          ->Fill( leadJetPt, jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(iLayer)/1e3, eventWeight);
//      m_leadJetPt_samplingLayerPercent.at(iLayer).at(iB)   ->Fill( leadJetPt, jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(iLayer)/jets->at(0)->e(), eventWeight);
//      m_leadJetPt_trueSamplingLayerPercent.at(iLayer).at(iB)   ->Fill( leadJetPt, jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(iLayer)/1e3/totalEnergy, eventWeight);
//
//    }//iLayer

  }//iB

  return StatusCode::SUCCESS;
}

