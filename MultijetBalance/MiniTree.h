#ifndef AnalysisExample_MiniTree_H
#define AnalysisExample_MiniTree_H
/** @file MiniTree.h
 *  @brief Manage the MJB TTree output
 *  @author Jeff Dandoy
 *  @bug No known bugs
 */

#include "xAODAnaHelpers/HelpTreeBase.h"
#include "TTree.h"

/**
  @brief Define and fill the MJB TTrees.  Inherits from xAODAnaHelpers::HelpTreeBase
*/
class MiniTree : public HelpTreeBase
{

  private:

    /** @brief Number of jets in the event*/
    int m_njet;
    /** @brief Luminosity block of the event */
    int m_lumiBlock;

    /** @brief Number of actual interactions per crossing*/
    //float m_actualInteractionsPerCrossing;
    /** @brief Average number of interactions per crossing*/
    float m_corrected_averageInteractionsPerCrossing;

    /** @brief Total weight of the event, a multiplication of other event weights*/
    float m_weight;
    /** @brief Cross-section weight of the event */
    float m_weight_xs;
    /** @brief MC simulation weight of the event */
    float m_weight_mcEventWeight;
    /** @brief Prescale weight of the event in data */
    float m_weight_prescale;
    /** @brief Pileup weight of the event in MC */
    float m_weight_pileup;

    /** @brief \f$p_{T}\f$ asymmetry variable*/
    float m_ptAsym;
    /** @brief \f$\alpha\f$ variable*/
    float m_alpha;
    /** @brief Average \f$\beta\f$ from all jets in an event*/
    float m_avgBeta;
    /** @brief \f$p_{T}\f$ balance of the leading jet over the recoil system */
    float m_ptBal;
    /** @brief An alternative representation of MiniTree#m_ptBal, defined as \f$ \frac{1}{2} \frac{p_{T}^{lead} + p_{T}^{recoil}}{p_{T}^{recoil}} \f$ */
    float m_ptBal2;

    /** @brief \f$p_{T}\f$ of the recoil system*/
    float m_recoilPt;
    /** @brief \f$\eta\f$ of the recoil system*/
    float m_recoilEta;
    /** @brief \f$\phi\f$ of the recoil system*/
    float m_recoilPhi;
    /** @brief Mass of the recoil system*/
    float m_recoilM;
    /** @brief Energy of the recoil system*/
    float m_recoilE;



    /** @brief Detector \f$\eta\f$ of each jet */
    std::vector<float> m_jet_detEta;
    /** @brief TileCorrected \f$p_{T}\f$ of each jet */
    std::vector<float> m_jet_TileCorrectedPt;
    /** @brief \f$\beta\f$ of each jet */
    std::vector<float> m_jet_beta;
    /** @brief Ratio of the final calibrated \f$p_{T}\f$ to the original input \f$p_{T}\f$ for each jet */
    std::vector<float> m_jet_corr;
    /** @brief Fraction of energy deposited in the EM calorimeter for each jet */
    std::vector<float> m_jet_EMFrac;
    /** @brief Fraction of energy deposited in the HEC calorimeter for each jet */
    std::vector<float> m_jet_HECFrac;
    /** @brief Fraction of energy deposited in the Tile calorimeter for each jet */
    std::vector<float> m_jet_TileFrac;
    /** @brief Jvt for each jet */
    std::vector<float> m_jet_Jvt;
    /** @brief Fraction of energy deposited in each calorimeter sampling layer for each jet */
    std::vector< std::vector<float> > m_jet_EnergyPerSampling;

    /** @brief Name of each b-tagging working point*/
    std::vector< std::string > m_jet_BTagNames;
    /** @brief B-tagging decision for each working point for each jet*/
    std::vector< std::vector<int> > m_jet_BTagBranches;
    /** @brief MC b-tagging scale factor for each working point for each jet*/
    std::vector< std::vector<float> > m_jet_BTagSFBranches;

    /** @brief Some sort of trigger decision.  It appears this variable isn't used*/
    int m_trig;

  public:

    /** @brief Create the base HelpTreeBase instance */
    MiniTree(xAOD::TEvent * event, TTree* tree, TFile* file);
    /** @brief Standard destructor*/
    ~MiniTree();

    /** @brief Connect the branches for event-level variables */
    void AddEventUser( const std::string detailStr = "" );
    /** @brief Connect the branches for jet-level variables */
    void AddJetsUser( const std::string detailStr = "" , const std::string jetName = "jet");
    /** @brief Fill the TTree with event-level variables */
    void FillEventUser( const xAOD::EventInfo* eventInfo );
    /** @brief Fill the TTree with jet-level variables */
    void FillJetsUser( const xAOD::Jet* jet, const std::string jetName = "jet" );
    /** @brief Clear vectors used by event-level variables*/
    void ClearEventUser();
    /** @brief Clear vectors used by jet-level variables*/
    void ClearJetsUser(const std::string jetName = "jet");

//    void AddMJB(std::string detailStringMJB = "");
//    void FillEventUser( const xAOD::EventInfo* eventInfo );
//    void FillJetsUser( const xAOD::Jet* jet );
//    void ClearMJB();
///    void FillMuonsUser( const xAOD::Muon* muon );
///    void FillElectronsUser( const xAOD::Electron* electron );
///    void FillFatJetsUser( const xAOD::Jet* fatJet );

};
#endif
