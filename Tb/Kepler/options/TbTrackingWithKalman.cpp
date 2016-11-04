// Gaudi
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiUtils/Aida2ROOT.h"
#include "GaudiUtils/HistoLabels.h"

// Tb/TbKernel
#include "TbKernel/TbFunctors.h"

// ROOT
#include "TMath.h"

#include <iostream>
#include <math.h>

// Local
#include "TbTracking.h"
#include <algorithm>

DECLARE_ALGORITHM_FACTORY(TbTracking)

//=============================================================================
// Standard constructor
//=============================================================================
TbTracking::TbTracking(const std::string& name, ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator),
      m_tracks(NULL),
      m_trackFit(NULL),
      m_clusterFinder(NULL) {

  lowR = -0.2;
  highR = 0.2;
  binsR = 500;
  
  lowS = -0.02;
  highS = 0.02;

  // Declare algorithm properties - see header for description.
  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);
  declareProperty("Monitoring", m_monitoring = false);
  declareProperty("HitError2", m_hiterror2 = 1.6e-5);
  declareProperty("Scat2", m_scat2 = 1.2e-8);
  declareProperty("TimeWindow", m_twindow = 150. * Gaudi::Units::ns);
  declareProperty("MinNClusters", m_MinNClusters = 7);
  declareProperty("SearchRadius", m_vol_radius = 1 * Gaudi::Units::mm);
  declareProperty("SearchRadiusY", m_vol_radiusY = 1 * Gaudi::Units::mm);
  declareProperty("MaxChi2", m_ChiSqRedCut = 20000.);
  declareProperty("SearchVolume", m_search_3vol = "diabolo");
  declareProperty("VolumeAngle", m_vol_theta = 0.015);
  declareProperty("VolumeAngleY", m_vol_thetaY = 0.005);
  declareProperty("SearchVolumeFillAlgorithm",
                  m_ClusterFinderSearchAlgorithm = "adap_seq");
  declareProperty("nComboCut", m_nComboCut = 300);
  declareProperty("SearchPlanes", m_PlaneSearchOrder = {4, 3, 5});
}

//=============================================================================
// Destructor
//=============================================================================
TbTracking::~TbTracking() {

  if (m_clusterFinder) delete m_clusterFinder;
}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbTracking::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  // Setup the track fit tool.
  m_trackFit = tool<ITbTrackFit>("TbTrackFit", "Fitter", this);
  // Set up the cluster finder.
  m_clusterFinder =
      new TbClusterFinder(m_ClusterFinderSearchAlgorithm, m_nPlanes);
  
  // setup Kalman histos
  setup_hists();
  
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbTracking::execute() {

  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    const std::string clusterLocation = m_clusterLocation + std::to_string(i);
    LHCb::TbClusters* clusters = getIfExists<LHCb::TbClusters>(clusterLocation);
    if (!clusters) {
      error() << "No clusters in " << clusterLocation << endmsg;
      return StatusCode::FAILURE;
    }
    // Store the cluster iterators in the cluster finder.
    m_clusterFinder->setClusters(clusters, i);
  }
  
  // Clear Kalman track container
  ktracks_vec.clear();


  // Create a track container and transfer its ownership to the TES.
  m_tracks = new LHCb::TbTracks();
  put(m_tracks, LHCb::TbTrackLocation::Default);

  // Do the tracking and time order.
  performTracking();
  timeOrderTracks();

  // fill the histos with the Kalman fit results
  fill_khists(ktracks_vec);

  counter("NumberOfTracks") += m_tracks->size();

  return StatusCode::SUCCESS;
}

//=============================================================================
// Actual tracking
//=============================================================================
void TbTracking::performTracking() {
  // The working of this algorithm is similar to the cluster maker, such that:
  // - Loop over some set of seed clusters (those on the m_SeedPlanes).
  // - Create a container (TbTrackVolume) centered on each seed cluster
  // (in space and time).
  // - Impose the condition that any formed track must contain this seed.
  // - Evaluate that 4-volume, to see if it contained a track.
  // - If a choice (e.g. more than one possible combination of clusters),
  // take the best fitting. Priority is given to more complete tracks.
  // - If another track happened to be in the 4volume, but not
  //   containing this seed, then it will be found later.

  // Iterate over planes in the specified order.
  for (const auto& plane : m_PlaneSearchOrder) {
    // Iterate over this planes' clusters - first check for any.
    if (masked(plane) || m_clusterFinder->empty(plane)) continue;
    auto ic = m_clusterFinder->first(plane);
    const auto end = m_clusterFinder->end(plane);
    for (; ic != end; ++ic) {

      // Check if the seed has already been used.
      if ((*ic)->tracked()) continue;
      // Form the TbTrackVolume container, and fill with clusters that fall
      // in its space-time volume.
      TbTrackVolume* track_volume =
          new TbTrackVolume((*ic), m_search_3vol, m_nPlanes, m_twindow,
                            m_vol_radius, m_vol_radiusY, m_vol_theta,
                            m_vol_thetaY, m_MinNClusters);
      fillATrackVolume(track_volume);

      // Look for a track - automatically keeps if suitable.
      evaluateTrackVolume(track_volume);
      // PoorMansEvaluation(track_volume); // Alternative evaluator.

      delete track_volume;
    }
  }
}

