// ROOT
#include "TMath.h"

// Gaudi
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiUtils/Aida2ROOT.h"

// Tb
#include "TbKernel/ITbGeometrySvc.h"
#include "Event/TbCluster.h"
#include "Event/TbTrack.h"
#include "Event/TbState.h"

#include "Event/TbKalmanTrack.h"
#include "Event/TbKalmanNode.h"

#include <math.h>

// Local
#include "TbTrackFitter.h"

/** @file TbTrackFitter.cpp
 *
 *  Implementation of class : TbTrackFitter
 *	Author: Panagiotis Tsopelas
 */

DECLARE_ALGORITHM_FACTORY(TbTrackFitter)

//=============================================================================
/// Standard constructor
//=============================================================================
TbTrackFitter::TbTrackFitter(const std::string& name, ISvcLocator* pSvcLocator)
    : GaudiTupleAlg(name, pSvcLocator), m_trackFit(NULL) {

  // Global default values.
  declareProperty("onOff_hists", m_onOff_hists = true);
  declareProperty("scat2", m_scat2 = 1.35e-8);
  declareProperty("hiterror2", m_hiterror2 = 1.6e-5);
  declareProperty("theta0", m_theta0 = 1.e-4);
  declareProperty("direction", m_direction = -1);
  // Local default values.
  m_nPlanes = 8;
  sigmax = 0.004 * Gaudi::Units::mm;
  sigmay = 0.004 * Gaudi::Units::mm;
}

