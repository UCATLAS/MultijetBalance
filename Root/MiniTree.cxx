#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODEventInfo/EventInfo.h"

#include "MultijetBalanceAlgo/MiniTree.h"

MiniTree :: MiniTree(xAOD::TEvent * event, TTree* tree, TFile* file) :
  HelpTreeBase(event, tree, file, 1e3)
{
  if ( m_debug ) Info("MiniTree", "Creating output TTree %s", tree->GetName());
}

MiniTree :: ~MiniTree()
{
}

void MiniTree::AddEventUser(std::string detailStringMJB)
{
  // event variables
  m_tree->Branch("njet", &m_njet, "njet/I");
  m_tree->Branch("trig", &m_trig, "trig/I");
//  m_tree->Branch("lumiBlock", &m_lumiBlock, "lumiBlock/I");

 // m_tree->Branch("actualInteractionsPerCrossing", &m_actualInteractionsPerCrossing, "actualInteractionsPerCrossing/F");
 // m_tree->Branch("averageInteractionsPerCrossing", &m_averageInteractionsPerCrossing, "averageInteractionsPerCrossing/F");

  m_tree->Branch("weight", &m_weight, "weight/F");
  m_tree->Branch("weight_xs", &m_weight_xs, "weight_xs/F");
  m_tree->Branch("weight_mcEventWeight", &m_weight_mcEventWeight, "weight_mcEventWeight/F");
  m_tree->Branch("weight_prescale", &m_weight_prescale, "weight_prescale/F");

  m_tree->Branch("ptAsym", &m_ptAsym, "ptAsym/F");
  m_tree->Branch("alpha", &m_alpha, "alpha/F");
  m_tree->Branch("avgBeta", &m_avgBeta, "avgBeta/F");
  m_tree->Branch("ptBal", &m_ptBal, "ptBal/F");
  m_tree->Branch("ptBal2", &m_ptBal2, "ptBal2/F");

  m_tree->Branch("recoilPt", &m_recoilPt, "recoilPt/F");
  m_tree->Branch("recoilEta", &m_recoilEta, "recoilEta/F");
  m_tree->Branch("recoilPhi", &m_recoilPhi, "recoilPhi/F");
  m_tree->Branch("recoilM", &m_recoilM, "recoilM/F");
  m_tree->Branch("recoilE", &m_recoilE, "recoilE/F");


}

void MiniTree::AddJetsUser(const std::string detailStr, const std::string jetName)
{
  // jet things
  m_tree->Branch("jet_detEta", &m_jet_detEta);
  m_tree->Branch("jet_beta", &m_jet_beta);
  m_tree->Branch("jet_corr", &m_jet_corr);

//  m_tree->Branch("jet_EMFrac", &m_jet_EMFrac);
//  m_tree->Branch("jet_HECFrac", &m_jet_HECFrac);
  m_tree->Branch("jet_TileFrac", &m_jet_TileFrac);

//  m_tree->Branch("jet_EnergyPerSampling", &m_jet_EnergyPerSampling);
    //just do this for first jet?
//  vector < vector< float > >  = jets->at(iJet)->auxdata< vector<float> >("EnergyPerSampling").at(iLayer)/1e3
//
}

void MiniTree::FillEventUser( const xAOD::EventInfo* eventInfo ) {

//  m_lumiBlock = eventInfo->lumiBlock();
//  m_actualInteractionsPerCrossing = eventInfo->actualInteractionsPerCrossing();
//  m_averageInteractionsPerCrossing = eventInfo->averageInteractionsPerCrossing();

  m_weight = eventInfo->auxdecor< float >( "weight" );
  m_weight_xs = eventInfo->auxdecor< float >( "weight_xs" );
  m_weight_mcEventWeight = eventInfo->auxdecor< float >( "weight_mcEventWeight" );
  m_weight_prescale = eventInfo->auxdecor< float >( "weight_prescale" );

  m_ptAsym = eventInfo->auxdecor< float >( "ptAsym" );
  m_alpha = eventInfo->auxdecor< float >( "alpha" );
  m_avgBeta = eventInfo->auxdecor< float >( "avgBeta" );
  m_ptBal = eventInfo->auxdecor< float >( "ptBal" );
  m_ptBal2 = eventInfo->auxdecor< float >( "ptBal2" );

  if( eventInfo->isAvailable< float >("recoilPt") )
    m_recoilPt = eventInfo->auxdecor< float >( "recoilPt" );
  else
    m_recoilPt = -999;

  m_recoilEta = eventInfo->auxdecor< float >( "recoilEta" );
  m_recoilPhi = eventInfo->auxdecor< float >( "recoilPhi" );
  m_recoilM = eventInfo->auxdecor< float >( "recoilM" );
  m_recoilE = eventInfo->auxdecor< float >( "recoilE" );
  m_trig = eventInfo->auxdecor< int > ( "trig" );

}

void MiniTree::FillJetsUser( const xAOD::Jet* jet, const std::string ) {

  if( jet->isAvailable< float >( "detEta" ) ) {
    m_jet_detEta.push_back( jet->auxdata< float >("detEta") );
  } else {
    m_jet_detEta.push_back( -999 );
  }

  if (jet->isAvailable< float >( "beta" ) ){
    m_jet_beta.push_back( jet->auxdata< float >("beta") );
  }else{
    m_jet_beta.push_back( -999 );
  }
  if (jet->isAvailable< float >( "jetCorr" ) ){
    m_jet_corr.push_back( jet->auxdata< float >("jetCorr") );
  }else{
    m_jet_corr.push_back( -999 );
  }
//  if (jet->isAvailable< float >( "EMFrac" ) ){
//    m_jet_EMFrac.push_back( jet->auxdata< float >("EMFrac") );
//  }else{
//    m_jet_EMFrac.push_back( -999 );
//  }
//  if (jet->isAvailable< float >( "HECFrac" ) ){
//    m_jet_HECFrac.push_back( jet->auxdata< float >("HECFrac") );
//  }else{
//    m_jet_HECFrac.push_back( -999 );
//  }
  if (jet->isAvailable< float >( "TileFrac" ) ){
    m_jet_TileFrac.push_back( jet->auxdata< float >("TileFrac") );
  }else{
    m_jet_TileFrac.push_back( -999 );
  }

//  std::vector<float> tempVector;
//  if( jet->isAvailable< std::vector<float> >("EnergyPerSampling") ){
//    tempVector = jet->auxdata< std::vector<float> >("EnergyPerSampling");
//  }
//  m_jet_EnergyPerSampling.push_back( tempVector );

}


void MiniTree::ClearEventUser() {
}

void MiniTree::ClearJetsUser(const std::string jetName ) {
  m_jet_detEta.clear();
  m_jet_beta.clear();
  m_jet_corr.clear();
//  m_jet_EMFrac.clear();
//  m_jet_HECFrac.clear();
  m_jet_TileFrac.clear();
//  m_jet_EnergyPerSampling.clear();
}


//void MiniTree::FillMuonsUser( const xAOD::Muon* muon ) {
//}
//void MiniTree::FillElectronsUser( const xAOD::Electron* electron ){
//}
//void MiniTree::FillFatJetsUser( const xAOD::Jet* fatJet ){
//}
//
