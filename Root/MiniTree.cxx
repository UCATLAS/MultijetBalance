#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODEventInfo/EventInfo.h"

#include "MultijetBalance/MiniTree.h"


MiniTree :: MiniTree(xAOD::TEvent * event, TTree* tree, TFile* file) :
  HelpTreeBase(event, tree, file, 1e3)
{
  if ( m_debug ) Info("MiniTree", "Creating output TTree %s", tree->GetName());
}

MiniTree :: ~MiniTree()
{
}

void MiniTree::AddEventUser(std::string detailStr)
{

  m_tree->Branch("njet", &m_njet, "njet/I");
  m_tree->Branch("trig", &m_trig, "trig/I");

  m_tree->Branch("weight", &m_weight, "weight/F");
  m_tree->Branch("weight_xs", &m_weight_xs, "weight_xs/F");
  m_tree->Branch("weight_mcEventWeight", &m_weight_mcEventWeight, "weight_mcEventWeight/F");
  m_tree->Branch("weight_prescale", &m_weight_prescale, "weight_prescale/F");
  m_tree->Branch("weight_pileup", &m_weight_pileup, "weight_pileup/F");

  m_tree->Branch("ptAsym", &m_ptAsym, "ptAsym/F");
  m_tree->Branch("alpha", &m_alpha, "alpha/F");
  m_tree->Branch("avgBeta", &m_avgBeta, "avgBeta/F");
  m_tree->Branch("ptBal", &m_ptBal, "ptBal/F");

  m_tree->Branch("recoilPt", &m_recoilPt, "recoilPt/F");
  m_tree->Branch("recoilEta", &m_recoilEta, "recoilEta/F");
  m_tree->Branch("recoilPhi", &m_recoilPhi, "recoilPhi/F");
  m_tree->Branch("recoilM", &m_recoilM, "recoilM/F");
  m_tree->Branch("recoilE", &m_recoilE, "recoilE/F");


}

void MiniTree::AddJetsUser(const std::string detailStr, const std::string jetName)
{

  if(m_debug)  Info("AddJetsUser with detail string %s", detailStr);

  if (detailStr.find("extra") != std::string::npos)
    m_extraVar = true;

// The following can come from xAH?
//  // If saving b-tagging info, loop over each WP
//  if (detailStr.find("MJBbTag:") != std::string::npos){
//    std::string bTagStr = detailStr.substr(detailStr.find("MJBbTag:")+8, detailStr.size());
//    bTagStr = bTagStr.substr(0, detailStr.find_last_of(' '));
//
//    std::stringstream ssbTagStr(bTagStr);
//    std::string thisSubStr;
//    while (std::getline(ssbTagStr, thisSubStr, ',')) {
//      m_jet_BTagNames.push_back( thisSubStr );
//      std::vector<int> thisBTagBranch;
//      m_jet_BTagBranches.push_back( thisBTagBranch );
//      m_tree->Branch( ("jet_BTag_"+thisSubStr).c_str(), &m_jet_BTagBranches.at(m_jet_BTagBranches.size()-1) );
//      std::vector<float> thisBTagSFBranch;
//      m_jet_BTagSFBranches.push_back( thisBTagSFBranch );
//      m_tree->Branch( ("jet_BTagSF_"+thisSubStr).c_str(), &m_jet_BTagSFBranches.at(m_jet_BTagSFBranches.size()-1) );
//
//    }// for each btag WP
//  }
  m_tree->Branch("jet_PartonTruthLabelID", &m_jet_PartonTruthLabelID);

  m_tree->Branch("jet_detEta", &m_jet_detEta);
  m_tree->Branch("jet_TileCorrectedPt", &m_jet_TileCorrectedPt);
  m_tree->Branch("jet_beta", &m_jet_beta);
  m_tree->Branch("jet_corr", &m_jet_corr);
  m_tree->Branch("jet_Jvt", &m_jet_Jvt);

  if( m_extraVar ){
    m_tree->Branch("jet_EMFrac", &m_jet_EMFrac);
    m_tree->Branch("jet_HECFrac", &m_jet_HECFrac);
    m_tree->Branch("jet_TileFrac", &m_jet_TileFrac);
    m_tree->Branch("jet_EnergyPerSampling", &m_jet_EnergyPerSampling);
    //vector < vector< float > >  = jets->at(iJet)->auxdata< vector<float> >("EnergyPerSampling").at(iLayer)/1e3
  }


}

