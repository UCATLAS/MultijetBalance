#ifndef MJB_MultijetHists_H
#define MJB_MultijetHists_H

#include "xAODAnaHelpers/JetHists.h"
#include "xAODAnaHelpers/HelperClasses.h"
#include <xAODJet/JetContainer.h>
#include <xAODJet/Jet.h>

class MultijetHists : public JetHists
{
  public:

    bool f_extraMJBHists;
    bool f_minimalMJBHists;
    bool m_debug;

    int m_numSavedJets;
    std::vector<int> m_samplingLayers;
    int m_numPtBinnings;

    MultijetHists(std::string name, std::string detailStr);
    ~MultijetHists() {};

    StatusCode initialize(std::string binning);
    StatusCode execute( std::vector< xAOD::Jet* >* jets, const xAOD::EventInfo* eventInfo, int pvLoc );
    StatusCode execute( const xAOD::JetContainer* jets, float eventWeight, int pvLoc ) { return JetHists::execute( jets, eventWeight, pvLoc ); };


  private:

    //NLeadingJets
    //std::vector< std::vector< TH1F* > > m_MJBNjetsPt;       //!
    std::vector< TH1F* > m_MJBNjetsPt;       //!
    std::vector< TH1F* > m_MJBNjetsEta;      //!
    std::vector< TH1F* > m_MJBNjetsPhi;      //!
    std::vector< TH1F* > m_MJBNjetsM;        //!
    std::vector< TH1F* > m_MJBNjetsE;        //!
    std::vector< TH1F* > m_MJBNjetsRapidity; //!
    std::vector< TH1F* > m_MJBNjetsBeta; //!

    //std::vector< std::vector< TH2F* > > m_recoilPt_samplingLayer; //!
    //std::vector< std::vector< TH2F* > > m_recoilPt_samplingLayerPercent; //!
    ////std::vector< std::vector< TH2F* > > m_leadJetPt_samplingLayer; //!
    //std::vector< std::vector< TH2F* > > m_leadJetPt_samplingLayerPercent; //!
    //std::vector< std::vector< TH2F* > > m_leadJetPt_trueSamplingLayerPercent; //!


    TH1F* m_avgBeta;    //!
    TH1F* m_alpha;    //!
    TH1F* m_njet;    //!
    TH1F* m_ptAsym;    //!
    TH1F* m_ptBal;    //!
    TH1F* m_ptBal2;    //!
    TH2F* m_ptAsym_njet;            //!
    TH1F* m_recoilEta;    //!
    TH1F* m_recoilPhi;    //!
    TH1F* m_recoilM;    //!
    TH1F* m_recoilE;    //!
    TH1F* m_subOverRecoilPt;   //!
    TH1F* m_recoilPt_center ;  //!
    std::vector<TH1F*> m_recoilPt;    //!


    std::vector<TH2F*> m_recoilPt_jet0Pt;          //!
    std::vector<TH2F*> m_recoilPt_jet1Pt;          //!
    std::vector<TH2F*> m_recoilPt_avgBeta;         //!
    std::vector<TH2F*> m_recoilPt_alpha;           //!
    std::vector<TH2F*> m_recoilPt_njet;           //!
    //std::vector<TH2F*> m_recoilPt_averageIPC;           //!
    //std::vector<TH2F*> m_recoilPt_actualIPC;           //!
    std::vector<TH2F*> m_leadJetPt_jet1Pt;          //!
    std::vector<TH2F*> m_leadJetPt_avgBeta;         //!
    std::vector<TH2F*> m_leadJetPt_alpha;           //!
    std::vector<TH2F*> m_leadJetPt_njet;           //!

    std::vector<TH2F*> m_recoilPt_ptBal;   //!
    std::vector<TH2F*> m_leadJetPt_ptBal;   //!
//    std::vector<TH3F*> m_leadJetPt_recoilPt_ptBal;  //!

    std::vector<TH2F*> m_recoilPt_ptBal_eta1;   //!
    std::vector<TH2F*> m_recoilPt_ptBal_eta2;   //!
    std::vector<TH2F*> m_recoilPt_ptBal_eta3;   //!

    std::vector<TH2F*> m_recoilPt_EMFrac; //!
    std::vector<TH2F*> m_recoilPt_HECFrac; //!
    std::vector<TH2F*> m_recoilPt_TileFrac; //!
    std::vector<TH2F*> m_leadJetPt_EMFrac ; //!
    std::vector<TH2F*> m_leadJetPt_HECFrac; //!
    std::vector<TH2F*> m_leadJetPt_TileFrac; //!

};

#endif
