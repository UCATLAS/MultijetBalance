
// initialize and configure the JVT correction tool
EL::StatusCode InsituBalanceAlgo :: loadJVTTool(){
  ANA_MSG_INFO("loadJVTTool()");

  //// Set up tagger tool to update JVT value after calibration
  setToolName( m_JVTUpdateTool_handle );
  ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JVTUpdateTool_handle, JetVertexTaggerTool) );
  ANA_CHECK( m_JVTUpdateTool_handle.setProperty("JVTFileName","JetMomentTools/JVTlikelihood_20140805.root") );
  ANA_CHECK( m_JVTUpdateTool_handle.setProperty("JVFCorrName", m_JVTVar) );
  ANA_CHECK( m_JVTUpdateTool_handle.setProperty("OutputLevel", msg().level() ) );
  ANA_CHECK( m_JVTUpdateTool_handle.retrieve() );

  setToolName( m_JetJVTEfficiencyTool_handle );
  ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JetJVTEfficiencyTool_handle, CP::JetJvtEfficiency) );
  ANA_CHECK( m_JetJVTEfficiencyTool_handle.setProperty("WorkingPoint", m_JVTWP) );
  ANA_CHECK( m_JetJVTEfficiencyTool_handle.setProperty("OutputLevel", msg().level() ) );
  ANA_CHECK( m_JetJVTEfficiencyTool_handle.retrieve() );

  if(m_sysVariations.find("JVT") != std::string::npos){
    setToolName(m_JetJVTEfficiencyTool_handle_up);
    ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JetJVTEfficiencyTool_handle_up, CP::JetJvtEfficiency) );
    ANA_CHECK( m_JetJVTEfficiencyTool_handle_up.setProperty("WorkingPoint", "Tight") );
    ANA_CHECK( m_JetJVTEfficiencyTool_handle_up.setProperty("OutputLevel", msg().level() ) );
    ANA_CHECK( m_JetJVTEfficiencyTool_handle_up.retrieve() );

    setToolName(m_JetJVTEfficiencyTool_handle_down);
    ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JetJVTEfficiencyTool_handle_down, CP::JetJvtEfficiency) );
    ANA_CHECK( m_JetJVTEfficiencyTool_handle_down.setProperty("WorkingPoint", "Loose") );
    ANA_CHECK( m_JetJVTEfficiencyTool_handle_down.setProperty("OutputLevel", msg().level() ) );
    ANA_CHECK( m_JetJVTEfficiencyTool_handle_down.retrieve() );
  }


  return EL::StatusCode::SUCCESS;
}

EL::StatusCode InsituBalanceAlgo :: loadJetCalibrationTool(){

  ANA_MSG_INFO("loadJetCalibrationTool()");

  setToolName( m_JetCalibrationTool_handle );
  ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JetCalibrationTool_handle, JetCalibrationTool) );
  ANA_CHECK( m_JetCalibrationTool_handle.setProperty("JetCollection", m_jetDef) );
  ANA_CHECK( m_JetCalibrationTool_handle.setProperty("ConfigFile", m_jetCalibConfig) );
  ANA_CHECK( m_JetCalibrationTool_handle.setProperty("CalibSequence", m_jetCalibSequence) );
  ANA_CHECK( m_JetCalibrationTool_handle.setProperty("IsData", !m_isMC) );
  ANA_CHECK( m_JetCalibrationTool_handle.setProperty("OutputLevel", msg().level()) );
  ANA_CHECK( m_JetCalibrationTool_handle.retrieve() );

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode InsituBalanceAlgo :: loadJetCleaningTool(){
  ANA_MSG_INFO("loadJetCleaningTool()");

  setToolName( m_JetCleaningTool_handle );
  ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JetCleaningTool_handle, JetCleaningTool) );
  ANA_CHECK( m_JetCleaningTool_handle.setProperty("CutLevel", m_jetCleanCutLevel) );
  if (m_jetCleanUgly)  ANA_CHECK( m_JetCleaningTool_handle.setProperty("DoUgly", true) );
  ANA_CHECK( m_JetCleaningTool_handle.setProperty("OutputLevel", msg().level() ) );
  ANA_CHECK( m_JetCleaningTool_handle.retrieve() );

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode InsituBalanceAlgo :: loadJetUncertaintyTool(){
  ANA_MSG_INFO("loadJetUncertaintyTool()");

//!!Test    m_jetUncertaintyConfig = "/home/jdandoy/Documents/Dijet/Rel21MJB/JetUncertainties/share/JES_2016/Moriond2017/"+m_jetUncertaintyConfig;

  std::string tmp = PathResolverFindCalibFile(m_jetUncertaintyConfig);
  if( tmp.size() > 0)
    m_jetUncertaintyConfig = tmp;
  else
    return EL::StatusCode::SUCCESS;
//    m_jetUncertaintyConfig = "/home/jdandoy/Documents/Dijet/Rel21MJB/JetUncertainties/share/JES_2016/Moriond2017/"+m_jetUncertaintyConfig;


    std::cout << "config is " << m_jetUncertaintyConfig << std::endl;
    std::cout << "config is " << PathResolverFindCalibFile(m_jetUncertaintyConfig) << std::endl;
  setToolName( m_JetUncertaintiesTool_handle );
  ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JetUncertaintiesTool_handle, JetUncertaintiesTool) );
  ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("JetDefinition", m_jetDef) );
  ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("ConfigFile", m_jetUncertaintyConfig ) );
