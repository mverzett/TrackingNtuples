// -*- C++ -*-
//
// Class:      TrackingNtuples
//
/**\class TrackingNtuples TrackingNtuples.cc TrackingNtuples/TrackingNtuples/plugins/TrackingNtuples.cc

Description: [one line class summary]

         Implementation:
             [Notes on implementation]
*/
//
// Original Author:  Swapneel Sundeep Mehta
//         Created:  Mon, 24 Sep 2018 09:17:20 GMT
//
//


// system include files
#include <memory>
#include <utility>
#include <cstdint>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

// Including TrackReco/TrackBase and related files to retrieve the covariance matrix
// and the parameter set and store it in corresponding branches of the 
// output ROOT tree
#include "Rtypes.h"
#include "DataFormats/TrackReco/interface/TrackBase.h"
#include "DataFormats/TrackReco/interface/fillCovariance.h"
#include <algorithm>

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TMatrixD.h"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"

// Adding Message Logging Capabilities
#include "FWCore/MessageLogger/interface/MessageLogger.h"

// Adding DataFormats for tracking Rechits
#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHitCollection.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2D.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2DCollection.h"

#include "DataFormats/TrackerRecHit2D/interface/SiStripMatchedRecHit2D.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripMatchedRecHit2DCollection.h"

#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHitCollection.h"
#include "DataFormats/TrajectorySeed/interface/TrajectorySeedCollection.h"
#include "DataFormats/TrackCandidate/interface/TrackCandidateCollection.h"

#include "TrackingTools/TransientTrackingRecHit/interface/TransientTrackingRecHit.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"

// Include the library file required for localpoint
#include "DataFormats/GeometrySurface/interface/GloballyPositioned.h"

// #include "DataFormats/Common/interface/DetSet.h"

// Include the Dataformat for track association
#include "DataFormats/RecoCandidate/interface/TrackAssociation.h"

// Include data format for simhits
#include "SimDataFormats/TrackingHit/interface/PSimHit.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"

#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/SiPixelDigi/interface/PixelDigi.h"
#include "SimDataFormats/TrackerDigiSimLink/interface/PixelDigiSimLink.h"
#include "SimDataFormats/TrackerDigiSimLink/interface/StripDigiSimLink.h"

// Tracking Particle Clustering
#include "SimTracker/TrackHistory/interface/TrackClassifier.h"
#include "SimTracker/TrackerHitAssociation/interface/ClusterTPAssociation.h"
#include "SimTracker/TrackerHitAssociation/interface/TrackerHitAssociator.h"

#include "SimDataFormats/Associations/interface/TrackToTrackingParticleAssociator.h"

// Possibly needed for the SiPixelCluster definition
#include "RecoPixelVertexing/PixelLowPtUtilities/interface/ClusterData.h"

// Required for the Tracking Parrticle Association
#include "DataFormats/TrackerRecHit2D/interface/OmniClusterRef.h"

//
// class declaration
//