//=============================================================================
// Fill 4volume.
//=============================================================================
void TbTracking::fillATrackVolume(TbTrackVolume* track_volume) {
  // Fills the given TbTrackVolume (that has a particular space-time volume)
  // with clusters from all planes (ie m_clusters) that fall in this space-time
  // volume.

  for (unsigned int iplane = 0; iplane < m_nPlanes; iplane++) {
    // Check for any clusters, or track contains seed condition.
    if (m_clusterFinder->empty(iplane) ||
        iplane == track_volume->seed()->plane() || masked(iplane))
      continue;

    // Create an iterator and find the appropriate cluster on the ith plane
    // whose TOA is at the start of this TbTrackVolumes' time window.
    auto ic = m_clusterFinder->getIterator(track_volume->t_lower(), iplane);
    const auto end = m_clusterFinder->end(iplane);
    // Loop until the TOA reaches the end of this TbTrackVolumes' time window.
    // (dummy end condition used in the for loop).
    for (; ic != end; ++ic) {
      // Add cluster if inside and not already tracked.
      track_volume->consider_cluster((*ic));
      // Stop if too far ahead in time.
      if ((*ic)->htime() > track_volume->t_upper()) break;
    }
  }
}

//=============================================================================
// Finding the best tracks
//=============================================================================
void TbTracking::evaluateTrackVolume(TbTrackVolume* track_volume) {
  // CURRENTLY, finds the most likely track containing either clusters on all
  // planes,
  // or tracks with one cluster missing.

  // Declare some variables to be used.
  LHCb::TbTrack* best_track = NULL;
  double best_chi_dof = -1;  // Something unphysical.

  // Get the number of combinations to check (depends on the size of the track
  // to be formed).
  int ncombos = track_volume->nCombos();

  track_volume->ResetComboCounters();
  // Do the search over this number of combinations - the TbTrackVolume has
  // methods
  // to retrive a particular combination of clusters forming a track (specified
  // by icombo).
  for (int icombo = 0; icombo < ncombos && icombo < m_nComboCut; icombo++) {

    // Get the icombo'th track and fit.
    LHCb::TbTrack* trial_track = track_volume->get_track_combo();
    // Sort the clusters by z-position.
    SmartRefVector<LHCb::TbCluster>& clusters =
      const_cast<SmartRefVector<LHCb::TbCluster>&>(trial_track->clusters());
    std::sort(clusters.begin(), clusters.end(),
              TbFunctors::LessByZ<const LHCb::TbCluster*>());
    m_trackFit->fit(trial_track);

    // Evaluate this track.
    const double chi2_per_dof = trial_track->chi2PerNdof();

    // First combination tried case.
    if (icombo == 0) {
      best_chi_dof = chi2_per_dof;
      best_track = trial_track;
      track_volume->update_best();  // TbTrackVolumes keep a record of the best
      // fitting combination of clusters internally.
    } else if (chi2_per_dof < best_chi_dof) {
      // Case of better combination.
      delete best_track;
      best_chi_dof = chi2_per_dof;
      best_track = trial_track;
      track_volume->update_best();
    } else
      delete trial_track;
    // Prepare for next combination.
    track_volume->increment_combo_counters();
  }

  if (best_track) {
    // At this point, found the best fitting track in the volume. Apply chi cut,
    // and save.
    if (best_chi_dof < m_ChiSqRedCut) {

      SmartRefVector<LHCb::TbCluster>& clusters =
        const_cast<SmartRefVector<LHCb::TbCluster>&>(best_track->clusters());
      auto earliest_hit = std::min_element(clusters.begin(), clusters.end(),
              TbFunctors::LessByTime<const LHCb::TbCluster*>());

      if( timingSvc()->beforeOverlap( (*earliest_hit)->time() ) ){

        track_volume->set_tracked_clusters();
        m_tracks->insert(best_track);
        
        // =========================== Kalman code =======================================
        // create a fit track object (which is actually also a TbTrack)
        LHCb::TbKalmanTrack* ktrack = new LHCb::TbKalmanTrack(*best_track, m_hiterror2, m_scat2) ;
        // fit it
        ktrack->fit() ;
        // store the ktrack in the track vector
        ktracks_vec.push_back(ktrack);
        // ===============================================================================
        
        best_track->setTime(timingSvc()->localToGlobal(best_track->htime()));
        if (m_monitoring) {
          plot(track_volume->nCombos(), "nComboDist_of_TrackVolumes",
               "nComboDist_of_TrackVolumes", 0., 1100., 1100);
          plot(track_volume->nclusters(), "nCluster_in_TrackVolumes",
               "nCluster_in_TrackVolumes", 0., 100., 100);
        }
      } 
      else{
  //      info() << "Earliest track time is within overlap, deleting" << endmsg;
 //       info() << *best_track << endmsg;
        delete best_track;
      }
     }
     else delete best_track;
  }
}