//!!  ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("ConfigFile", PathResolverFindCalibFile(m_jetUncertaintyConfig) ) );
  if( m_isAFII ){
    ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("MCType", "AFII") );
  }else{
    ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("MCType", "MC15") );
//!!    ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("MCType", "MC15") );
  }
  ANA_CHECK( m_JetUncertaintiesTool_handle.setProperty("OutputLevel", msg().level() ) );
  ANA_CHECK( m_JetUncertaintiesTool_handle.retrieve() );

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode InsituBalanceAlgo :: loadJetResolutionTool(){
  ANA_MSG_INFO("loadJetResolutionTool()");

  // Instantiate the JER Uncertainty tool
  setToolName( m_JERTool_handle );
  ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JERTool_handle, JERTool) );
  ANA_CHECK( m_JERTool_handle.setProperty("PlotFileName", m_JERUncertaintyConfig.c_str()) );
  ANA_CHECK( m_JERTool_handle.setProperty("CollectionName", m_jetDef) );
  ANA_CHECK( m_JERTool_handle.setProperty("OutputLevel", msg().level() ) );
  ANA_CHECK( m_JERTool_handle.retrieve() );

  // Instantiate the JER Smearing tool
  setToolName( m_JERSmearingTool_handle );
  ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JERSmearingTool_handle, JERSmearingTool) );
  ANA_CHECK( m_JERSmearingTool_handle.setProperty("JERTool", m_JERTool_handle.getHandle()) );
  ANA_CHECK( m_JERSmearingTool_handle.setProperty("isMC", m_isMC) );
  ANA_CHECK( m_JERSmearingTool_handle.setProperty("ApplyNominalSmearing", m_JERApplySmearing) );
  ANA_CHECK( m_JERSmearingTool_handle.setProperty("SystematicMode", m_JERSystematicMode) );
  ANA_CHECK( m_JERSmearingTool_handle.setProperty("OutputLevel", msg().level() ) );
  ANA_CHECK( m_JERSmearingTool_handle.retrieve() );

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode InsituBalanceAlgo :: loadJetTileCorrectionTool(){
  ANA_MSG_INFO("loadJetTileCorrectionTool()");

  setToolName( m_JetTileCorrectionTool_handle );
  ANA_CHECK( ASG_MAKE_ANA_TOOL(m_JetTileCorrectionTool_handle, CP::JetTileCorrectionTool) );
    //!!m_JetTileCorrectionTool_handle.setProperty("CorrectionFileName","JetTileCorrection/JetTile_pFile_010216.root");
  ANA_CHECK( m_JetTileCorrectionTool_handle.setProperty("OutputLevel", msg().level() ) );
  ANA_CHECK( m_JetTileCorrectionTool_handle.retrieve() );

  return EL::StatusCode::SUCCESS;
}

//Setup V+jet calibration and systematics files
EL::StatusCode InsituBalanceAlgo :: loadVjetCalibration(){
  ANA_MSG_INFO("loadVjetCalibration()");

  TFile *VjetFile = TFile::Open( PathResolverFindCalibFile(m_VjetCalibFile).c_str() , "READ" );

  //Only retrieve Nominal container
  std::string VjetHistName = m_jetDef+"_correction";
  m_VjetHist = (TH1F*) VjetFile->Get( VjetHistName.c_str() );
  if(!m_VjetHist){
    Error("loadVjetCalibration", "Could not load Vjet histogram %s for file %s", VjetHistName.c_str(), PathResolverFindCalibFile(m_VjetCalibFile).c_str() );
    return EL::StatusCode::FAILURE;
  }

  m_VjetHist->SetDirectory(0); //Detach histogram from file to memory
  VjetFile->Close();

  //// Find out the subleading jet pt cut for V+jets ////
  if( m_subjetThreshold.at(0) == -1000 ){
    m_subjetThreshold.at(0) = m_VjetHist->GetXaxis()->GetBinUpEdge( m_VjetHist->GetNbinsX() ) * 1e3;
    //float lastCalibFactor = m_VjetHist->GetBinContent( m_VjetHist->GetNbinsX() );
    //m_subjetThreshold.at(0) = floor( m_subjetThreshold.at(0)*(1./lastCalibFactor) );
    Info("loadVjetCalibration", "Setting first subleading jet pt threshold to %f, taken from the V+jet configuration file", m_subjetThreshold.at(0));
  }

  return EL::StatusCode::SUCCESS;
}

