// -*- C++ -*-
//
// Package:    TrackingNtuples/TrackingNtuples
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

// Include the library file required for localpoint
#include "DataFormats/GeometrySurface/interface/GloballyPositioned.h"

// #include "DataFormats/Common/interface/DetSet.h"

// TODO: Check if this is necessary
// #include "DataFormats/TrackerRecHit2D/interface/Phase2TrackerRecHit1D.h"
// #include "DataFormats/TrackerRecHit2D/interface/TkCloner.h"

// For the Geometry - is this required? Probably at a later point.
/*
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/TrackerNumberingBuilder/interface/GeometricDet.h"
#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetType.h"
#include "Geometry/TrackerGeometryBuilder/interface/StripGeomDetType.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/GeometryVector/interface/LocalPoint.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
*/
//
// class declaration
//

class MyTrackingNtuples : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit MyTrackingNtuples(const edm::ParameterSet&);
      ~MyTrackingNtuples();
   
   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      // ----------member data ---------------------------
      int nevent_ = 0;
      int nlumi_ = 0;
      int nrun_ = 0;

      //used to select what tracks to read from configuration file
      edm::EDGetTokenT<reco::TrackCollection> tracksToken_; 
      
      //used to select extra track information to read 
      edm::EDGetTokenT<reco::TrackExtraCollection> trackExtraToken_;
      
      edm::Service<TFileService> fs_;
      TTree *tree_;
      
      // Track properties
      std::vector<double> jet_eta_;
      std::vector<double> jet_phi_;
      std::vector<double> qoverp_;
      std::vector<double> dxy_;
      std::vector<double> dsz_;
      
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
      //edm::EDGetTokenT<bool> doPixel_;
      //edm::EDGetTokenT<bool> doStrip_;
      
      // doPixel_( conf.getParameter<bool>("associatePixel") );
      // doStrip_( conf.getParameter<bool>("associateStrip") );
      
      edm::EDGetTokenT< SiStripMatchedRecHit2DCollection > matchedRecHitToken_;
      edm::EDGetTokenT< SiStripRecHit2DCollection > rphiRecHitToken_;
      edm::EDGetTokenT< SiStripRecHit2DCollection > stereoRecHitToken_;
      edm::EDGetTokenT< SiPixelRecHitCollection > siPixelRecHitsToken_;
      // edm::EDGetTokenT<edm::DetSetVector<Phase2TrackerRecHit1D> > siPhase2RecHitsToken_;

      std::vector<float> stereo_x;
      std::vector<float> stereo_y;
      std::vector<float> stereo_z;

      std::vector<float> rphi_x;
      std::vector<float> rphi_y;
      std::vector<float> rphi_z;

      std::vector<float> pixel_x;
      std::vector<float> pixel_y;
      std::vector<float> pixel_z;

      std::vector<float> stereo_r;
      std::vector<float> stereo_phi;
      std::vector<float> stereo_eta;

      // TODO: Try using a struct to save time and effort
      struct customEventData {  
          std::vector<double> jet_eta_;
          std::vector<double> jet_phi_;
          std::vector<double> qoverp_;
          std::vector<double> dxy_;
          std::vector<double> dsz_;
          
          reco::TrackBase::CovarianceMatrix covariance_mat_;
          reco::TrackBase::ParameterVector track_parameters_;
        };
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
//  src_(iConfig.getParameter<edm::InputTag>( "src" )),
  tracksToken_(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("pixelTracks"))),
  trackExtraToken_(consumes<reco::TrackExtraCollection>(iConfig.getParameter<edm::InputTag>("pixelTracks"))),
//  rphiRecHits_(consumes<std::vector<SiStripRecHit2D>&>(iConfig.getParameter<edm::InputTag>("rphiRecHits"))),
//  stereoRecHits_(consumes<std::vector<SiStripRecHit2D>&>(iConfig.getParameter<edm::InputTag>("stereoRecHits"))),
//  matchedRecHits_(consumes<std::vector<SiStripRecHit2D>&>(iConfig.getParameter<edm::InputTag>("matchedRecHits"))),
  matchedRecHitToken_(consumes<SiStripMatchedRecHit2DCollection>(iConfig.getParameter<edm::InputTag>("matchedRecHits"))),
  rphiRecHitToken_(consumes<SiStripRecHit2DCollection>(iConfig.getParameter<edm::InputTag>("rphiRecHits"))),   
