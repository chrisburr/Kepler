#ifndef TB_SIMPLETRACKING_H
#define TB_SIMPLETRACKING_H 1

// Tb/TbEvent
#include "Event/TbCluster.h"
#include "Event/TbTrack.h"

// Tb/TbKernel
#include "TbKernel/ITbTrackFit.h"
#include "TbKernel/TbAlgorithm.h"

/** @class TbSimpleTracking TbSimpleTracking.h
 *
 */

class TbSimpleTracking : public TbAlgorithm {
 public:
  /// Constructor
  TbSimpleTracking(const std::string &name, ISvcLocator *pSvcLocator);
  /// Destructor
  virtual ~TbSimpleTracking() {}

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  /// TES location prefix of clusters
  std::string m_clusterLocation;
  /// TES location of tracks
  std::string m_trackLocation;

  /// Tolerance window in time for adding clusters to a track and for occupancy
  /// cut
  double m_timeWindow;
  /// Minimum number of clusters required to form a track
  unsigned int m_nMinPlanes;
  /// Tolerance window in x and y for adding clusters to a track
  double m_maxDist;
  /// Angular cut (in radians) for adding clusters to a track
  double m_maxAngle;
  bool m_recheckTrack;
  bool m_removeOutliers;
  double m_chargeCutLow;
  unsigned int m_maxClusterSize;
  unsigned int m_maxClusterWidth;
  double m_maxChi2;
  unsigned int m_maxOccupancy;
  std::vector<double> m_htimesHighOccupancy;
  bool m_doOccupancyCut;
  bool m_monitoring;

  /// List of clusters
  std::vector<LHCb::TbClusters *> m_clusters;

  /// Name of the track fit tool
  std::string m_trackFitTool;
  /// Track fit tool
  ITbTrackFit *m_trackFit = nullptr;

  void findHighOccupancies();
  bool lowClusterOccupancy(const double t) const;
  void appendTrackingEfficiencies();
  void recheckTrack(LHCb::TbTrack *track);
  /// Extrapolate and add clusters to a given seed track.
  bool extendTrack(LHCb::TbTrack *track, const bool fwd);
  /// Look for a matching cluster on a given plane.
  const LHCb::TbCluster *bestCluster(const unsigned int plane,
                                     const double xPred, const double yPred,
                                     const double tPred, const double tol);
  /// Functor for lower bound search.
  class lowerBound {
   public:
    bool operator()(const LHCb::TbCluster *lhs, const double t) const {
      return lhs->htime() < t;
    }
  };
};
#endif