//=============================================================================
/// Poor mans tracker (useful for testing). - only finds complete tracks.
//=============================================================================
void TbTracking::poorMansEvaluation(TbTrackVolume* track_volume) {
  // Only accept tracks with at least one cluster on each plane.
  bool one_per_plane = true;  // Assumed, now checked.
  for (unsigned int i = 0; i < m_nPlanes; i++) {
    if (track_volume->m_clusters[i].size() == 0) {
      one_per_plane = false;
      break;
    }
  }

  if (one_per_plane) {
    double t = 0.0;
    // We have a track!
    LHCb::TbTrack* track = new LHCb::TbTrack();
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      LHCb::TbCluster* c = track_volume->nearest_cluster(i);
      t += c->htime();
      c->setAssociated(true);
      track->addToClusters(c);
    }
    t /= (double)m_nPlanes;
    m_trackFit->fit(track);
    m_tracks->insert(track);
    track->setTime(timingSvc()->localToGlobal(t));
  }
}

//=============================================================================
// Track time ordering - honeycomb
//=============================================================================
void TbTracking::timeOrderTracks() {

  const double s_factor = 1.3;
  LHCb::TbTrack* track1;
  LHCb::TbTrack* track2;
  unsigned int gap = m_tracks->size() / s_factor;
  bool swapped = false;

  // Start the swap loops.
  while (gap > 1 || swapped) {
    if (gap > 1) gap /= s_factor;
    swapped = false;  // Reset per swap loop.

    // Do the swaps.
    LHCb::TbTracks::iterator itt;
    for (itt = m_tracks->begin(); itt < m_tracks->end() - gap; ++itt) {
      track1 = *itt;
      track2 = *(itt + gap);
      if (track1->time() > track2->time()) {
        // Do the swap.
        std::iter_swap(itt, itt + gap);
        swapped = true;
      }
    }
  }
}