//  siPhase2RecHitsToken_(consumes<edm::DetSetVector<Phase2TrackerRecHit1D> >(iConfig.getParameter<edm::InputTag>("siPhase2RecHits"))),
  stereoRecHitToken_(consumes<SiStripRecHit2DCollection>(iConfig.getParameter<edm::InputTag>("stereoRecHits"))),
  siPixelRecHitsToken_(consumes<SiPixelRecHitCollection>(iConfig.getParameter<edm::InputTag>("siPixelRecHits")))
{
    gROOT->Reset();
    usesResource("TFileService");
    
    //now do what ever initialization is needed
    tree_ = fs_->make<TTree>("tree","tree");
    tree_->Branch("nevent", &nevent_, "nevent/I");
    tree_->Branch("nlumi", &nlumi_, "nlumi/I");
    tree_->Branch("nrun", &nrun_, "nrun/I");
    
    tree_->Branch("jeteta", &jet_eta_, "jeteta/D");
    tree_->Branch("jetphi", &jet_phi_, "jetphi/D");
    tree_->Branch("qoverp", &jet_eta_, "qoverp/D");
    tree_->Branch("dxy", &dxy_);
    tree_->Branch("dsz", &dsz_);

    tree_->Branch("trackparameters", &track_parameters_);
    tree_->Branch("covariancearray", &covariance_array_);

    //tree_->Branch("NStereoHits",&NStereoHits_,"NStereoHits/I");
    tree_->Branch("StereoHitX",&stereo_x,"StereoHitX[1000]/F");
    tree_->Branch("StereoHitY",&stereo_y,"StereoHitY[1000]/F");
    tree_->Branch("StereoHitZ",&stereo_z,"StereoHitZ[1000]/F");

    tree_->Branch("MonoHitX",&rphi_x,"MonoHitX[1000]/F");
    tree_->Branch("MonoHitY",&rphi_y,"MonoHitY[1000]/F");
    tree_->Branch("MonoHitZ",&rphi_z,"MonoHitZ[1000]/F");

    tree_->Branch("PixelHitX",&pixel_x,"PixelHitX[1000]/F");
    tree_->Branch("PixelHitY",&pixel_y,"PixelHitY[1000]/F");
    tree_->Branch("PixelHitZ",&pixel_z,"PixelHitZ[1000]/F");
    /* 
    tree_->Branch("StereoHitSigX",&StereoHitSigX_,"StereoHitSigX[1000]/F");
    tree_->Branch("StereoHitSigY",&StereoHitSigY_,"StereoHitSigY[1000]/F");
    tree_->Branch("StereoHitCorr",&StereoHitCorr_,"StereoHitCorr[1000]/F");
 
    tree_->Branch("StereoHitSignal",&StereoHitSignal_,"StereoHitSignal[1000]/F");
    tree_->Branch("StereoHitNoise",&StereoHitNoise_,"StereoHitNoise[1000]/F");
    tree_->Branch("StereoHitWidth",&StereoHitWidth_,"StereoHitWidth[1000]/I");
    */

    // siRphiHitCollection_ = iConfig.getParameter<std::string>("siRphiHitCollection");
    // siStereoHitCollection_ = iConfig.getParameter<std::string>("siStereoHitCollection");
    // siMatchedHitCollection_ = iConfig.getParameter<std::string>("siMatchedHitCollection");



}


MyTrackingNtuples::~MyTrackingNtuples()
{
    // do anything here that needs to be done at destruction time
    // (e.g. close files, deallocate resources etc.)
}

/*void MyTrackingNtuples::initNtuple(){

  LogInfo("Tracks") << " In initNtuple " ;

  NStereoHits_ = -999 ;

  for (int init = 0 ; init < myMaxHits ; ++init){
    StereoHitX_[init] = -999.;
    StereoHitY_[init] = -999.;
    StereoHitZ_[init] = -999.;
    StereoHitR_[init] = -999.;
    StereoHitPhi_[init] = -999.;
    StereoHitTheta_[init] = -999.;

    StereoHitSignal_[init] = -999.;
    StereoHitNoise_[init] = -999.;
    StereoHitWidth_[init] = -999. ;
  }
}*/

//
// member functions
//