//Setup Previous MJB calibration and systematics files
EL::StatusCode InsituBalanceAlgo :: loadMJBCalibration(){
  ANA_MSG_INFO("loadMJBCalibration()");

  if(m_MJBIteration == 0 && !m_closureTest)
    return EL::StatusCode::SUCCESS;

  if( m_isMC )
    return EL::StatusCode::SUCCESS;

  // Load the file.  If m_closureTest, then apply the current iteration
  TFile* MJBFile = TFile::Open( PathResolverFindCalibFile(m_MJBCorrectionFile).c_str() , "READ" );
  ANA_MSG_INFO("Loaded MJB input file");
  if (m_closureTest)
    m_ss << m_MJBIteration;
  else
    m_ss << (m_MJBIteration-1);

  std::string mjbIterPrefix = "Iteration"+m_ss.str()+"_";
  std::string histPrefix = "DoubleMJB";

  m_ss.str( std::string() );

  // Get the MJB correction histograms in the input file
  TKey *key;
  TIter next(MJBFile->GetListOfKeys());
  while ((key = (TKey*) next() )){
    std::string dirName = key->GetName();
    if (dirName.find( mjbIterPrefix ) != std::string::npos) { //If it's a Iteration Dir
      TH1D *MJBHist;
      MJBHist = (TH1D*) MJBFile->Get( (dirName+"/"+histPrefix).c_str() );
      //Remove Iteration part of name
      std::string newHistName = dirName.substr(11);
      if( newHistName.find("MCType") == std::string::npos ){
        MJBHist->SetName( newHistName.c_str() );
        MJBHist->SetDirectory(0);
        m_MJBHists.push_back(MJBHist);
      }
    }
  }

  // Fix the systematics mappings to match the input MJB corrections
  std::vector<std::string> new_sysName;
  std::vector<int> new_sysType;
  std::vector<int> new_sysDetail;

  int foundCount = 0;
  // Loop over each input MJB systematic histogram
  for(unsigned int i=0; i < m_MJBHists.size(); ++i){
    bool foundMatch = false;
    std::string histName = m_MJBHists.at(i)->GetName();

    //Add the MCType systematic if it's in the input, as this isn't added regularly
    if( histName.find("MCType") != std::string::npos ){
      new_sysName.push_back( histName );
      new_sysType.push_back( m_sysType.at(m_NominalIndex) ); //Treat like nominal calibration
      new_sysDetail.push_back( m_sysDetail.at(m_NominalIndex) );
      foundMatch = true;
    } else {
      // Loop over the loaded systematics and find the match
      for(unsigned int iSys=0; iSys < m_sysName.size(); ++iSys){
        if( histName.find( m_sysName.at(iSys) ) != std::string::npos ){
          new_sysName.push_back( histName );
          new_sysType.push_back( m_sysType.at(iSys) );
          new_sysDetail.push_back( m_sysDetail.at(iSys) );
          foundMatch = true;
          break;
        }
      }//for m_sysName
    }// if searching m_sysName
    foundCount++;
    if( foundMatch == false){
        Error("loadMJBCalibration()", "Can't find Systematic Variation corresponding to MJB Correction %s. Exiting...", histName.c_str() );
        return EL::StatusCode::FAILURE;
    }
  }//for m_MJBHists

  //Fix the m_NominalIndex
  for(unsigned int i=0; i < m_MJBHists.size(); ++i){
    std::string histName = m_MJBHists.at(i)->GetName();
    if( histName.find("Nominal") != std::string::npos){
      m_NominalIndex = i;
      break;
    }
  }//for m_MJBHists

  //Replace systematic vectors with their new versions
  m_sysName    =  new_sysName;
  m_sysType   =  new_sysType;
  m_sysDetail =  new_sysDetail;

  Info("loadMJBCalibration()", "Succesfully loaded MJB calibration file");

  MJBFile->Close();

return EL::StatusCode::SUCCESS;
}



