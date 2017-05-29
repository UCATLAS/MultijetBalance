#include <InsituBalance/MultijetHists.h>
#include <sstream>

using namespace std;

MultijetHists :: MultijetHists (std::string name, std::string detailStr) :
  JetHists(name, detailStr)
{

  if( detailStr.find( "extraMJB" ) != std::string::npos)
    f_extraMJBHists = true;
  else
    f_extraMJBHists = false;

  if( detailStr.find( "bootstrapIteration" ) != std::string::npos)
    f_minimalMJBHists = true;
  else
    f_minimalMJBHists = false;

  m_debug = false;
}

StatusCode MultijetHists::initialize(std::string binning) {

  if( !f_minimalMJBHists ){
    JetHists::initialize();
  }

  //details for Multijet Balance
  if ( m_debug ) Info("initialize()", "Adding MJB plots");
  stringstream ss;

  //which sampling layers to save, there are 24 total
  m_samplingLayers.push_back(0.);  m_samplingLayers.push_back(1.);  m_samplingLayers.push_back(2.);
  m_samplingLayers.push_back(3.);  m_samplingLayers.push_back(10.);  m_samplingLayers.push_back(11.);
  m_samplingLayers.push_back(12.);  m_samplingLayers.push_back(13.);

  //The leading jets for which to save kinematic information
  m_numSavedJets = 6;

  //////Setup Binnings to use ///
  Double_t binArray[100];
  std::stringstream ssb(binning);
  std::string thisBinStr;
  std::string::size_type sz;
  vector<double> vecBins;
  while (std::getline(ssb, thisBinStr, ',')) {
    vecBins.push_back( std::stof(thisBinStr, &sz) );
  }

  m_numBins = vecBins.size()-1;

  for( int iBin=0; iBin < m_numBins+1; ++iBin ){
    binArray[iBin] = vecBins.at(iBin);
//    cout << "Bin " << iBin << " gets " << binArray[iBin] << endl;
  }


  //Temporary vectors for sampling layers
  vector<TH1F*> tmpVec;
  vector<TH2F*> tmpVec2;

  // pt balance binnings
  Double_t ptBalBins[501];
  int numPtBalBins = 500;
  for(int i=0; i < numPtBalBins+1; ++i){
    ptBalBins[i] = i/100.;
  }

  if( f_minimalMJBHists ){
    m_recoilPt_ptBal = book(m_name, ("recoilPt_PtBal"),
            "Recoil System p_{T} [GeV]", m_numBins, binArray,
            "p_{T} Balance", numPtBalBins, ptBalBins);
  return StatusCode::SUCCESS;
  }


  for(int iJet=0; iJet < m_numSavedJets; ++iJet){
    ss << iJet;

    m_MJBNjetsPt.push_back( book(m_name, ("jetPt_jet"+ss.str()),       "jet p_{T} [GeV]", 400, 0, 4000. ) );
    m_MJBNjetsEta.push_back(      book(m_name, ("jetEta_jet"+ss.str()),      ("jet_{"+ss.str()+"} #eta"), 80, -4., 4.) );
    m_MJBNjetsPhi.push_back(      book(m_name, ("jetPhi_jet"+ss.str()),      ("jet_{"+ss.str()+"} #phi"),60, -TMath::Pi(), TMath::Pi() ) );
    m_MJBNjetsM.push_back(        book(m_name, ("jetMass_jet"+ss.str()),     ("jet_{"+ss.str()+"} Mass [GeV]"), 80, 0, 400.) );
    m_MJBNjetsE.push_back(        book(m_name, ("jetEnergy_jet"+ss.str()),   ("jet_{"+ss.str()+"} Energy [GeV]"), 100, 0, 3000.) );
    m_MJBNjetsRapidity.push_back( book(m_name, ("jetRapidity_jet"+ss.str()), "jet Rapidity",80, -4., 4.) );
    m_MJBNjetsBeta.push_back(     book(m_name, ("jetBeta_jet"+ss.str()),     ("jet_{"+ss.str()+"} #beta"), 80, 0, 3.2) );

//    m_MJBNjetsPt.push_back(tmpVec);
//    m_MJBNjetsPt.at(iJet).push_back( book(m_name, ("jetPt_jet"+ss.str()),       "jet p_{T} [GeV]", m_numBins, binArray) );
//
    ss.str("");
  }//for iJet

  m_avgBeta = book(m_name, "avgBeta", "Average Beta Angle", 80, 0, 3.2);
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
  //m_recoilPt_center = book(m_name, "recoilPt_center", "Recoil System p_{T}", 2000, 0, 4000.);
  m_recoilPt_center = book(m_name, "recoilPt_center", "Recoil System p_{T}", 400, 0, 4000.);


  m_recoilPt = book(m_name, ("recoilPt"), "Recoil System p_{T} (GeV)", m_numBins, binArray) ;

  m_recoilPt_jet0Pt = book(m_name, ("recoilPt_leadJetPt"),
          "Recoil System p_{T} [GeV]", m_numBins, binArray,
          "Leading Jet p_{T} [GeV]", 200, 0, 4000. ) ;

  m_recoilPt_jet1Pt = book(m_name, ("recoilPt_jet1Pt"),
          "Recoil System p_{T} [GeV]", m_numBins, binArray,
          "Subleading Jet p_{T} [GeV]", 150, 0, 3000) ;
  m_recoilPt_avgBeta = book(m_name, ("recoilPt_avgBeta"),
          "Recoil System p_{T} [GeV]", m_numBins, binArray,
          "Average #beta", 80, 0, 3.2) ;
  m_recoilPt_alpha = book(m_name, ("recoilPt_alpha"),
          "Recoil System p_{T} [GeV]", m_numBins, binArray,
          "Alpha", 80, 2.74, 3.15) ;
  m_recoilPt_njet = book(m_name, ("recoilPt_njet"),
          "Recoil System p_{T} [GeV]", m_numBins, binArray,
          "Number of Jets", 12, 0., 12.) ;

  m_leadJetPt_jet1Pt = book(m_name, ("leadJetPt_jet1Pt"),
          "Leading Jet p_{T} [GeV]", 200, 0, 4000.,
          "Subleading Jet p_{T} [GeV]", 150, 0, 3000. ) ;
  m_leadJetPt_avgBeta = book(m_name, ("leadJetPt_avgBeta"),
          "Leading Jet p_{T} [GeV]", 200, 0, 4000.,
          "Average #beta", 80, 0, 3.2) ;
  m_leadJetPt_alpha = book(m_name, ("leadJetPt_alpha"),
          "Leading Jet p_{T} [GeV]", 200, 0, 4000.,
          "Alpha", 80, 2.74, 3.15) ;
  m_leadJetPt_njet = book(m_name, ("leadJetPt_njet"),
          "Leading Jet p_{T} [GeV]", 200, 0, 4000.,
          "Number of Jets", 12, 0., 12.) ;

  /////////////////jetPt vs correction ///////////////////////
  m_leadJetPt_ptBal = book(m_name, ("leadJetPt_PtBal"),
          "Leading Jet p_{T} [GeV]", 200, 0, 4000.,
          "p_{T} Balance",  numPtBalBins, ptBalBins) ;
  m_recoilPt_ptBal = book(m_name, ("recoilPt_PtBal"),
          "Recoil System p_{T} [GeV]", m_numBins, binArray,
          "p_{T} Balance", numPtBalBins, ptBalBins) ;

  m_recoilPt_ptBal_eta1 = book(m_name, ("recoilPt_PtBal_eta1"),
          "Recoil System p_{T} [GeV]", m_numBins, binArray,
          "p_{T} Balance", numPtBalBins, ptBalBins) ;
  m_recoilPt_ptBal_eta2 = book(m_name, ("recoilPt_PtBal_eta2"),
          "Recoil System p_{T} [GeV]", m_numBins, binArray,
          "Inverse  p_{T} Balance", numPtBalBins, ptBalBins) ;
  m_recoilPt_ptBal_eta3 = book(m_name, ("recoilPt_PtBal_eta3"),
          "Recoil System p_{T} [GeV]", m_numBins, binArray,
          "p_{T} Balance", numPtBalBins, ptBalBins) ;


  //////////////// jetPt vs sampling Pt /////////////////////////////////
  m_leadJetPt_EMFrac = book(m_name, ("leadJetPt_EMFrac"),
          "Leading Jet p_{T} [GeV]", 100, 0, 4000.,
          "EMFrac", 100, 0., 1.) ;
  m_recoilPt_EMFrac = book(m_name, ("recoilPt_EMFrac"),
          "Recoil System p_{T} [GeV]", m_numBins, binArray,
          "EMFrac", 100, 0., 1.) ;
  m_leadJetPt_HECFrac = book(m_name, ("leadJetPt_HECFrac"),
          "Leading Jet p_{T} [GeV]", 100, 0, 4000.,
          "HECFrac", 100, 0., 1.) ;
  m_recoilPt_HECFrac = book(m_name, ("recoilPt_HECFrac"),
          "Recoil System p_{T} [GeV]", m_numBins, binArray,
          "HECFrac", 100, 0., 1.) ;
  m_leadJetPt_TileFrac = book( m_name, ("leadJetPt_TileFrac"),
          "Leading Jet p_{T} [GeV]", 100, 0, 4000.,
          "TileFrac", 100, 0., 1.) ;
  m_recoilPt_TileFrac = book( m_name, ("recoilPt_TileFrac"),
          "Recoil System p_{T} [GeV]", m_numBins, binArray,
          "TileFrac", 100, 0., 1.) ;

//    m_recoilPt_averageIPC = book(m_name, ("recoilPt_averageIPC"),
//            "Recoil System p_{T} [GeV]", m_numBins, binArray,
//            "Average Interactions Per Crossing", 60., 0., 60.) ;
//    m_recoilPt_actualIPC = book(m_name, ("recoilPt_actualIPC"),
//            "Recoil System p_{T} [GeV]", m_numBins, binArray,
//            "Actual Interactions Per Crossing", 60., 0., 60.) ;


  ///// Layer plots - currently not used ///////
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
//
//      m_recoilPt_samplingLayer.at(iLayer).push_back(   book(m_name, ("recoilPt_SamplingLayer"+ss.str()),
//            "Recoil System p_{T} [GeV]", m_numBins, binArray,
//            ("Energy in Layer "+ss.str()), m_numBins, binArray ) );
//      m_recoilPt_samplingLayerPercent.at(iLayer).push_back(   book(m_name, ("recoilPt_SamplingLayerPercent"+ss.str()),
//            "Recoil System p_{T} [GeV]", m_numBins, binArray,
//            ("\% of Recoil System Energy in Layer "+ss.str()), 110, 0., 1.1 ) );
////      m_leadJetPt_samplingLayer.at(iLayer).push_back(   book(m_name, ("leadJetPt_SamplingLayer"+ss.str()),
////            "Leading Jet p_{T} [GeV]", m_numBins, binArray,
////            ("Energy in Layer "+ss.str()), m_numBins, binArray ) );
//      m_leadJetPt_samplingLayerPercent.at(iLayer).push_back(   book(m_name, ("leadJetPt_SamplingLayerPercent"+ss.str()),
//            "Leading Jet p_{T} [GeV]", m_numBins, binArray,
//            ("\% of Leading Jet Energy in Layer "+ss.str()), 110, 0., 1.1 ) );
//      m_leadJetPt_trueSamplingLayerPercent.at(iLayer).push_back(   book(m_name, ("leadJetPt_TrueSamplingLayerPercent"+ss.str()),
//            "Leading Jet p_{T} [GeV]", m_numBins, binArray,
//            ("\% of Leading Jet Sampling Energy in Layer "+ss.str()), 110, 0., 1.1 ) );
//
//    ss.str("");
//  }


  return StatusCode::SUCCESS;
}

