#ifndef MJB_MultijetHists_H
#define MJB_MultijetHists_H
/** @file MultijetHists.h
 *  @brief Manage the MJB histograms
 *  @author Jeff Dandoy
 *  @bug No known bugs
 */

#include "xAODAnaHelpers/JetHists.h"
#include "xAODAnaHelpers/HelperClasses.h"
#include <xAODJet/JetContainer.h>
#include <xAODJet/Jet.h>

/**
  @brief Define and fill the MJB histograms.  Inherits from xAODAnaHelpers::JetHists
*/
class MultijetHists : public JetHists
{
  public:
    /** @brief Plot extra histograms, currently not used*/
    bool f_extraMJBHists;
    /** @brief Save only the MultijetHists#m_recoilPt_ptBal histogram necessary for the MJB result
     * @note This histogram is used for determining the population within each recoil system \f$p_{T}\f$ bin, 
     * and getting the correct bin center*/
    bool f_minimalMJBHists;
    /** @brief Verbose mode*/
    bool m_debug;

    /** @brief Number of jets, in \f$p_{T}\f$ order, for which to save individual histograms*/
    int m_numSavedJets;
    /** @brief List of sampling layers for which to save individual histograms
     * @note Currently not used, and commented out in the code */
    std::vector<int> m_samplingLayers;
//TODO not used    int m_numPtBinnings;

    /** @brief Standrad constructor
     * @param name        The name of the MultijetHists isntance
     * @param detailStr   A string consists of substring details which determine which sets of histograms to include*/
    MultijetHists(std::string name, std::string detailStr);
    /** @brief Standard destructor*/
    ~MultijetHists() {};

    /** @brief Initialize all histograms
     * @param binning     The binning of the recoil system \f$p_{T}\f$*/
    StatusCode initialize(std::string binning);
    /** @brief Execute function for only the MJB-defined histograms 
     * @param jets        A vector of xAOD::Jet pointers
     * @param eventInfo   The xAOD::EventInfo of the event*/
    StatusCode execute( std::vector< xAOD::Jet* >* jets, const xAOD::EventInfo* eventInfo);
    /** @brief Execute function inherited from JetHists, filling non-MJB jet histograms
     * @param jets        An xAODLLJetContainer of the jets
     * @param eventWeight The weight to be applied to each event*/
    StatusCode execute( const xAOD::JetContainer* jets, float eventWeight) { return JetHists::execute( jets, eventWeight); };


  private:

    /** @brief Number of bins in the recoil system \f$p_{T}\f$ histogram */
    int m_numBins;

    //NLeadingJets
    //std::vector< std::vector< TH1F* > > m_MJBNjetsPt;       //!
    std::vector< TH1F* > m_MJBNjetsPt;       //!
    std::vector< TH1F* > m_MJBNjetsEta;      //!
    std::vector< TH1F* > m_MJBNjetsPhi;      //!
    std::vector< TH1F* > m_MJBNjetsM;        //!
    std::vector< TH1F* > m_MJBNjetsE;        //!
    std::vector< TH1F* > m_MJBNjetsRapidity; //!
    std::vector< TH1F* > m_MJBNjetsBeta; //!

    //std::vector< TH2F* > m_recoilPt_samplingLayer; //!
    //std::vector< TH2F* > m_recoilPt_samplingLayerPercent; //!
    //std::vector< TH2F* > m_leadJetPt_samplingLayer; //!
    //std::vector< TH2F* > m_leadJetPt_samplingLayerPercent; //!
    //std::vector< TH2F* > m_leadJetPt_trueSamplingLayerPercent; //!


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

    TH1F* m_recoilPt;    //!
    TH2F* m_recoilPt_jet0Pt;          //!
    TH2F* m_recoilPt_jet1Pt;          //!
    TH2F* m_recoilPt_avgBeta;         //!
    TH2F* m_recoilPt_alpha;           //!
    TH2F* m_recoilPt_njet;           //!
    TH2F* m_leadJetPt_jet1Pt;          //!
    TH2F* m_leadJetPt_avgBeta;         //!
    TH2F* m_leadJetPt_alpha;           //!
    TH2F* m_leadJetPt_njet;           //!

    TH2F* m_recoilPt_ptBal;   //!
    TH2F* m_leadJetPt_ptBal;   //!

    TH2F* m_recoilPt_ptBal_eta1;   //!
    TH2F* m_recoilPt_ptBal_eta2;   //!
    TH2F* m_recoilPt_ptBal_eta3;   //!

    TH2F* m_recoilPt_EMFrac; //!
    TH2F* m_recoilPt_HECFrac; //!
    TH2F* m_recoilPt_TileFrac; //!
    TH2F* m_leadJetPt_EMFrac ; //!
    TH2F* m_leadJetPt_HECFrac; //!
    TH2F* m_leadJetPt_TileFrac; //!

    //TH2F* m_recoilPt_averageIPC;           //!
    //TH2F* m_recoilPt_actualIPC;           //!
//    TH3F* m_leadJetPt_recoilPt_ptBal;  //!

};

#endif