//=============================================================================
/// Fill Histograms for TbKalmanTracks
//=============================================================================
void TbTracking::fill_khists(std::vector<LHCb::TbKalmanTrack*>& ktracks) {
  
  std::vector<LHCb::TbKalmanTrack*>::iterator icktra;
  for (icktra = ktracks.begin(); icktra != ktracks.end(); icktra++) {
    
    // Fill the track histos
    m_Kfit_chi2->Fill((*icktra)->chi2());
    m_Kfit_prob->Fill( TMath::Prob((*icktra)->chi2(), (*icktra)->ndof()) );
    
    // Get the nodes of this TbKalmanTrack
    //const std::vector<LHCb::TbKalmanNode*>& knodes = (*icktra)->nodes();
    
    // Loop through the nodes of this TbKalmanTrack
    for( auto node : (*icktra)->nodes() ) {
      auto pixnode = dynamic_cast< LHCb::TbKalmanPixelMeasurement*>( node) ;
      if( pixnode ) {
        int ichip = pixnode->cluster().plane();
        
        // Fill unbiased residuals
        m_XunresKfit[ichip]->Fill( pixnode->residualX() * pixnode->covX() / pixnode->residualCovX() );
        m_YunresKfit[ichip]->Fill( pixnode->residualY() * pixnode->covY() / pixnode->residualCovY() );
        
        // Fill biased residuals
        m_XresKfit[ichip]->Fill( pixnode->residualX() );
        m_YresKfit[ichip]->Fill( pixnode->residualY() );
        
        // Fill biased residuals on X,Y
        m_XresKfitOnX[ichip]->Fill( pixnode->cluster().x() , pixnode->residualX() );
        m_XresKfitOnY[ichip]->Fill( pixnode->cluster().y(),  pixnode->residualX() );
        
        m_YresKfitOnY[ichip]->Fill( pixnode->cluster().y() , pixnode->residualY() );
        m_YresKfitOnX[ichip]->Fill( pixnode->cluster().x(),  pixnode->residualY() );
        
        // Fill biased residuals on X,Y slopes
        m_XresKfitOnTX[ichip]->Fill( pixnode->state().tx()  , pixnode->residualX() );
        m_XresKfitOnTY[ichip]->Fill( pixnode->state().ty() ,  pixnode->residualX() );
        
        m_YresKfitOnTY[ichip]->Fill( pixnode->state().ty()  , pixnode->residualY() );
        m_YresKfitOnTX[ichip]->Fill( pixnode->state().tx() ,  pixnode->residualY() );
        
        
        // Fill residual errors
        m_XreserrKfit[ichip]->Fill( std::sqrt(pixnode->residualCovX()) );
        m_YreserrKfit[ichip]->Fill( std::sqrt(pixnode->residualCovY()) );
        
        // Fill residual pulls
        m_XrespullKfit[ichip]->Fill( pixnode->residualX() / std::sqrt(pixnode->residualCovX() ) );
        m_YrespullKfit[ichip]->Fill( pixnode->residualY() / std::sqrt(pixnode->residualCovY() ) );
        
        // Fill chi2 quality histos :
        // take the chi2 of the track..
        LHCb::ChiSquare chi2 ( (*icktra)->chi2(), (*icktra)->ndof() ) ;
        // we should add a proper function to KalmanTrack, or store it
        
        // ..now calculate and subtract the chi2 of this hit: should become a function of node as well
        double resX = pixnode->residualX() ;
        double resY = pixnode->residualY() ;
        int nd = 2 * (  (*icktra)->nodes().size()  -1 ) - 4;
        LHCb::ChiSquare chi2hit  ( resX*resX / pixnode->residualCovX() + resY*resY / pixnode->residualCovY() , nd );
        chi2 -= chi2hit;
        // apply quality check
        if (chi2.chi2()/chi2.nDoF()<4) {
          // Fill quality biased residuals for this hit
          m_qXresKfit[ichip]->Fill( pixnode->residualX() );
          m_qYresKfit[ichip]->Fill( pixnode->residualY() );
          
          // Fill quality residual pulls for this hit
          m_qXrespullKfit[ichip]->Fill( pixnode->residualX() / std::sqrt(pixnode->residualCovX() ) );
          m_qYrespullKfit[ichip]->Fill( pixnode->residualY() / std::sqrt(pixnode->residualCovY() ) );
        }
        
      } // end of node check
    } // end of node loop
  }  // end of Ktrack loop
}


