
//////////////////////////////// Selections //////////////////////////
bool MultijetBalanceAlgo:: cut_LeadEta(){
  ANA_MSG_DEBUG("Cut: lead jet detector eta");


  xAOD::JetFourMom_t jetConstituentP4 = m_jets->at(0)->getAttribute<xAOD::JetFourMom_t>("JetConstitScaleMomentum"); // or JetEMScaleMomentum
  m_jets->at(0)->auxdecor< float >( "detEta") = jetConstituentP4.eta();
  if( fabs(jetConstituentP4.eta()) > 1.2 )
    return false;
  else
    return true;
}

bool MultijetBalanceAlgo:: cut_JetEta(){

  for (unsigned int iJet = 0; iJet < m_jets->size(); ++iJet){
    xAOD::JetFourMom_t jetConstituentP4 = m_jets->at(iJet)->getAttribute<xAOD::JetFourMom_t>("JetConstitScaleMomentum"); // or JetEMScaleMomentum
    m_jets->at(iJet)->auxdecor< float >( "detEta") = jetConstituentP4.eta();
    if( fabs(jetConstituentP4.eta()) > 2.8 ){
      m_jets->erase( m_jets->begin()+iJet );
      --iJet;
    }
  }// for m_jets


  if (m_jets->size() >= m_numJets)
    return true;
  else
    return false;

}

bool MultijetBalanceAlgo:: cut_MCCleaning(){
  ANA_MSG_DEBUG("Cut: MC Cleaning");

  if(m_useMCPileupCheck && m_isMC){
    float pTAvg = ( m_jets->at(0)->pt() + m_jets->at(1)->pt() ) / 2.0;
    if( m_truthJets->size() == 0 || (pTAvg / m_truthJets->at(0)->pt() > 1.4) ){
      return false;
    }
  }

  return true;
}

bool MultijetBalanceAlgo:: cut_SubPt(){
  ANA_MSG_DEBUG("Cut: Subleading jet pt");

  if( m_jets->at(1)->pt() > m_subjetThreshold.at(m_MJBIteration) )
    return false;
  else
    return true;

}

bool MultijetBalanceAlgo:: cut_JetPtThresh() {

  ANA_MSG_DEBUG("Cut: jet pt threshold");

  float thisPtCut = m_ptThresh;
  if( m_sysType.at(m_iSys) == CUTPt )
    thisPtCut += m_sysDetail.at(m_iSys);

  for (unsigned int iJet = 0; iJet < m_jets->size(); ++iJet){
    if( m_jets->at(iJet)->pt()/1e3 < thisPtCut ){ //Default 25 GeV
      m_jets->erase(m_jets->begin()+iJet);
      --iJet;
    }
  }

  if (m_jets->size() >= m_numJets)
    return true;
  else
    return false;

}

bool MultijetBalanceAlgo:: cut_LeadJetPtThresh() {

  ANA_MSG_DEBUG("Cut: lead jet pt threshold");

  float thisLeadJetPtCut = m_leadJetPtThresh;
  if( m_jets->at(0)->pt()/1e3 >= thisLeadJetPtCut )
    return true;
  else
    return false;

}

bool MultijetBalanceAlgo:: cut_JVT() {

  ANA_MSG_DEBUG("Cut: JVT");
  for(unsigned int iJet = 0; iJet < m_jets->size(); ++iJet){
    m_jets->at(iJet)->auxdata< float >("Jvt") = m_JVTUpdateTool_handle->updateJvt( *(m_jets->at(iJet)) );

    if(  (m_sysType.at(m_iSys) == JVT && m_sysDetail.at(m_iSys) == -1 && !m_JetJVTEfficiencyTool_handle_down->passesJvtCut(*(m_jets->at(iJet))) )
      || (m_sysType.at(m_iSys) == JVT && m_sysDetail.at(m_iSys) == 1 && !m_JetJVTEfficiencyTool_handle_up->passesJvtCut(*(m_jets->at(iJet))) )
      || (!m_JetJVTEfficiencyTool_handle->passesJvtCut( *(m_jets->at(iJet))) ) ){
        m_jets->erase(m_jets->begin()+iJet);  --iJet;
    }
  }

  if (m_jets->size() >= m_numJets)
    return true;
  else
    return false;

}

//// Ignore event if any of the used jets are not clean ////
bool MultijetBalanceAlgo:: cut_CleanJet(){
  ANA_MSG_DEBUG("Cut: Jet Cleaning");
  for(unsigned int iJet = 0; iJet < m_jets->size(); ++iJet){
    if(! m_JetCleaningTool_handle->keep( *(m_jets->at(iJet))) ){
      return false;
    }//clean jet
  }

  return true;

}

bool MultijetBalanceAlgo:: cut_TriggerEffRecoil(){
  ///// Trigger Efficiency and prescale /////

  m_prescale = 1.;
  bool passedTriggers = false;
  if (m_triggers.size() == 0)
    passedTriggers = true;

  static SG::AuxElement::Decorator< std::vector< std::string > >  passTrigs("passTriggers");
  static SG::AuxElement::Decorator< std::vector< float > >        trigPrescales("triggerPrescales");
  std::vector< std::string > passingTriggers = passTrigs( *m_eventInfo );

  for( unsigned int iT=0; iT < m_triggers.size(); ++iT){
    if( m_recoilTLV.Pt() > m_triggerThresholds.at(iT)){
      auto trigIndex = std::find(passingTriggers.begin(), passingTriggers.end(), m_triggers.at(iT));
      if( trigIndex !=  passingTriggers.end() ){
        passedTriggers = true;
        m_prescale = trigPrescales( *m_eventInfo ).at( trigIndex-passingTriggers.begin() );
      }
      break; // only check 1 trigger in correct pt range
    }//recoil Pt
  } // each Trigger

  if( passedTriggers ){
    return true;
  } else{
    return false;
  }

}