StatusCode MultijetHists::execute( std::vector< xAOD::Jet* >* jets, const xAOD::EventInfo* eventInfo) {


  //////// Grab Accessors and commonly accessed values ////////////////
  static SG::AuxElement::ConstAccessor<float> recoilPt ("recoilPt");
  float recoilJetPt = recoilPt( *eventInfo )/1e3;
  static SG::AuxElement::ConstAccessor<float> ptBal ("ptBal");
  float thisPtBal = ptBal( *eventInfo );
  static SG::AuxElement::ConstAccessor<float> weight ("weight");
  float eventWeight = weight( *eventInfo );
  static SG::AuxElement::ConstAccessor<float> weight_prescale ("weight_prescale");
  float eventWeight_prescale = weight_prescale( *eventInfo );

  if( f_minimalMJBHists ){
    m_recoilPt_ptBal->Fill(recoilJetPt, thisPtBal, eventWeight);
    return StatusCode::SUCCESS;
  }

  static SG::AuxElement::ConstAccessor<float> avgBeta ("avgBeta");
  static SG::AuxElement::ConstAccessor<float> alpha ("alpha");
  static SG::AuxElement::ConstAccessor<int> njet ("njet");
  static SG::AuxElement::ConstAccessor<float> ptAsym ("ptAsym");
  static SG::AuxElement::ConstAccessor<float> recoilEta ("recoilEta");
  static SG::AuxElement::ConstAccessor<float> recoilPhi ("recoilPhi");
  static SG::AuxElement::ConstAccessor<float> recoilM ("recoilM");
  static SG::AuxElement::ConstAccessor<float> recoilE ("recoilE");
  static SG::AuxElement::ConstAccessor<float> detEta ("detEta");
  static SG::AuxElement::ConstAccessor<float> beta ("beta");

  float leadJetPt = jets->at(0)->pt()/1e3;


  //Fill Nominal Jet Info
  for(unsigned int iJet=0; iJet < jets->size(); ++iJet){
    JetHists::execute( jets->at(iJet), eventWeight, eventInfo);
  }

  //For Sampling Layer Plots //
//  float totalEnergy = 0.;
//  for(int iLayer=0; iLayer < 24; ++iLayer){
//    totalEnergy += (jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(iLayer)/1e3);
//  }

  /////////////////// Fill individual Jet Hists //////////////////////////
  int numJets = std::min( m_numSavedJets, (int)jets->size() );
  for(int iJet=0; iJet < numJets; ++iJet){
    m_MJBNjetsPt.at(iJet)->        Fill( jets->at(iJet)->pt()/1e3,   eventWeight);
    m_MJBNjetsEta.at(iJet)->       Fill( jets->at(iJet)->eta(),      eventWeight);
    m_MJBNjetsPhi.at(iJet)->       Fill( jets->at(iJet)->phi(),      eventWeight);
    m_MJBNjetsM.at(iJet)->         Fill( jets->at(iJet)->m()/1e3,    eventWeight);
    m_MJBNjetsE.at(iJet)->         Fill( jets->at(iJet)->e()/1e3,    eventWeight);

    if( beta.isAvailable(*jets->at(iJet)) ) m_MJBNjetsBeta.at(iJet)->Fill( beta(*jets->at(iJet)), eventWeight);
  }

  m_recoilEta->Fill( recoilEta( *eventInfo ), eventWeight);
  m_recoilPhi->Fill( recoilPhi( *eventInfo ), eventWeight);
  m_recoilM->Fill( recoilM( *eventInfo )/1e3, eventWeight);
  m_recoilE->Fill( recoilE( *eventInfo )/1e3, eventWeight);
  m_recoilPt_center->Fill( recoilJetPt, eventWeight);
  if( jets->size() >=2 ){
    m_subOverRecoilPt->Fill( (jets->at(1)->pt()/1e3)/recoilJetPt, eventWeight);
    m_recoilPt_jet1Pt      ->Fill(recoilJetPt, jets->at(1)->pt()/1e3, eventWeight);
    m_leadJetPt_jet1Pt      ->Fill(leadJetPt, jets->at(1)->pt()/1e3, eventWeight);
  }

  if( avgBeta.isAvailable(*eventInfo) ) m_avgBeta->Fill(avgBeta( *eventInfo ), eventWeight);
  if( alpha.isAvailable(*eventInfo) ) m_alpha->Fill(alpha( *eventInfo ), eventWeight);
  if( njet.isAvailable(*eventInfo) ) m_njet->Fill(njet( *eventInfo ), eventWeight);
  if( ptAsym.isAvailable(*eventInfo) ) m_ptAsym->Fill(ptAsym( *eventInfo ), eventWeight);
  m_ptBal->Fill(thisPtBal, eventWeight);

  if( njet.isAvailable(*eventInfo) ) m_ptAsym_njet->Fill(ptAsym( *eventInfo ), njet( *eventInfo ), eventWeight);


  ///// For Pt Binned Histograms //////

  m_recoilPt->Fill( recoilJetPt, eventWeight);

  m_recoilPt_jet0Pt      ->Fill(recoilJetPt, leadJetPt, eventWeight);
  if( avgBeta.isAvailable(*eventInfo) ) m_recoilPt_avgBeta->Fill(recoilJetPt, avgBeta( *eventInfo ), eventWeight);
  if( alpha.isAvailable(*eventInfo) ) m_recoilPt_alpha       ->Fill(recoilJetPt, alpha( *eventInfo ), eventWeight);
  if( njet.isAvailable(*eventInfo) ) m_recoilPt_njet        ->Fill(recoilJetPt, njet( *eventInfo ), eventWeight);


  if( avgBeta.isAvailable(*eventInfo) ) m_leadJetPt_avgBeta     ->Fill(leadJetPt, avgBeta( *eventInfo ), eventWeight);
  if( alpha.isAvailable(*eventInfo) ) m_leadJetPt_alpha       ->Fill(leadJetPt, alpha( *eventInfo ), eventWeight);
  if( njet.isAvailable(*eventInfo) ) m_leadJetPt_njet        ->Fill(leadJetPt, njet( *eventInfo ), eventWeight);



  if( eventWeight_prescale > 0.){
    m_recoilPt_ptBal ->Fill(recoilJetPt, thisPtBal, eventWeight/eventWeight_prescale);
    m_leadJetPt_ptBal ->Fill(leadJetPt, thisPtBal, eventWeight/eventWeight_prescale);
  }else{
    m_recoilPt_ptBal ->Fill(recoilJetPt, thisPtBal, eventWeight);
    m_leadJetPt_ptBal ->Fill(leadJetPt, thisPtBal, eventWeight);
  }

  if( fabs( detEta(*jets->at(0))) <= 0.4 ) {
    m_recoilPt_ptBal_eta1->Fill(recoilJetPt, thisPtBal, eventWeight);
  }else if( (fabs( detEta(*jets->at(0))) > 0.4) && (fabs(detEta(*jets->at(0))) <= 0.8) ) {
    m_recoilPt_ptBal_eta2->Fill(recoilJetPt, thisPtBal, eventWeight);
  }else if( fabs(detEta(*jets->at(0))) <= 1.2 ) {
    m_recoilPt_ptBal_eta3->Fill(recoilJetPt, thisPtBal, eventWeight);
  }

  m_recoilPt_EMFrac   ->Fill( recoilJetPt, jets->at(0)->auxdata<float>("EMFrac"), eventWeight );
  m_recoilPt_HECFrac  ->Fill( recoilJetPt, jets->at(0)->auxdata<float>("HECFrac"), eventWeight );
  m_recoilPt_TileFrac ->Fill( recoilJetPt, jets->at(0)->auxdecor<float>("TileFrac"), eventWeight);

  m_leadJetPt_EMFrac     ->Fill( leadJetPt, jets->at(0)->auxdata<float>("EMFrac"), eventWeight );
  m_leadJetPt_HECFrac    ->Fill( leadJetPt, jets->at(0)->auxdata<float>("HECFrac"), eventWeight );
  m_leadJetPt_TileFrac ->Fill( leadJetPt, jets->at(0)->auxdecor<float>("TileFrac"), eventWeight);

  //Sampling Layer plots //

//    int iLayer;
//    for(int iLVec=0; iLVec < m_samplingLayers.size(); ++iLVec){
//      iLayer = m_samplingLayers.at(iLVec);
//
//      m_recoilPt_samplingLayer.at(iLayer)        ->Fill( recoilJetPt, jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(iLayer)/1e3, eventWeight);
//      m_recoilPt_samplingLayerPercent.at(iLayer) ->Fill( recoilJetPt, jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(iLayer)/recoilE(*eventInfo), eventWeight);
////      m_leadJetPt_samplingLayer.at(iLayer)          ->Fill( leadJetPt, jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(iLayer)/1e3, eventWeight);
//      m_leadJetPt_samplingLayerPercent.at(iLayer)   ->Fill( leadJetPt, jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(iLayer)/jets->at(0)->e(), eventWeight);
//      m_leadJetPt_trueSamplingLayerPercent.at(iLayer)   ->Fill( leadJetPt, jets->at(0)->auxdata< vector<float> >("EnergyPerSampling").at(iLayer)/1e3/totalEnergy, eventWeight);
//
//    }//iLayer


  return StatusCode::SUCCESS;
}