class MyTrackingNtuples : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit MyTrackingNtuples(const edm::ParameterSet&);
      ~MyTrackingNtuples();
   
   private:
	void reset_vectors();
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      // ----------member data ---------------------------
      int nevent_ = 0;
      int nlumi_ = 0;
      int nrun_ = 0;

      //used to select what tracks to read from configuration file
      edm::EDGetTokenT<edm::View<reco::Track> > tracksToken_; 
      
      //used to select extra track information to read 
      edm::EDGetTokenT<reco::TrackExtraCollection> trackExtraToken_;
      
      edm::Service<TFileService> fs_;
      TTree *tree_;
      
      // Track properties
      std::vector<double> eta_;
      std::vector<double> phi_;
      std::vector<double> qoverp_;
      std::vector<double> dxy_;
      std::vector<double> dsz_;
      std::vector<double> dz_;
      std::vector<int> track_tp_idx_;
      
      std::vector<double> eta_Error_;
      std::vector<double> phi_Error_;
      std::vector<double> qoverp_Error_;
      std::vector<double> dxy_Error_;
      std::vector<double> dsz_Error_;
      std::vector<double> dz_Error_;
      
      // Store the return values of parameter and 
      // covariance matrix functions
      reco::TrackBase::CovarianceMatrix covariance_mat_;
      reco::TrackBase::ParameterVector track_parameters_;
      
      // Handle the Reshaping of the Covariance Matrix
      std::vector<double> reshaped_cov_mat_;
      std::vector< std::vector<double> > covariance_array_;
          
      // Temporary Variables
      std::vector< std::vector<double> > tmpMatrix;
      std::vector<double> tmpVector1;
      std::vector<double> tmpVector2;
      std::vector<double> tmpVector3;
      std::vector<double> tmpVector4;
      std::vector<double> tmpVector5;

      // Declare default buffer size for SiStripRecHit Tracking
      int bufsize = 32000;

      // -------------------- Testing Rechit Retrieval --------------------------
      
      edm::EDGetTokenT< SiStripMatchedRecHit2DCollection > matchedRecHitToken_;
      edm::EDGetTokenT< SiStripRecHit2DCollection > rphiRecHitToken_;
      edm::EDGetTokenT< SiStripRecHit2DCollection > stereoRecHitToken_;
      edm::EDGetTokenT< SiPixelRecHitCollection > siPixelRecHitsToken_;

      edm::EDGetTokenT< reco::RecoToSimCollection > association_;

      // Tracking Particle Association
      edm::EDGetTokenT< ClusterTPAssociation > clusterTPMapToken_;

      std::vector<float> stereo_x_;
      std::vector<float> stereo_y_;
      std::vector<float> stereo_z_;
      std::vector<float> stereo_r_;
      std::vector<float> stereo_phi_;
      std::vector<float> stereo_eta_;
      std::vector< int > stereo_layer_;
      std::vector< int > stereo_tp_idx_;
      std::vector< int > stereo_hit_match_;
    //  std::vector< edm::Ref<TrackingParticle> > stereo_tp_ref_;
      //std::vector< TrackingParticle > stereo_tp_;

      std::vector<float> rphi_x_;
      std::vector<float> rphi_y_;
      std::vector<float> rphi_z_;
      std::vector<float> rphi_r_;
      std::vector<float> rphi_phi_;
      std::vector<float> rphi_eta_;
      std::vector< int > rphi_layer_;
      std::vector< int > rphi_tp_idx_;
      std::vector< int > rphi_hit_match_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//


