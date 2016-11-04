#ifndef TB_TRACKING_H
#define TB_TRACKING_H 1

// Root
#include "TH1.h"
#include "TH2.h"

// Tb/TbKernel
#include "TbKernel/ITbTrackFit.h"
#include "TbKernel/TbAlgorithm.h"

// Tb/TbEvent
#include "Event/TbCluster.h"
#include "Event/TbTrack.h"
// Kalman classes in TbEvent
#include "Event/TbKalmanTrack.h"
#include "Event/TbKalmanNode.h"
#include "Event/TbKalmanPixelMeasurement.h"

// Local
#include "TbClusterFinder.h"
#include "TbTrackVolume.h"

/** @class TbTracking TbTracking.h
 *
 *  Algorithm for track reconstruction in Timepix3 telescope
 *
 * @author Dan Saunders
 */

class TbTracking : public TbAlgorithm {
 public:
  /// Constructor
  TbTracking(const std::string &name, ISvcLocator *pSvcLocator);
  /// Destructor
  virtual ~TbTracking();

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  /// Track container (to be filled).
  LHCb::TbTracks *m_tracks;

  /// Track fit tool
  ITbTrackFit *m_trackFit;
  /// Tool to find particular clusters.
  TbClusterFinder *m_clusterFinder;

  // Tracking specific options.
  /// TES location of cluster containers.
  std::string m_clusterLocation;
  /// Flag to fill (or not) monitoring histograms.
  bool m_monitoring;
  /// Time width (in ns) of TbTrackVolumes.
  double m_twindow;
  /// Minimum number of clusters to form a track.
  unsigned int m_MinNClusters;
  /// Spatial shapes of TbTrackVolumes {cylinder, diabolo}.
  std::string m_search_3vol;
  /// Spatial shape parameter.
  double m_vol_radius;
  double m_vol_radiusY;
  /// Spatial shape parameter.
  double m_vol_theta;
  double m_vol_thetaY;
  /// Chi2 cut.
  double m_ChiSqRedCut;
  /// Upper cut on the number of cominations to try in a TbTrackVolume.
  /// Useful for speed, and rarely used; set O(100).
  int m_nComboCut;
  /// Search algorithm used to fill TbTrackVolumes - {"seq", "adap_seq"}.
  /// "adap_seq" recommeneded.
  std::string m_ClusterFinderSearchAlgorithm;

  /// For certain volumes, it's advantageous to use seeds from the center of
  /// the telescope, so the order of the search can be specified here.
  std::vector<unsigned int> m_PlaneSearchOrder;

  void performTracking();
  void fillATrackVolume(TbTrackVolume *);
  void evaluateTrackVolume(TbTrackVolume *);
  void timeOrderTracks();
  void poorMansEvaluation(TbTrackVolume *);
  
  // Errors for Kalman fit
  double  m_hiterror2;
  double  m_scat2;
  
  // Histo functions
  void setup_hists();
  void fill_khists(std::vector<LHCb::TbKalmanTrack*>&);
  
  float lowR, highR, binsR, lowS, highS;
  
  
  // Kalman filter histos
  //---------------------------------------------------
  
  // Track parameters
  TH1D* m_Kfit_chi2;
  TH1D* m_Kfit_prob;
  
  // unbiased residuals
  std::vector<TH1D*> m_XunresKfit;
  std::vector<TH1D*> m_YunresKfit;
  // biased residuals
  std::vector<TH1D*> m_XresKfit;
  std::vector<TH1D*> m_YresKfit;
  // biased residuals on X,Y
  std::vector<TH2D*> m_XresKfitOnX;
  std::vector<TH2D*> m_XresKfitOnY;
  
  std::vector<TH2D*> m_YresKfitOnY;
  std::vector<TH2D*> m_YresKfitOnX;
  
  // biased residuals on X,Y slopes
  std::vector<TH2D*> m_XresKfitOnTX;
  std::vector<TH2D*> m_XresKfitOnTY;
  
  std::vector<TH2D*> m_YresKfitOnTY;
  std::vector<TH2D*> m_YresKfitOnTX;
  
  // biased residuals errors
  std::vector<TH1D*> m_XreserrKfit;
  std::vector<TH1D*> m_YreserrKfit;
  
  // residual pulls
  std::vector<TH1D*> m_XrespullKfit;
  std::vector<TH1D*> m_YrespullKfit;
  
  // quality biased residuals
  std::vector<TH1D*> m_qXresKfit;
  std::vector<TH1D*> m_qYresKfit;
  
  // quality residual pulls
  std::vector<TH1D*> m_qXrespullKfit;
  std::vector<TH1D*> m_qYrespullKfit;
  
  
  // TbKalmanTrack container
  std::vector<LHCb::TbKalmanTrack*> ktracks_vec;
  //---------------------------------------------------
  
};
#endif