//=============================================================================
/// Setup Histograms
//=============================================================================
void TbTracking::setup_hists() {
  
  // TbKalmanTrack parameters plots
  m_Kfit_chi2 = Gaudi::Utils::Aida2ROOT::aida2root(
          book1D("KalmanFit/chi2", "Chi2", -0.5, 49.5, 100));
  
  m_Kfit_prob = Gaudi::Utils::Aida2ROOT::aida2root(
          book1D("KalmanFit/probability", "Chi2 prob of K fit", 0.0, 1.0, 100));
  
    
  // Kalman fit plots
  std::string hist_name;
  for (unsigned int i = 0; i < m_nPlanes; i++) {
    std::stringstream ss_chip;
    ss_chip << i;
    
    
    hist_name = "KalmanFit/UnbiasedResidualsX/plane_" + ss_chip.str();
    m_XunresKfit.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book1D(hist_name.c_str(), hist_name.c_str(), lowR, highR, binsR)));
    
    
    hist_name = "KalmanFit/UnbiasedResidualsY/plane_" + ss_chip.str();
    m_YunresKfit.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book1D(hist_name.c_str(), hist_name.c_str(), lowR, highR, binsR)));
    
    //
    
    hist_name = "KalmanFit/BiasedResidualsX/plane_" + ss_chip.str();
    m_XresKfit.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book1D(hist_name.c_str(), hist_name.c_str(), lowR, highR, binsR)));
    
    hist_name = "KalmanFit/BiasedResidualsY/plane_" + ss_chip.str();
    m_YresKfit.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book1D(hist_name.c_str(), hist_name.c_str(), lowR, highR, binsR)));
    //
    
    hist_name = "KalmanFit/ResidualsX_on_X/plane_" + ss_chip.str();
    m_XresKfitOnX.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book2D(hist_name.c_str(), hist_name.c_str(), -20., 20., 200, lowR, highR, binsR)));
    
    hist_name = "KalmanFit/ResidualsX_on_Y/plane_" + ss_chip.str();
    m_XresKfitOnY.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book2D(hist_name.c_str(), hist_name.c_str(), -20., 20., 200, lowR, highR, binsR)));
    
    
    hist_name = "KalmanFit/ResidualsY_on_Y/plane_" + ss_chip.str();
    m_YresKfitOnY.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book2D(hist_name.c_str(), hist_name.c_str(), -20., 20., 200, lowR, highR, binsR)));
    
    hist_name = "KalmanFit/ResidualsY_on_X/plane_" + ss_chip.str();
    m_YresKfitOnX.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book2D(hist_name.c_str(), hist_name.c_str(), -20., 20., 200, lowR, highR, binsR)));
    //
    
    hist_name = "KalmanFit/ResidualsX_on_slopeX/plane_" + ss_chip.str();
    m_XresKfitOnTX.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book2D(hist_name.c_str(), hist_name.c_str(), lowS, highS, binsR, lowR, highR, binsR)));
    
    hist_name = "KalmanFit/ResidualsX_on_slopeY/plane_" + ss_chip.str();
    m_XresKfitOnTY.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book2D(hist_name.c_str(), hist_name.c_str(), lowS, highS, binsR, lowR, highR, binsR)));
    
    
    hist_name = "KalmanFit/ResidualsY_on_slopeY/plane_" + ss_chip.str();
    m_YresKfitOnTY.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book2D(hist_name.c_str(), hist_name.c_str(), lowS, highS, binsR, lowR, highR, binsR)));
    
    hist_name = "KalmanFit/ResidualsY_on_slopeX/plane_" + ss_chip.str();
    m_YresKfitOnTX.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book2D(hist_name.c_str(), hist_name.c_str(), lowS, highS, binsR, lowR, highR, binsR)));
    
    //
    
    hist_name = "KalmanFit/Residual_errors_on_X/plane_" + ss_chip.str();
    m_XreserrKfit.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book1D(hist_name.c_str(), hist_name.c_str(), 0., 5.e-3, 1000)));
    
    
    hist_name = "KalmanFit/Residual_errors_on_Y/plane_" + ss_chip.str();
    m_YreserrKfit.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book1D(hist_name.c_str(), hist_name.c_str(), 0., 5.e-3, 1000)));
    
    
    hist_name = "KalmanFit/ResidualPull_on_X/plane_" + ss_chip.str();
    m_XrespullKfit.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book1D(hist_name.c_str(), hist_name.c_str(), -10., 10., 100)));
    
    
    hist_name = "KalmanFit/ResidualPull_on_Y/plane_" + ss_chip.str();
    m_YrespullKfit.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book1D(hist_name.c_str(), hist_name.c_str(), -10., 10., 100)));
    
    //
    
    hist_name = "KalmanFit/BiasedResidualsX_after_chi2_cut/plane_" + ss_chip.str();
    m_qXresKfit.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book1D(hist_name.c_str(), hist_name.c_str(), lowR, highR, binsR)));
    
    hist_name = "KalmanFit/BiasedResidualsY_after_chi2_cut/plane_" + ss_chip.str();
    m_qYresKfit.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book1D(hist_name.c_str(), hist_name.c_str(), lowR, highR, binsR)));
    
    //
    
    hist_name = "KalmanFit/ResidualPull_on_X_after_chi2_cut/plane_" + ss_chip.str();
    m_qXrespullKfit.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book1D(hist_name.c_str(), hist_name.c_str(), -10., 10., 100)));
    
    
    hist_name = "KalmanFit/ResidualPull_on_Y_after_chi2_cut/plane_" + ss_chip.str();
    m_qYrespullKfit.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
            book1D(hist_name.c_str(), hist_name.c_str(), -10., 10., 100)));
    
    
  }
}