bool MultijetBalanceAlgo:: cut_PtAsym(){
  //Remove dijet events, i.e. events where subleading jet dominates the recoil jets
  ANA_MSG_DEBUG("Cut: Pt asymmetry");

  //Var cut changes with recoil object pt, while the minimum pt cut never changes
  float thisAsymVarCut = m_ptAsymVar;
  float thisAsymMinCut = m_ptAsymMin;

  if( m_sysType.at(m_iSys) == CUTAsym ){
    thisAsymVarCut += m_sysDetail.at(m_iSys);
    // Change min cut systematic variable manually
    if( m_sysDetail.at(m_iSys)  > 0 )
      thisAsymMinCut += 10.; //GeV
    else
      thisAsymMinCut -= 10.;
  }

  float maxPtCut = fmax( thisAsymMinCut, thisAsymVarCut * m_recoilTLV.Pt() );

  if( m_jets->size() >= 2){
    m_eventInfo->auxdecor< float >( "ptAsym" ) = m_jets->at(1)->pt() / m_recoilTLV.Pt();
    if( m_jets->at(1)->pt() > maxPtCut )
      return false;
    else
      return true;
  } else {
    m_eventInfo->auxdecor< float >( "ptAsym" ) = 0.;
    return true;
  }

}

bool MultijetBalanceAlgo:: cut_Alpha(){
  //Alpha is phi angle between leading jet and recoilJet system
  ANA_MSG_DEBUG("Cut: Alpha (delta phi)");
  double alpha = fabs(DeltaPhi( m_jets->at(0)->phi(), m_recoilTLV.Phi() )) ;
  m_eventInfo->auxdecor< float >( "alpha" ) = alpha;

  float thisAlphaCut = m_alpha;
  if( m_sysType.at(m_iSys) == CUTAlpha )
    thisAlphaCut += m_sysDetail.at(m_iSys);

  if( (M_PI-alpha) > thisAlphaCut ){  //0.3 by default
    return false;
  }
  return true;
}

bool MultijetBalanceAlgo:: cut_Beta(){
  //Beta is phi angle between leading jet and each other passing jet
  ANA_MSG_DEBUG("Cut: Beta");

  double smallestBeta=10., avgBeta = 0., jetBeta=0.;

  for(unsigned int iJet=1; iJet < m_jets->size(); ++iJet){
    jetBeta = DeltaPhi(m_jets->at(iJet)->phi(), m_jets->at(0)->phi() );
    m_jets->at(iJet)->auxdecor< float >( "beta") = jetBeta;
    avgBeta += jetBeta;

    if( (jetBeta < smallestBeta) && (m_jets->at(iJet)->pt() > m_jets->at(0)->pt()*m_betaPtVar) )
      smallestBeta = jetBeta;
  }
  avgBeta /= (m_jets->size()-1);
  m_eventInfo->auxdecor< float >( "avgBeta" ) = avgBeta;

  float thisBetaCut = m_beta;
  if( m_sysType.at(m_iSys) == CUTBeta )
    thisBetaCut += m_sysDetail.at(m_iSys);

  if( smallestBeta < thisBetaCut ){
    return false;
  }

  return true;
}

/// MPF code just ignores any subleading photons
bool MultijetBalanceAlgo:: cut_ConvPhot(){
  // Remove Photons that are consistent with a converted photon
  ANA_MSG_DEBUG("Cut: Converted photon");

  if (xAOD::EgammaHelpers::conversionType(m_recoilPhoton) == 0)
    return true;

  double clusterEt = m_recoilPhoton->caloCluster()->e() / cosh(m_recoilPhoton->caloCluster()->eta());
  double trackPt  = xAOD::EgammaHelpers::momentumAtVertex(m_recoilPhoton).perp();
  if (xAOD::EgammaHelpers::conversionType(m_recoilPhoton) > 2){
    if (clusterEt / trackPt > 1.5 or clusterEt / trackPt < 0.5)
      return false;
  } else if (clusterEt / trackPt > 2.0){
    return false;
  }

  return true;
}

/// MPF code just ignores any subleading photons
bool MultijetBalanceAlgo:: cut_OverlapRemoval(){
  // Remove jets that overlap with a photon
  ANA_MSG_DEBUG("Cut: overlap removal");
  for(unsigned int iJet=0; iJet < m_jets->size(); ++iJet){
    if( m_jets->at(iJet)->p4().DeltaR(m_recoilTLV) < m_overlapDR ){
      m_jets->erase( m_jets->begin()+iJet );
      --iJet;
    }
  }//for iJet

  if (m_jets->size() >= m_numJets)
    return true;
  else
    return false;
}


//-----------------------------
//   for all jets
//   if (isPhoton and ( (*jet_itr)->p4().DeltaR(Ref) < ph_overlapRemoval)) -> remove jets