void MiniTree::FillEventUser( const xAOD::EventInfo* eventInfo ) {

  static SG::AuxElement::Accessor< float > weight("weight");
  if(weight.isAvailable( *eventInfo )){  m_weight = weight.isAvailable( *eventInfo ); }
  static SG::AuxElement::Accessor< float > weight_xs("weight_xs");
  if(weight_xs.isAvailable( *eventInfo )){  m_weight_xs = weight_xs.isAvailable( *eventInfo ); }
  static SG::AuxElement::Accessor< float > weight_mcEventWeight("weight_mcEventWeight");
  if(weight_mcEventWeight.isAvailable( *eventInfo )){  m_weight_mcEventWeight = weight_mcEventWeight.isAvailable( *eventInfo ); }
  static SG::AuxElement::Accessor< float > weight_prescale("weight_prescale");
  if(weight_prescale.isAvailable( *eventInfo )){  m_weight_prescale = weight_prescale.isAvailable( *eventInfo ); }
  static SG::AuxElement::Accessor< float > weight_pileup("PileupWeight");
  if(weight_pileup.isAvailable( *eventInfo )){  m_weight_pileup = weight_pileup.isAvailable( *eventInfo ); }

  static SG::AuxElement::Accessor< float > ptAsym("ptAsym");
  if(ptAsym.isAvailable( *eventInfo )){  m_ptAsym = ptAsym.isAvailable( *eventInfo ); }
  static SG::AuxElement::Accessor< float > alpha("alpha");
  if(alpha.isAvailable( *eventInfo )){  m_alpha = alpha.isAvailable( *eventInfo ); }
  static SG::AuxElement::Accessor< float > avgBeta("avgBeta");
  if(avgBeta.isAvailable( *eventInfo )){  m_avgBeta = avgBeta.isAvailable( *eventInfo ); }
  static SG::AuxElement::Accessor< float > ptBal("ptBal");
  if(ptBal.isAvailable( *eventInfo )){  m_ptBal = ptBal.isAvailable( *eventInfo ); }

  static SG::AuxElement::Accessor< float > recoilPt("recoilPt");
  if(recoilPt.isAvailable( *eventInfo )){  m_recoilPt = recoilPt.isAvailable( *eventInfo ); }
  static SG::AuxElement::Accessor< float > recoilEta("recoilEta");
  if(recoilEta.isAvailable( *eventInfo )){  m_recoilEta = recoilEta.isAvailable( *eventInfo ); }
  static SG::AuxElement::Accessor< float > recoilPhi("recoilPhi");
  if(recoilPhi.isAvailable( *eventInfo )){  m_recoilPhi = recoilPhi.isAvailable( *eventInfo ); }
  static SG::AuxElement::Accessor< float > recoilM("recoilM");
  if(recoilM.isAvailable( *eventInfo )){  m_recoilM = recoilM.isAvailable( *eventInfo ); }
  static SG::AuxElement::Accessor< float > recoilE("recoilE");
  if(recoilE.isAvailable( *eventInfo )){  m_recoilE = recoilE.isAvailable( *eventInfo ); }

  static SG::AuxElement::Accessor< int > trig("trig");
  if(trig.isAvailable( *eventInfo )){  m_trig = trig.isAvailable( *eventInfo ); }

}

void MiniTree::FillJetsUser( const xAOD::Jet* jet, const std::string ) {

  if( jet->isAvailable< float >( "detEta" ) ) {
    m_jet_detEta.push_back( jet->auxdata< float >("detEta") );
  } else {
    m_jet_detEta.push_back( -999 );
  }
  if( jet->isAvailable< float >( "TileCorrectedPt" ) ) {
    m_jet_TileCorrectedPt.push_back( jet->auxdata< float >("TileCorrectedPt")/1e3 );
  } else {
    m_jet_TileCorrectedPt.push_back( -999 );
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
  if( m_extraVar ) {
    if (jet->isAvailable< float >( "EMFrac" ) ){
      m_jet_EMFrac.push_back( jet->auxdata< float >("EMFrac") );
    }else{
      m_jet_EMFrac.push_back( -999 );
    }
    if (jet->isAvailable< float >( "HECFrac" ) ){
      m_jet_HECFrac.push_back( jet->auxdata< float >("HECFrac") );
    }else{
      m_jet_HECFrac.push_back( -999 );
    }
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
  if (jet->isAvailable< float >( "Jvt" ) ){
    m_jet_Jvt.push_back( jet->auxdata< float >("Jvt") );
  }else{
    m_jet_Jvt.push_back( -999 );
  }
  if (jet->isAvailable< int >( "PartonTruthLabelID" ) ){
    m_jet_PartonTruthLabelID.push_back( jet->auxdata< int >("PartonTruthLabelID") );
  }else{
    m_jet_PartonTruthLabelID.push_back( -999 );
  }


//  for(unsigned int iB=0; iB < m_jet_BTagNames.size(); ++iB){
//    std::string thisBTagName = m_jet_BTagNames.at(iB);
//
//
//    if( jet->isAvailable< int >( ("BTag_"+thisBTagName+"Fixed").c_str() ) ){
//      m_jet_BTagBranches.at(iB).push_back( jet->auxdata< int >( ("BTag_"+thisBTagName+"Fixed").c_str()) );
//    }else{
//      m_jet_BTagBranches.at(iB).push_back( -999 );
//    }
//
//    if( jet->isAvailable< float >( ("BTagSF_"+thisBTagName+"Fixed").c_str() ) ){
//      m_jet_BTagSFBranches.at(iB).push_back( jet->auxdata< float >( ("BTagSF_"+thisBTagName+"Fixed").c_str()) );
//    }else{
//      m_jet_BTagSFBranches.at(iB).push_back( -999 );
//    }
//  }//for iB branches



}


void MiniTree::ClearEventUser() {
}

void MiniTree::ClearJetsUser(const std::string jetName ) {
  m_jet_detEta.clear();
  m_jet_TileCorrectedPt.clear();
  m_jet_beta.clear();
  m_jet_corr.clear();
//  m_jet_EMFrac.clear();
//  m_jet_HECFrac.clear();
  m_jet_TileFrac.clear();
  m_jet_Jvt.clear();
  m_jet_PartonTruthLabelID.clear();

//  m_jet_EnergyPerSampling.clear();
  for(unsigned int iB=0; iB < m_jet_BTagNames.size(); ++iB){
    m_jet_BTagBranches.at(iB).clear();
    m_jet_BTagSFBranches.at(iB).clear();
  }
}