//=============================================================================
/// Initialization
//=============================================================================
StatusCode TbTrackFitter::initialize() {

  StatusCode sc = GaudiAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  setHistoTopDir("Tb/");
  if (m_onOff_hists) setup_hists();

  // Setup the track fit tool.
  m_trackFit = tool<ITbTrackFit>("TbTrackFit", "Fitter", this);

  // Initialize the random number generator.
  sc = m_gauss.initialize(randSvc(), Rndm::Gauss(0.0, 1.0));
  if (!sc) {
    error() << "Cannot initialize Gaussian random number generator." << endmsg;
    return sc;
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
/// Main execution
//=============================================================================
StatusCode TbTrackFitter::execute() {
  if (msgLevel(MSG::DEBUG)) debug() << " ==> execute()" << endmsg;

  // TbTrack container
  std::vector<LHCb::TbTrack*> tracks_vec;
  // TbKalmanTrack container
  std::vector<LHCb::TbKalmanTrack*> ktracks_vec;
  
  const unsigned int nPlanes = m_nPlanes;
//  const double sigmax = 0.004 * Gaudi::Units::mm;
//  const double sigmay = 0.004 * Gaudi::Units::mm;

  // Gap from next/previous event
//  std::cout << std::endl << std::endl << std::endl ;

  const std::vector<double> scat2(nPlanes, m_scat2);
  // make random track parameters -> True straight track - NO scattering
  double Rx0 = m_gauss() * 15. * Gaudi::Units::mm ;
  double Rtx = m_gauss() * 1.e-4 ;
  double Ry0 = m_gauss() * 15. * Gaudi::Units::mm ;
  double Rty = m_gauss() * 1.e-4 ;
  // print random initial track parameters
//  std::cout << "--> Random track with x0 = " << Rx0 << " and tx = " << Rtx << std::endl;
 
  // theta0 for MCS through small angles
  double theta0 = TMath::ATan(m_theta0) ;
  // store true slope and intercept from scattering
  std::vector<double> scatRtx(nPlanes);
  std::vector<double> scatRty(nPlanes);
  
  // Declare TbTrack
  LHCb::TbTrack* track = new LHCb::TbTrack();

  // make random clusters ..
  std::vector<LHCb::TbCluster> clusters(nPlanes);
  
  // .. but fix their Z positions according to the Alignment File
  clusters[0].setZ( 0. );           clusters[4].setZ( 306. );
  clusters[1].setZ( 31. );          clusters[5].setZ( 336. );
  clusters[2].setZ( 60. );          clusters[6].setZ( 366. );
  clusters[3].setZ( 89. );          clusters[7].setZ( 397. );
  
  const double weight = 1. / (0.004 * 0.004); 
  // initialize 1st cluster
  clusters[0].setX( ( Rtx + Rx0 ) + m_gauss()*sigmax ); // random straight line
  clusters[0].setY( ( Rty + Ry0 ) + m_gauss()*sigmay ); // random straight line
  clusters[0].setXErr( 0.004 );
  clusters[0].setYErr( 0.004 );
  clusters[0].setWx(weight);
  clusters[0].setWy(weight);
  clusters[0].setPlane(0);
  track->addToClusters(&clusters[0]);
  
//  std::cout << " Clusters in X: " ;
//  std::cout << clusters[0].x() << ", " ;
  // update x0 and tx due to scattering
  scatRx0[0] = Rx0 ;
  scatRtx[0] = Rtx + m_gauss()*theta0;
  scatRy0[0] = Ry0;
  scatRty[0] = Rty + m_gauss()*theta0;
  
  // fill the rest of the clusters
  for (unsigned int i = 1; i < nPlanes; ++i) {
  
    // update x0 and tx due to scattering
    scatRx0[i] = ( clusters[i].z() - clusters[i-1].z() ) * scatRtx[i-1] + scatRx0[i-1] ;
    scatRtx[i] = scatRtx[i-1] + m_gauss() * theta0;
    scatRy0[i] = ( clusters[i].z() - clusters[i-1].z() ) * scatRty[i-1] + scatRy0[i-1];
    scatRty[i] = scatRty[i-1] + m_gauss() * theta0;
  
    clusters[i].setX( scatRx0[i] + m_gauss()*sigmax );
    clusters[i].setY( scatRy0[i] + m_gauss()*sigmay );
    clusters[i].setXErr( 0.004 );
    clusters[i].setYErr( 0.004 );
    clusters[i].setWx(weight);
    clusters[i].setWy(weight);
    clusters[i].setPlane(i);
    track->addToClusters(&clusters[i]);
    if (i==nPlanes) continue;
    
  }
  
  // const SmartRefVector<LHCb::TbCluster>& clustersL = track->clusters();
//  std::cout << std::endl << "Track with " << track->size() << " clusters. " << std::endl;
//  // print true positions of track
//  std::cout << " True X positions of (scattered) track: ";
//    std::cout << Rx0 << ",   " ;
//  for (int i=0; i<nPlanes; i++){
//    std::cout << scatRx0[i] << ",   " ;
//  }
//  std::cout << std::endl;
//  std::cout << " Cluster X positions                  :             ";
//  for (auto it = clustersL.cbegin(), end = clustersL.cend(); it != end; ++it) {
//    std::cout << (*it)->x() << ", ";
//  }
//  std::cout << " Cluster X errors                  :             ";
//  for (auto it = clustersL.cbegin(), end = clustersL.cend(); it != end; ++it) {
//    std::cout << (*it)->xErr() << ", ";
//  }
////  for (int i=0; i<nPlanes; i++){
////    std::cout << clusters[i].x() << ",   " ;
////  }
//
//  std::cout << std::endl;
//  std::cout << " Cluster Z positions                  :             ";
//  for (auto it = clustersL.cbegin(), end = clustersL.cend(); it != end; ++it) {
//    std::cout << (*it)->z() << ", ";
//  }
////  for (int i=0; i<nPlanes; i++){
////    std::cout << clusters[i].z() << ",   " ;
////  }
//  std::cout << std::endl;
  
  
  // fit the track
  m_trackFit->fit(track);



  // print results of simple fit
//  std::cout << std::endl << " * chi2 on x-z plane: " << track->chi2() << " with ndof = " << track->ndof() << std::endl
//			 << " x0: " << track->firstState().x() << " +/- " << TMath::Sqrt(track->firstState().errX2() )
//			 << ", y0: " << track->firstState().y() << " +/- " << TMath::Sqrt(track->firstState().errY2() )
//			 << std::endl 
//			 << " tx: " << track->firstState().tx() << " +/- " << TMath::Sqrt( track->firstState().errTx2() )
//			 << ", ty: " << track->firstState().ty() << " +/- " << TMath::Sqrt( track->firstState().errTy2() )
//			 << std::endl ;





  //---------------------------------------------------------------------------
  // Wouter's new code
  //---------------------------------------------------------------------------
  
  // node : combination of a measurement and a track state is referred to as a node
  // std::cout << std::endl << " ==================================================   " << std::endl;
  
  // create a fit track object (which is actually also a TbTrack)
  LHCb::TbKalmanTrack* ktrack = new LHCb::TbKalmanTrack(*track,  scat2);
  // fit it
  ktrack->fit() ;
  
  // dump some info about the fit
//  ktrack->print() ;
   // std::cout << " ==================================================   " << std::endl;
  //---------------------------------------------------------------------------


  // store the track in the track vector
  tracks_vec.push_back(track);
  // store the ktrack in the track vector
  ktracks_vec.push_back(ktrack);

  // Check whether to fill histograms
  if (m_onOff_hists) {
   fill_hists(tracks_vec);
   fill_khists(ktracks_vec);
  }

  // delete the pointer to the track
  delete track;

  return StatusCode::SUCCESS;
}

//=============================================================================
///               F  U  N  C  T  I  O  N  S
//=============================================================================

//=============================================================================
/// Fill Histograms for TbTracks
//=============================================================================
void TbTrackFitter::fill_hists(std::vector<LHCb::TbTrack*>& tracks) {

  std::vector<LHCb::TbTrack*>::iterator ictra;
  for (ictra = tracks.begin(); ictra != tracks.end(); ictra++) {

    // if ( TMath::Prob((*ictra)->chi2() , (*ictra)->ndof()) < 0.05) continue;

    // Fill the track histos
    m_slopesX->Fill((*ictra)->firstState().tx());
    m_slopesY->Fill((*ictra)->firstState().ty());
    m_Sfit_chi2->Fill((*ictra)->chi2PerNdof());
    m_Sfit_prob->Fill( TMath::Prob((*ictra)->chi2() , (*ictra)->ndof()) );

    // Get the clusters of this TbTrack
    const SmartRefVector<LHCb::TbCluster> clusters = (*ictra)->clusters();

    // Loop through the clusters of this TbTrack
    SmartRefVector<LHCb::TbCluster>::const_iterator icclu;
    for (icclu = clusters.begin(); icclu != clusters.end(); ++icclu) {
      int ichip = (*icclu)->plane();

      m_clustersX->Fill((*icclu)->x());
      m_clustersY->Fill((*icclu)->y());

      double xTraAtZclu = (*ictra)->firstState().x() +
                          (*ictra)->firstState().tx() * (double)(*icclu)->z();
      double yTraAtZclu = (*ictra)->firstState().y() +
                          (*ictra)->firstState().ty() * (double)(*icclu)->z();

      double resx = (*icclu)->x() - xTraAtZclu ;
      double resy = (*icclu)->y() - yTraAtZclu ;
      
      // Fill residuals
      m_resSfit_X[ichip]->Fill( resx );
      m_resSfit_Y[ichip]->Fill( resy );
      
      // Fill residual pulls
      m_respullSfit_X[ichip]->Fill((double) resx / sigmax);
      m_respullSfit_Y[ichip]->Fill((double) resy / sigmay);
      
      // Fill differences from true track
      m_trueSfit_X[ichip]->Fill( scatRx0[ichip] - xTraAtZclu );
      m_trueSfit_Y[ichip]->Fill( scatRy0[ichip] - yTraAtZclu );
      
      // Fill true pulls
      m_trackpullSfit_X[ichip]->Fill( (scatRx0[ichip] - xTraAtZclu) / std::sqrt((*ictra)->firstState().covariance()(0,0)) );
      m_trackpullSfit_Y[ichip]->Fill( (scatRy0[ichip] - yTraAtZclu) / std::sqrt((*ictra)->firstState().covariance()(1,1)) );
//      m_trackpullSfit_X[ichip]->Fill( (scatRx0[ichip] - xTraAtZclu) / sigmax );
//      m_trackpullSfit_Y[ichip]->Fill( (scatRy0[ichip] - yTraAtZclu) / sigmay );
      


    }  // end of cluster loop

  }  // end of track loop
}

//=============================================================================
/// Fill Histograms for TbKalmanTracks
//=============================================================================
void TbTrackFitter::fill_khists(std::vector<LHCb::TbKalmanTrack*>& ktracks) {
  
  std::vector<LHCb::TbKalmanTrack*>::iterator icktra;
  for (icktra = ktracks.begin(); icktra != ktracks.end(); icktra++) {
    
    // Fill the track histos
    m_Kfit_chi2->Fill((*icktra)->chi2());
    m_Kfit_prob->Fill( TMath::Prob((*icktra)->chi2(), (*icktra)->ndof()) );
    
    // Get the nodes of this TbKalmanTrack
    //const std::vector<LHCb::TbKalmanNode*>& knodes = (*icktra)->nodes();
    
    // Loop through the nodes of this TbKalmanTrack
    for( auto pixnode : (*icktra)->knodes() ) {
      if( pixnode ) {
        int ichip = pixnode->plane();
        
        // Fill unbiased residuals
        m_XunresKfit[ichip]->Fill( pixnode->residualX() * pixnode->covX() / pixnode->residualCovX() );
        m_YunresKfit[ichip]->Fill( pixnode->residualY() * pixnode->covY() / pixnode->residualCovY() );
        
        // Fill residuals
        m_resKfit_X[ichip]->Fill( pixnode->residualX() );
        m_resKfit_Y[ichip]->Fill( pixnode->residualY() );
        
        // Fill residual errors
        m_reserrKfit_X[ichip]->Fill( std::sqrt(pixnode->residualCovX()) );
        m_reserrKfit_Y[ichip]->Fill( std::sqrt(pixnode->residualCovY()) );
        
        // Fill residual pulls
        m_respullKfit_X[ichip]->Fill( pixnode->residualX() / std::sqrt(pixnode->residualCovX() ) );
        m_respullKfit_Y[ichip]->Fill( pixnode->residualY() / std::sqrt(pixnode->residualCovY() ) );
        
        // Fill differences from true tracks
        m_trueKfit_X[ichip]->Fill( scatRx0[ichip] - pixnode->state().x() );
        m_trueKfit_Y[ichip]->Fill( scatRy0[ichip] - pixnode->state().y() );
        
        // Fill differences errors from true tracks
        m_trueerrKfit_X[ichip]->Fill( TMath::Sqrt(pixnode->state().covariance()(0,0)) );
        m_trueerrKfit_Y[ichip]->Fill( TMath::Sqrt(pixnode->state().covariance()(1,1)) );
        
        // Fill true pulls
        m_trackpullKfit_X[ichip]->Fill( (scatRx0[ichip] - pixnode->state().x() ) / TMath::Sqrt(pixnode->state().covariance()(0,0)) );
        m_trackpullKfit_Y[ichip]->Fill( (scatRy0[ichip] - pixnode->state().y() ) / TMath::Sqrt(pixnode->state().covariance()(1,1)) );
        
      }
    }
  }  // end of Ktrack loop
}


//=============================================================================
/// Setup Histograms
//=============================================================================
void TbTrackFitter::setup_hists() {

  // TbTrack parameters plots
  m_Sfit_chi2 = Gaudi::Utils::Aida2ROOT::aida2root(
      book1D("StraightLineFit/chi2perndof", "Chi2/ndof", -0.5, 49.5, 100));
  
  m_Sfit_prob = Gaudi::Utils::Aida2ROOT::aida2root(
      book1D("StraightLineFit/probability", "Chi2 prob of S fit", 0.0, 1.0, 100));

  m_slopesX = Gaudi::Utils::Aida2ROOT::aida2root(
      book1D("StraightLineFit/slopesX", "slopes in X of TbTracks", -1.e-3, 1.e-3, 100));
  
  m_slopesY = Gaudi::Utils::Aida2ROOT::aida2root(
      book1D("StraightLineFit/slopesY", "slopes in Y of TbTracks", -1.e-3, 1.e-3, 100));

  m_Kfit_chi2 = Gaudi::Utils::Aida2ROOT::aida2root(
      book1D("KalmanFit/chi2", "Chi2", -0.5, 49.5, 100));
  
  m_Kfit_prob = Gaudi::Utils::Aida2ROOT::aida2root(
      book1D("KalmanFit/probability", "Chi2 prob of K fit", 0.0, 1.0, 100));
  
  m_clustersX = Gaudi::Utils::Aida2ROOT::aida2root(
      book1D("clustersX", "cluster X positions ", -1.e-2, 1.e-2, 100));

  m_clustersY = Gaudi::Utils::Aida2ROOT::aida2root(
      book1D("clustersY", "cluster Y positions ", -1.e-2, 1.e-2, 100));

  

  // Residual plots
  std::string hist_name;
  for (unsigned int i = 0; i < m_nPlanes; i++) {
    std::stringstream ss_chip;
    ss_chip << i;

    // straight line fit 
    hist_name = "StraightLineFit/Residuals_on_X/plane_" + ss_chip.str();
    m_resSfit_X.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -1.e-2, 1.e-2, 100)));

    hist_name = "StraightLineFit/Residuals_on_Y/plane_" + ss_chip.str();
    m_resSfit_Y.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -1.e-2, 1.e-2, 100)));


    hist_name = "StraightLineFit/ResidualPull_on_X/plane_" + ss_chip.str();
    m_respullSfit_X.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -10., 10., 100)));
    
    hist_name = "StraightLineFit/ResidualPull_on_Y/plane_" + ss_chip.str();
    m_respullSfit_Y.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -10., 10., 100)));


    hist_name = "StraightLineFit/Differences_from_true_on_X/plane_" + ss_chip.str();
    m_trueSfit_X.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -1.e-2, 1.e-2, 100)));
    
    hist_name = "StraightLineFit/Differences_from_true_on_Y/plane_" + ss_chip.str();
    m_trueSfit_Y.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -1.e-2, 1.e-2, 100)));


    hist_name = "StraightLineFit/TrackPull_on_X/plane_" + ss_chip.str();
    m_trackpullSfit_X.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -10., 10., 100)));

    hist_name = "StraightLineFit/TrackPull_on_Y/plane_" + ss_chip.str();
    m_trackpullSfit_Y.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -10., 10., 100)));
    

    
    // Kalman fit
    hist_name = "KalmanFit/UnbiasedResidualsX/plane_" + ss_chip.str();
    m_XunresKfit.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -1.e-2, 1.e-2, 100)));
    
    
    hist_name = "KalmanFit/UnbiasedResidualsY/plane_" + ss_chip.str();
    m_YunresKfit.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -1.e-2, 1.e-2, 100)));
    
    //
    
    hist_name = "KalmanFit/Residuals_on_X/plane_" + ss_chip.str();
    m_resKfit_X.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -1.e-2, 1.e-2, 100)));
    
    
    hist_name = "KalmanFit/Residuals_on_Y/plane_" + ss_chip.str();
    m_resKfit_Y.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -1.e-2, 1.e-2, 100)));
    
    hist_name = "KalmanFit/Residual_errors_on_X/plane_" + ss_chip.str();
    m_reserrKfit_X.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), 0., 5.e-3, 1000)));
    
    
    hist_name = "KalmanFit/Residual_errors_on_Y/plane_" + ss_chip.str();
    m_reserrKfit_Y.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), 0., 5.e-3, 1000)));
    
    
    hist_name = "KalmanFit/ResidualPull_on_X/plane_" + ss_chip.str();
    m_respullKfit_X.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -10., 10., 100)));
    
    
    hist_name = "KalmanFit/ResidualPull_on_Y/plane_" + ss_chip.str();
    m_respullKfit_Y.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -10., 10., 100)));
    

    hist_name = "KalmanFit/Differences_from_true_on_X/plane_" + ss_chip.str();
    m_trueKfit_X.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -1.e-2, 1.e-2, 100)));
    
    
    hist_name = "KalmanFit/Differences_from_true_on_Y/plane_" + ss_chip.str();
    m_trueKfit_Y.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -1.e-2, 1.e-2, 100)));
    
    hist_name = "KalmanFit/Differences_errors_from_true_on_X/plane_" + ss_chip.str();
    m_trueerrKfit_X.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), 0., 5.e-3, 1000)));
    
    
    hist_name = "KalmanFit/Differences_errors_from_true_on_Y/plane_" + ss_chip.str();
    m_trueerrKfit_Y.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), 0., 5.e-3, 1000)));
    
    
    hist_name = "KalmanFit/TrackPull_on_X/plane_" + ss_chip.str();
    m_trackpullKfit_X.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -10., 10., 100)));
    
    
    hist_name = "KalmanFit/TrackPull_on_Y/plane_" + ss_chip.str();
    m_trackpullKfit_Y.push_back(Gaudi::Utils::Aida2ROOT::aida2root(
        book1D(hist_name.c_str(), hist_name.c_str(), -10., 10., 100)));
  }
}

//=============================================================================
/// End
//=============================================================================
