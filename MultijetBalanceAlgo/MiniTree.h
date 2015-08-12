#ifndef AnalysisExample_MiniTree_H
#define AnalysisExample_MiniTree_H

#include "xAODAnaHelpers/HelpTreeBase.h"
#include "TTree.h"

class MiniTree : public HelpTreeBase
{

  private:

    int m_njet;
    int m_trig;
    int m_lumiBlock;

    float m_actualInteractionsPerCrossing;
    float m_averageInteractionsPerCrossing;

    float m_weight;
    float m_weight_xs;
    float m_weight_mcEventWeight;
    float m_weight_prescale;

    float m_ptAsym;
    float m_alpha;
    float m_avgBeta;
    float m_ptBal;
    float m_ptBal2;

    float m_recoilPt;
    float m_recoilEta;
    float m_recoilPhi;
    float m_recoilM;
    float m_recoilE;



    std::vector<float> m_jet_detEta;
    std::vector<float> m_jet_beta;
    std::vector<float> m_jet_corr;
    std::vector<float> m_jet_EMFrac;
    std::vector<float> m_jet_HECFrac;
    std::vector<float> m_jet_TileFrac;
    std::vector< std::vector<float> > m_jet_EnergyPerSampling;

  public:

    MiniTree(xAOD::TEvent * event, TTree* tree, TFile* file);
    ~MiniTree();

    void AddMJB(std::string detailStringMJB = "");
    void FillEventUser( const xAOD::EventInfo* eventInfo );
    void FillJetsUser( const xAOD::Jet* jet );
    void ClearMJB();
    void FillMuonsUser( const xAOD::Muon* muon );
    void FillElectronsUser( const xAOD::Electron* electron );
    void FillFatJetsUser( const xAOD::Jet* fatJet );

};
#endif