// ------------ method called for each event  ------------
void
MyTrackingNtuples::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    using namespace edm;
    using namespace std;
    using namespace reco;

    nevent_=iEvent.id().event();
    nlumi_=iEvent.id().luminosityBlock();
    nrun_=iEvent.id().run();
    
    std::cout << "Event Number: " << nevent_ << std::endl;
    std::cout << "Luminosity Block: " << nlumi_ << std::endl;
    std::cout << "Run Number: " << nrun_ << std::endl;
    
    // Set a counter to check the number of hits
    int numtracks_ = 0;
    
    jet_eta_.clear();
    jet_phi_.clear();
    qoverp_.clear();
    dsz_.clear();
    dxy_.clear();

    // TODO: Is this clearing the internal vectors 
    // as well as the container?
    covariance_array_.clear();

    // Get the information from the pixeltrack branches
    Handle<reco::TrackCollection> tracks_;
    iEvent.getByToken(tracksToken_, tracks_);

    for(reco::TrackCollection::const_iterator itTrack_ = tracks_->begin();
        itTrack_ != tracks_->end(); 
        ++itTrack_) {

        reco::Track trk_ = *itTrack_;
        
        jet_eta_.push_back(trk_.eta());
        jet_phi_.push_back(trk_.phi());
        qoverp_.push_back(trk_.qoverp());
        dsz_.push_back(trk_.dsz());
        dxy_.push_back(trk_.dxy());
        std::cout << "Jet Data: " << trk_.eta() << trk_.phi() << trk_.qoverp() << trk_.dxy() << trk_.dsz() << std::endl;
        
        covariance_mat_ = trk_.covariance();
        track_parameters_ = trk_.parameters();

        // TODO #1: Figure out how to store covariance
        // matrix in the TTree - Ntuples?

        std::cout << "Track Parameters: ";
        // Print the collected parameters from the parameter set
        for (int i_ = 0; i_ < track_parameters_.kSize; i_++) {
            std::cout << track_parameters_.At(i_);
        }
        std::cout << std::endl;

        // Reset counter variable
        // i_ = 0;

        // Print the covariance matrix (assume 5 x 5), eliminate the upper triangular half
        // and reshape it to store it in a fixed-dimension vector of doubles
        for (int i_ = 0; i_ < 5; i_++) {
          for (int j_ = 0; j_ <= i_; j_++) {
            // std::cout << "i: " << i_ << " j: " << j_ << std::endl;
            // std::cout << covariance_mat_[i_][j_] << " | ";
            reshaped_cov_mat_.push_back(covariance_mat_[i_][j_]);
          }
          // std::cout << std::endl;
        }

        std::cout << "Reshaped Covariance Matrix: " << reshaped_cov_mat_.size() << std::endl;
        for (int i_ = 0; (unsigned)i_ < reshaped_cov_mat_.size(); i_++) {
            std::cout << reshaped_cov_mat_.at(i_) << " ";
        }
        std::cout << std::endl;

        // Push the reshaped vector into a container of multiple vectors
        // which will contain all the track covariances for an event
        covariance_array_.push_back(reshaped_cov_mat_);
        reshaped_cov_mat_.clear();
        
        // std::cout << "Track Covariance and Parameter Set found" << std::endl;

        numtracks_ ++;
        LogInfo("Tracks") << "Found " << numtracks_ << " tracks" << std::endl;;       
        // do something with track parameters, e.g, plot the charge.
    }
    
    // Reset number of tracks to zero
    numtracks_ = 0;
    
    // Get the extra information from the pixeltrack branches
    Handle<reco::TrackExtraCollection> trackExtra_;
    iEvent.getByToken(trackExtraToken_, trackExtra_);

    /*  for(reco::TrackExtraCollection::const_iterator itTrackExtra_ = trackExtra_->begin();
        itTrackExtra_ != trackExtra_->end();
        ++itTrackExtra_) {
        numtracks_ ++;
        LogInfo("TrackExtra") << "Found extra info on " << numtracks_ << " tracks" << std::endl;
    }*/

    // std::cout << "Run in TrackExtra section " << numtracks_ << "times." << std::endl;

    // TODO #2: Print extra information about the tracks