///////////////// Apply calibrations //////////////
EL::StatusCode InsituBalanceAlgo :: applyJetCalibrationTool( std::vector< xAOD::Jet*>* jets ){

  ANA_MSG_DEBUG("applyJetCalibrationTool()");
  for(unsigned int iJet=0; iJet < jets->size(); ++iJet){
    if ( m_JetCalibrationTool_handle->applyCorrection( *(jets->at(iJet)) ) == CP::CorrectionCode::Error ) {
      Error("execute()", "JetCalibrationTool reported a CP::CorrectionCode::Error");
      return StatusCode::FAILURE;
    }
  }// for each jet
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode InsituBalanceAlgo :: applyJetUncertaintyTool( xAOD::Jet* jet , unsigned int iSys ){
  ANA_MSG_DEBUG("applyJetUncertaintyTool with systematic " << m_sysName.at(iSys) );

   if( m_sysType.at(iSys) != JES ) //If not a JetUncertaintyTool sys variation
    return EL::StatusCode::SUCCESS;


  if ( m_JetUncertaintiesTool_handle->applySystematicVariation( m_sysSet.at(iSys) ) != CP::SystematicCode::Ok ) {
    Error("execute()", "Cannot configure JetUncertaintiesTool for systematic %s", m_sysName.at(iSys).c_str());
    return EL::StatusCode::FAILURE;
  }

  if ( m_JetUncertaintiesTool_handle->applyCorrection( *jet ) == CP::CorrectionCode::Error ) {
    Error("execute()", "JetUncertaintiesTool reported a CP::CorrectionCode::Error");
    return EL::StatusCode::FAILURE;
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode InsituBalanceAlgo :: applyJetResolutionTool( xAOD::Jet* jet , unsigned int iSys ){
  ANA_MSG_DEBUG("applyJetResolutionToolwith systematic " << m_sysName.at(iSys) );

  if( ( m_sysType.at(iSys) != JER ) // If not a JetResolutionTool sys variation
      && !(m_JERApplySmearing && m_isMC)  )  //If not applying smearing to MC
    return EL::StatusCode::SUCCESS;

  // Get systematic set, or nominal for m_JERApplySmearing
  CP::SystematicSet thisSysSet;
  if ( m_sysType.at(iSys) == JER )
    thisSysSet = m_sysSet.at(iSys);
  else
    thisSysSet = m_sysSet.at( m_NominalIndex ); //an empty SystematicSet, equal to the nominal JER smearing


//  std::cout << " for JES " << m_sysName.at(iSys) << " pt of " << jet->pt() << " to ";
  if ( m_JERSmearingTool_handle->applySystematicVariation( thisSysSet ) != CP::SystematicCode::Ok ) {
    Error("execute()", "Cannot configure JetResolutionTool for systematic %s", m_sysName.at(iSys).c_str());
    return EL::StatusCode::FAILURE;
  }

  if ( m_JERSmearingTool_handle->applyCorrection( *jet ) == CP::CorrectionCode::Error ) {
    Error("execute()", "JetResolutionTool reported a CP::CorrectionCode::Error");
    return EL::StatusCode::FAILURE;
  }
//  std::cout << jet->pt() << std::endl;

  return EL::StatusCode::SUCCESS;
}


// Apply the Vjet Calibration to jets
EL::StatusCode InsituBalanceAlgo :: applyVjetCalibration( std::vector< xAOD::Jet*>* jets ){
  ANA_MSG_DEBUG("applyVjetCalibration()");

  if(m_isMC)
    return EL::StatusCode::SUCCESS;

  for(unsigned int iJet=0; iJet < jets->size(); ++iJet){

    // Do not apply if leading jet or validation mode
    if( iJet == 0 || m_validation )   continue;

    xAOD::Jet* jet = jets->at(iJet);

    // Do not apply if not in V+jet correction range
    if( jet->pt()/1e3 < m_VjetHist->GetXaxis()->GetBinLowEdge(1)  ||
        jet->pt()/1e3 > m_VjetHist->GetXaxis()->GetBinUpEdge( m_VjetHist->GetNbinsX()+1 ) )
      continue;

    //Get nominal V+jet correction
    float thisCalibration = 1.;
    thisCalibration = m_VjetHist->GetBinContent( m_VjetHist->FindBin(jet->pt()/1e3) );
    if( thisCalibration == 0){
      Error("applyVjetCalibration", "Vjet calibration factor is 0!");
      return EL::StatusCode::FAILURE;
    }
    xAOD::JetFourMom_t thisJet;
    thisJet.SetCoordinates( jet->pt(), jet->eta(), jet->phi(), jet->m() );
    thisJet *= (1./thisCalibration);
    jet->setJetP4( thisJet );

  }


  return EL::StatusCode::SUCCESS;
}

EL::StatusCode InsituBalanceAlgo :: applyMJBCalibration( xAOD::Jet* jet , unsigned int iSys ){
  ANA_MSG_DEBUG("applyMJBCalibration()");

  if(m_isMC)
    return EL::StatusCode::SUCCESS;

  // Get calibration
  float thisCalibration = 1. / m_MJBHists.at(iSys)->GetBinContent( m_MJBHists.at(iSys)->FindBin(jet->pt()/1e3) );

  xAOD::JetFourMom_t thisJet;
  thisJet.SetCoordinates( jet->pt(), jet->eta(), jet->phi(), jet->m() );
  thisJet *= thisCalibration;
  jet->setJetP4( thisJet );

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode InsituBalanceAlgo :: reorderJets( std::vector< xAOD::Jet*>* theseJets ){

  ANA_MSG_VERBOSE("reorderJets()");
  xAOD::Jet* tmpJet = nullptr;
  for(unsigned int iJet = 0; iJet < theseJets->size(); ++iJet){
    for(unsigned int jJet = iJet+1; jJet < theseJets->size(); ++jJet){
      if( theseJets->at(iJet)->pt() < theseJets->at(jJet)->pt() ){
        tmpJet = theseJets->at(iJet);
        theseJets->at(iJet) = theseJets->at(jJet);
        theseJets->at(jJet) = tmpJet;
      }
    }//jJet
  }//iJet

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode InsituBalanceAlgo::applyJetSysVariation(std::vector< xAOD::Jet*>* theseJets, unsigned int iSys ){
  ANA_MSG_DEBUG("applyJetSysVariation()");

  // No further corrections for earlier jet calibration stages
  if(m_sysType.at(iSys) == JCS)
    return EL::StatusCode::SUCCESS;

  // Apply MJB iterative calibration when appropriate
  for(unsigned int iJet = 0; iJet < theseJets->size(); ++iJet){
    xAOD::Jet* jet = theseJets->at(iJet);

    // If validation mode, don't apply MJB calibrations
    if( m_validation )
      continue;

    // If closure test and leading jet
    // or if subleading jets above threshold
    if(  (m_closureTest && iJet==0)
      || (iJet > 0 && jet->pt() > m_subjetThreshold.at(0)) ){
      ANA_CHECK( applyMJBCalibration( jet , iSys ) );
    }

  }//MJB calibration


  //JetUncertainty comes after V+jet Calibration, as these files *should* have been generated using V+jet!
  for(unsigned int iJet = 0; iJet < theseJets->size(); ++iJet){
    xAOD::Jet* jet = theseJets->at(iJet);

    if( jet->pt() <= 16.*1e3)  // tool has issues below 16 GeV
      continue;

    // If validation mode
    // or if a subleading jet below threshold
    if(  m_validation
      || (iJet > 0 && jet->pt() <= m_subjetThreshold.at(0)) ){

      ANA_CHECK( applyJetUncertaintyTool( jet , iSys ) );
      ANA_CHECK( applyJetResolutionTool( jet , iSys ) );
    }

  }// for each jet


  return EL::StatusCode::SUCCESS;
}

EL::StatusCode InsituBalanceAlgo :: applyJetTileCorrectionTool( std::vector< xAOD::Jet*>* jets, unsigned int iSys  ){
  ANA_MSG_DEBUG("applyJetTileCorrectionTool()");

  if( m_isMC )
    return EL::StatusCode::SUCCESS;

  // No further corrections for earlier jet calibration stages
  if(m_sysType.at(iSys) == JCS)
    return EL::StatusCode::SUCCESS;

  for(unsigned int iJet=0; iJet < jets->size(); ++iJet){

    xAOD::Jet* jet = jets->at(iJet);
    xAOD::JetFourMom_t previous_P4;
    previous_P4.SetCoordinates( jet->pt(), jet->eta(), jet->phi(), jet->m() );

    if ( m_JetTileCorrectionTool_handle->applyCorrection( *jet ) == CP::CorrectionCode::Error ) {
      Error("execute()", "JetTileCorrectionTool reported a CP::CorrectionCode::Error");
      return StatusCode::FAILURE;
    }

    jet->auxdata< float >("TileCorrectedPt") = jet->pt();

    if(!m_TileCorrection)
      jet->setJetP4( previous_P4 );
  }//for jet


  return EL::StatusCode::SUCCESS;
}