//
// constructors and destructor
//
MyTrackingNtuples::MyTrackingNtuples(const edm::ParameterSet& iConfig)
 :
  tracksToken_(consumes< edm::View<reco::Track> >(iConfig.getParameter<edm::InputTag>("pixelTracks"))),
  trackExtraToken_(consumes<reco::TrackExtraCollection>(iConfig.getParameter<edm::InputTag>("pixelTracks"))),
  matchedRecHitToken_(consumes<SiStripMatchedRecHit2DCollection>(iConfig.getParameter<edm::InputTag>("matchedRecHits"))),
  rphiRecHitToken_(consumes<SiStripRecHit2DCollection>(iConfig.getParameter<edm::InputTag>("rphiRecHits"))),   
  stereoRecHitToken_(consumes<SiStripRecHit2DCollection>(iConfig.getParameter<edm::InputTag>("stereoRecHits"))),
  siPixelRecHitsToken_(consumes<SiPixelRecHitCollection>(iConfig.getParameter<edm::InputTag>("siPixelRecHits"))),
  association_(consumes< reco::RecoToSimCollection >(iConfig.getParameter<edm::InputTag>("associator"))),
  clusterTPMapToken_(consumes< ClusterTPAssociation >(iConfig.getParameter<edm::InputTag>("clusterTPMap")))
{
    gROOT->Reset();
    usesResource("TFileService");
    
    //now do what ever initialization is needed
    tree_ = fs_->make<TTree>("tree","tree");
    tree_->Branch("nevent", &nevent_, "nevent/I");
    tree_->Branch("nlumi", &nlumi_, "nlumi/I");
    tree_->Branch("nrun", &nrun_, "nrun/I");
    
    tree_->Branch("trackEta", &eta_);
    tree_->Branch("trackPhi", &phi_);
    tree_->Branch("qoverp", &qoverp_);
    tree_->Branch("dxy", &dxy_);
    tree_->Branch("dsz", &dsz_);
    tree_->Branch("trackTPIdx", &track_tp_idx_);

    tree_->Branch("trackEtaError", &eta_Error_);
    tree_->Branch("trackPhiError", &phi_Error_);
    tree_->Branch("qoverpError", &eta_Error_);
    tree_->Branch("dxyError", &dxy_Error_);
    tree_->Branch("dszError", &dsz_Error_);
    
    tree_->Branch("trackParameters", &track_parameters_);
    tree_->Branch("covarianceArray", &covariance_array_);

    tree_->Branch("stereoHitX", &stereo_x_);
    tree_->Branch("stereoHitY", &stereo_y_);
    tree_->Branch("stereoHitZ", &stereo_z_);
    tree_->Branch("stereoHitR", &stereo_r_);
    tree_->Branch("stereoHitPhi", &stereo_phi_);
    tree_->Branch("stereoHitEta", &stereo_eta_);
    tree_->Branch("stereoHitLayer", &stereo_layer_);
    tree_->Branch("stereoTPIndex", &stereo_tp_idx_);
    tree_->Branch("stereoHitMatch", &stereo_hit_match_);

    tree_->Branch("monoHitX", &rphi_x_);
    tree_->Branch("monoHitY", &rphi_y_);
    tree_->Branch("monoHitZ", &rphi_z_);
    tree_->Branch("monoHitR", &rphi_r_);
    tree_->Branch("monoHitPhi", &rphi_phi_);
    tree_->Branch("monoHitEta", &rphi_eta_);
    tree_->Branch("monoHitLayer", &rphi_layer_);
    tree_->Branch("monoTPIndex", &rphi_tp_idx_);
    tree_->Branch("monoHitMatch", &rphi_hit_match_);
}