// ----------------------- Get the RecHits from the Data -------------------

    // int pixelcounter_ = 0;
    // int stripcounter_ = 0;

    edm::Handle<SiPixelRecHitCollection> pixelrechitColl_;
    edm::Handle<SiStripRecHit2DCollection> rphirechitColl_;
    edm::Handle<SiStripRecHit2DCollection> stereorechitColl_;
    edm::Handle<SiStripMatchedRecHit2DCollection> matchedrechitColl_;

    iEvent.getByToken(siPixelRecHitsToken_, pixelrechitColl_);
    iEvent.getByToken(rphiRecHitToken_, rphirechitColl_);
    iEvent.getByToken(stereoRecHitToken_, stereorechitColl_);
    iEvent.getByToken(matchedRecHitToken_, matchedrechitColl_);
    
    // Write a function that initializes the value of all of the variables 
    // initNtuple();

    // Print size of rphirechits
    std::cout << "RPhiRecHitColl Data Size: " << (rphirechitColl_.product())->dataSize();
    std::cout << "StereoRecHitColl Data Size: " << (stereorechitColl_.product())->dataSize();
    std::cout << "PixelRecHitColl Data Size: " << (pixelrechitColl_.product())->dataSize();

    // Iterate over rphirechits and check if the begin/end methods work
    /*for(std::vector<SiStripRecHit2D>::const_iterator hiter = rphirechits_.begin();
        hiter != rphirechits_.end();
        ++hiter) {
      std::cout << ".";
    }
    std::cout << std::endl;*/

    // Different approach to iterating over the rphirechits
    
    /*
    if((stereorechitColl_.product())->dataSize() > 0) {
      SiStripRecHit2DCollection::const_iterator recHitIdIterator      = (recHitColl.product())->begin();
      SiStripRecHit2DCollection::const_iterator recHitIdIteratorEnd   = (recHitColl.product())->end();
 
      SiStripRecHit2DCollection::DetSet stereo_detset = *recHitIdIterator;

      SiStripRecHit2DCollection::DetSet::const_iterator rechitRangeIteratorBegin = stereo_detset.begin();
      SiStripRecHit2DCollection::DetSet::const_iterator rechitRangeIteratorEnd   = stereo_detset.end();
      SiStripRecHit2DCollection::DetSet::const_iterator iterRecHit_;
      
      for (iterRecHit_ = rechitRangeIteratorBegin; 
            iterRecHit_ != rechitRangeIteratorEnd; ++iterRecHit) {

        LocalPoint lp = iterRecHit_->localPosition();
        stereo_x.push_back(lp.x());
        stereo_y.push_back(lp.y());
        stereo_z.push_back(lp.z());        
      }
    }*/

    if((rphirechitColl_.product())->dataSize() > 0) {
      SiStripRecHit2DCollection::const_iterator recHitIdIterator      = (recHitColl.product())->begin();
      SiStripRecHit2DCollection::const_iterator recHitIdIteratorEnd   = (recHitColl.product())->end();
      // SiStripRecHit2DCollection::const_iterator iterRecHit_; 

      for(SiStripRecHit2DCollection::const_iterator detunit_iterator_ = recHitIdIterator;
        detunit_iterator_ != recHitIdIteratorEnd; detunit_iterator_++) {
        
        SiStripRecHit2DCollection::DetSet detset = *detunit_iterator_;

        SiStripRecHit2DCollection::DetSet::const_iterator rechitRangeIteratorBegin = detset.begin();
        SiStripRecHit2DCollection::DetSet::const_iterator rechitRangeIteratorEnd   = detset.end();
        SiStripRecHit2DCollection::DetSet::const_iterator iterRecHit_;
        
        for (iterRecHit_ = rechitRangeIteratorBegin; 
              iterRecHit_ != rechitRangeIteratorEnd; ++iterRecHit) {

          LocalPoint lp = iterRecHit_->localPosition();
          rphi_x.push_back(lp.x());
          rphi_y.push_back(lp.y());
          rphi_z.push_back(lp.z());        
        }
      }
    }
      
    const SiPixelRecHitCollection *hits = pixelrechitColl_.product();

    for(SiPixelRecHitCollection::DataContainer::const_iterator hit = hits->data().begin(),
      end = hits->data().end(); hit != end; ++hit) {

      /*
      // Is this a different (x, y) location ?

      std::vector< SiPixelCluster::Pixel > pixels(hit->cluster()->pixels());
      bool pixelOnEdge = false;
      for(std::vector< SiPixelCluster::Pixel >::const_iterator pixel = pixels.begin();
          pixel != pixels.end(); ++pixel) {
        int pixelX = pixel->x;
        int pixelY = pixel->y;
        
        // Here lay code to check if it is an edge pixel

      }*/

      LocalPoint lp = hit->localPosition();
      pixel_x.push_back(lp.x());
      pixel_y.push_back(lp.y());
      pixel_z.push_back(lp.z());
    }
  // ------------------------------ Fill and Print the Tree -------------------------

  tree_->Fill();
  tree_->Print();
}


// ------------ method called once each job just before starting event loop  ------------
void
MyTrackingNtuples::beginJob()
{
  if( !fs_ ){
        throw edm::Exception( edm::errors::Configuration,
                "TFile Service is not registered in cfg file" );
    }
}

// ------------ method called once each job just after ending the event loop  ------------
void
MyTrackingNtuples::endJob()
{
    tree_->Write();
    tree_->ResetBranchAddresses();
    
    // The Covariance Matrix and Track Parameters are cleared by gROOT->Reset()
    tmpMatrix.swap(covariance_array_);

    tmpVector1.swap(jet_eta_);
    tmpVector2.swap(jet_phi_);
    tmpVector3.swap(qoverp_);
    tmpVector4.swap(dxy_);
    tmpVector5.swap(dsz_);
    /*
    TVectorD tmpVector1;
    tmpVector1.swap(jet_eta_);
    TVectorD tmpVector2;
    tmpVector2.swap(jet_phi_);
    TVectorD tmpVector3;
    tmpVector3.swap(qoverp_);
    */
    
    //std::vector<covariance_mat_>.swap(covariance_mat_);
    //std::vector<track_parameters_>.swap(track_parameters_);

    std::cout << ">> Ending job." << std::endl;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------

//define this as a plug-in
DEFINE_FWK_MODULE(MyTrackingNtuples);