MyTrackingNtuples::~MyTrackingNtuples()
{
    // do anything here that needs to be done at destruction time
    // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//

void MyTrackingNtuples::reset_vectors() {
    eta_.clear();
    eta_Error_.clear();
    phi_.clear();
    phi_Error_.clear();
    qoverp_.clear();
    qoverp_Error_.clear();
    dxy_.clear();
    dxy_Error_.clear();
    dsz_.clear();
    dsz_Error_.clear();
    covariance_array_.clear();
    reshaped_cov_mat_.clear();
    track_tp_idx_.clear();
		
    tmpMatrix.clear();
    
    stereo_x_.clear();
    stereo_y_.clear();
    stereo_z_.clear();
    stereo_r_.clear();
    stereo_phi_.clear();
    stereo_eta_.clear();
    stereo_layer_.clear(); 
    stereo_tp_idx_.clear();
    stereo_hit_match_.clear();
    
    rphi_x_.clear();
    rphi_y_.clear();
    rphi_z_.clear();
    rphi_r_.clear();
    rphi_phi_.clear();
    rphi_eta_.clear();
    rphi_layer_.clear();
    rphi_tp_idx_.clear();
    rphi_hit_match_.clear();
}

// ------------ method called for each event  ------------

void MyTrackingNtuples::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    using namespace edm;
    using namespace std;
    using namespace reco;

    nevent_ = iEvent.id().event();
    nlumi_ = iEvent.id().luminosityBlock();
    nrun_ = iEvent.id().run();
    
    std::cout << "Event Number: " << nevent_ << std::endl;
    std::cout << "Luminosity Block: " << nlumi_ << std::endl;
    std::cout << "Run Number: " << nrun_ << std::endl;
    
    // Reset all vectors and counters for tracks
    int numtracks_ = 0;
    reset_vectors();

    // Get the information from the pixeltrack branches
    Handle< edm::View<reco::Track> > tracks_;
    iEvent.getByToken(tracksToken_, tracks_);
    
    edm::Handle<reco::RecoToSimCollection> association;
    iEvent.getByToken(association_, association);    

    // Retrieving Cluster to Tracking Particle Association
    edm::Handle<ClusterTPAssociation> pCluster2TPListH;
    iEvent.getByToken(clusterTPMapToken_, pCluster2TPListH);
    const ClusterTPAssociation& clusterToTPMap_ = *pCluster2TPListH;
    
    int trackPrintCount_ = 0;

    for(size_t track_idx=0; track_idx<tracks_->size(); ++track_idx) {
        
        edm::RefToBase<reco::Track> trk_(tracks_, track_idx);
        
        //find the tracking particle based on the track
        auto gen_match = association->find(trk_);
        if(gen_match != association->end()) {
            auto tracking_particle_ = gen_match->val.front().first;
            if (trackPrintCount_ < 8) {
                std::cout << "Value at original variable: " << typeid(*tracking_particle_).name() << std::endl; 
                std::cout << "Address of value type track assoc.: " << typeid(&(*tracking_particle_)).name() << std::endl; 
                std::cout << "Address of value at trk assoc: " << &(*tracking_particle_) << std::endl;
                trackPrintCount_++;
            }

            track_tp_idx_.push_back(tracking_particle_.index());

		  } else {
                track_tp_idx_.push_back(-2);
          }
        
        // Add all the Track parameters and corresponding errors to the vectors
        // to be put into the TTree
        eta_.push_back(trk_->eta());
        eta_Error_.push_back(trk_->etaError());
        phi_.push_back(trk_->phi());
        phi_Error_.push_back(trk_->phi());
        qoverp_.push_back(trk_->qoverp());
        dsz_.push_back(trk_->dsz());
        dsz_Error_.push_back(trk_->dszError());
        dxy_.push_back(trk_->dxy());
        dxy_Error_.push_back(trk_->dxyError());
        dz_.push_back(trk_->dz());
        dz_Error_.push_back(trk_->dzError());
        
        covariance_mat_ = trk_->covariance();
        track_parameters_ = trk_->parameters();


        // Reshape the covariance matrix to store it in a fixed-dimension
        // vector of doubles
        for (int i_ = 0; i_ < 5; i_++) {
          for (int j_ = 0; j_ <= i_; j_++) {
            reshaped_cov_mat_.push_back(covariance_mat_[i_][j_]);
          }
        }

        // Push the reshaped vector into a container of multiple vectors
        // which will contain all the track covariances for an event
        covariance_array_.push_back(reshaped_cov_mat_);
        reshaped_cov_mat_.clear();
        
        numtracks_ ++;
    }
    
    std::cout << "Found " << numtracks_ << " tracks" << std::endl;;       
    
    // Get the extra information from the pixeltrack branches
    // We haven't really done anything with this though
    // Handle<reco::TrackExtraCollection> trackExtra_;
    // iEvent.getByToken(trackExtraToken_, trackExtra_);
            
// ----------------------- Get the RecHits from the Data -------------------

    // Find the collections of rechits and pixelhits
    edm::Handle<SiStripRecHit2DCollection> rphirechitColl_;
    edm::Handle<SiStripRecHit2DCollection> stereorechitColl_;
    edm::Handle<SiStripMatchedRecHit2DCollection> matchedrechitColl_;

    iEvent.getByToken(rphiRecHitToken_, rphirechitColl_);
    iEvent.getByToken(stereoRecHitToken_, stereorechitColl_);
    iEvent.getByToken(matchedRecHitToken_, matchedrechitColl_);
    
    // TODO: Write a function that initializes the value of all of the variables 
    // initNtuple();

    // Print size of rphirechits
    std::cout << "RPhiRecHitColl Data Size: " << (rphirechitColl_.product())->dataSize() << std::endl;
    std::cout << "StereoRecHitColl Data Size: " << (stereorechitColl_.product())->dataSize() << std::endl;
    int clusterFound_ = 0;
    int clusterMatched_ = 0;
    int noMatch_ = 0;
    int printCount_ = 0;
    
    // Approach to iterating over the stereorechits
    if((stereorechitColl_.product())->dataSize() > 0) {
        SiStripRecHit2DCollection::const_iterator stereorecHitIdIterator = (stereorechitColl_.product())->begin();
        SiStripRecHit2DCollection::const_iterator stereorecHitIdIteratorEnd = (stereorechitColl_.product())->end();
        for(SiStripRecHit2DCollection::const_iterator stereo_detunit_iterator_ = stereorecHitIdIterator;
            stereo_detunit_iterator_ != stereorecHitIdIteratorEnd; stereo_detunit_iterator_++) {
        
            SiStripRecHit2DCollection::DetSet stereo_detset_ = *stereo_detunit_iterator_;
            SiStripRecHit2DCollection::DetSet::const_iterator stereorechitRangeIteratorBegin = stereo_detset_.begin();
            SiStripRecHit2DCollection::DetSet::const_iterator stereorechitRangeIteratorEnd = stereo_detset_.end();
            SiStripRecHit2DCollection::DetSet::const_iterator stereo_iterRecHit_;
            
            for (stereo_iterRecHit_ = stereorechitRangeIteratorBegin; 
                  stereo_iterRecHit_ != stereorechitRangeIteratorEnd; ++stereo_iterRecHit_) {

                auto stereo_cluster_ = stereo_iterRecHit_->firstClusterRef();
                auto clusterTPMapIter_ = clusterToTPMap_.equal_range(stereo_cluster_);

                // TrackingParticle default_stereo_tp_;
                if (clusterTPMapIter_.first != clusterTPMapIter_.second) {
                    clusterFound_++;
                    auto stereo_tp_id_ = ((clusterTPMapIter_.first)->second);
                    
                    if (printCount_ < 5){
                        std::cout << "Original Stereo tp id: " << typeid(stereo_tp_id_).name() << std::endl; 
                        //std::cout << "Address of value type at stereo tp id: " << typeid(&(*stereo_tp_id_)).name() << std::endl; 
                        //std::cout << "Address of value at stereo tp id: " << &(*stereo_tp_id_) << std::endl; 
                        printCount_++;
                    }
                    
                    stereo_tp_idx_.push_back(stereo_tp_id_.index());
                    
                    if (std::find(track_tp_idx_.begin(), track_tp_idx_.end(), stereo_tp_id_.index()) != track_tp_idx_.end()){

                        stereo_hit_match_.push_back(1);
                        clusterMatched_++;
                    }

                } else {
                    noMatch_++;
                    stereo_hit_match_.push_back(0);
                    stereo_tp_idx_.push_back(-1);
                }

                // Obtain the local position in terms of coordinates and store it in the vector
                GlobalPoint stereo_gp = stereo_iterRecHit_->globalPosition(); 
                stereo_x_.push_back(stereo_gp.x());
                stereo_y_.push_back(stereo_gp.y());
                stereo_z_.push_back(stereo_gp.z());        
                stereo_r_.push_back(stereo_gp.perp());
                stereo_phi_.push_back(stereo_gp.phi());
                stereo_eta_.push_back(stereo_gp.eta());
                //stereo_layer_.push_back(); //How to get it?
            }
        }
    }
    std::cout << clusterFound_ << " Hits matched to Clusters" << std::endl;
    std::cout << clusterMatched_ << " Hits matched to Tracks" << std::endl;

    clusterFound_ = 0;
    clusterMatched_ = 0;
    noMatch_ = 0;
    printCount_ = 0;

    // Approach to iterating over the rphirechits/monorechits
    if((rphirechitColl_.product())->dataSize() > 0) {
        SiStripRecHit2DCollection::const_iterator rphirecHitIdIterator = (rphirechitColl_.product())->begin();
        SiStripRecHit2DCollection::const_iterator rphirecHitIdIteratorEnd = (rphirechitColl_.product())->end();

        for(SiStripRecHit2DCollection::const_iterator rphi_detunit_iterator_ = rphirecHitIdIterator;
            rphi_detunit_iterator_ != rphirecHitIdIteratorEnd; rphi_detunit_iterator_++) {
        
            SiStripRecHit2DCollection::DetSet rphi_detset = *rphi_detunit_iterator_;

            SiStripRecHit2DCollection::DetSet::const_iterator rechitRangeIteratorBegin = rphi_detset.begin();
            SiStripRecHit2DCollection::DetSet::const_iterator rechitRangeIteratorEnd   = rphi_detset.end();
            SiStripRecHit2DCollection::DetSet::const_iterator rphi_iterRecHit_;
        
            for (rphi_iterRecHit_ = rechitRangeIteratorBegin; 
                rphi_iterRecHit_ != rechitRangeIteratorEnd; ++rphi_iterRecHit_) {
          
                auto rphi_cluster_ = rphi_iterRecHit_->firstClusterRef();
                auto clusterTPMapIter_ = clusterToTPMap_.equal_range(rphi_cluster_);

                if (clusterTPMapIter_.first != clusterTPMapIter_.second) {
                    clusterFound_++;
                    auto rphi_tp_id_ = ((clusterTPMapIter_.first)->second);
                    
                    if (printCount_ < 5){
                        std::cout << "Original rphi tp id: " << typeid(rphi_tp_id_).name() << std::endl; 
                        //std::cout << "Address of value type at rphi tp id: " << typeid(&(*rphi_tp_id_)).name() << std::endl; 
                        //std::cout << "Address of value at rphi tp id: " << &(*rphi_tp_id_) << std::endl; 
                        printCount_++;
                    }
                    
                    rphi_tp_idx_.push_back(rphi_tp_id_.index());

                    if (std::find(track_tp_idx_.begin(), track_tp_idx_.end(), rphi_tp_id_.index()) != track_tp_idx_.end()){
                        rphi_hit_match_.push_back(1);
                        clusterMatched_++;
                    }


                } else {
                    noMatch_++;
                    rphi_hit_match_.push_back(0);
                    rphi_tp_idx_.push_back(-1);
                }

                // Obtain the local position in terms of coordinates and store it in the vector
                        GlobalPoint rphi_gp = rphi_iterRecHit_->globalPosition();
                        rphi_x_.push_back(rphi_gp.x());
                        rphi_y_.push_back(rphi_gp.y());
                        rphi_z_.push_back(rphi_gp.z());        
                        rphi_r_.push_back(  rphi_gp.perp());
                        rphi_phi_.push_back(rphi_gp.phi());
                        rphi_eta_.push_back(rphi_gp.eta());
                        // rphi_layer_.push_back(); //How to get it?
            }
        }
    }
    std::cout << clusterFound_ << " Hits matched to Clusters" << std::endl;
    std::cout << clusterMatched_ << " Hits matched to Tracks" << std::endl;
      
  // ------------------------------ Fill and Print the Tree -------------------------

  tree_->Fill();
  
  std::cout << "Tree filled" << std::endl;
  // tree_->Print();
}


// ------------ method called once each job just before starting event loop  ------------
void
MyTrackingNtuples::beginJob()
{
  // Defined a custom check for file pointer to store root ntuple
  if( !fs_ ){
        throw edm::Exception( edm::errors::Configuration,
                "TFile Service is not registered in cfg file" );
    }
}

// ------------ method called once each job just after ending the event loop  ------------
void
MyTrackingNtuples::endJob()
{
    // The Covariance Matrix and Track Parameters are cleared by gROOT->Reset()
    tmpMatrix.swap(covariance_array_);
    std::cout << ">> Ending job." << std::endl;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------

//define this as a plug-in
DEFINE_FWK_MODULE(MyTrackingNtuples);
