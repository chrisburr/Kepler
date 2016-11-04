#ifndef TB_TRACKING_H
#define TB_TRACKING_H 1

// Tb/TbKernel
#include "TbKernel/ITbTrackFit.h"
#include "TbKernel/ITbClusterFinder.h"
#include "TbKernel/TbAlgorithm.h"

// Tb/TbEvent
#include "Event/TbCluster.h"
#include "Event/TbTrack.h"

// Local
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

  /// Name of the track fit tool
  std::string m_trackFitTool;
  /// Track fit tool
  ITbTrackFit *m_trackFit;
  /// Tool to find particular clusters.
  ITbClusterFinder *m_clusterFinder;
  /// Tool for evaluating seed tracks.
  TbTrackVolume *m_trackVolume;

  /// TES location prefix of cluster containers.
  std::string m_clusterLocation;
  /// TES location prefix of track containers.
  std::string m_trackLocation;

  /// Flag to fill (or not) monitoring histograms.
  bool m_monitoring;
  /// Time width (in ns) of search window around seed cluster.
  double m_twindow;
  /// Minimum number of clusters to form a track.
  unsigned int m_MinNClusters;
  /// Spatial shapes of TbTrackVolumes {cylinder, diabolo}.
  std::string m_search_3vol;
  /// Spatial shape parameter.
  double m_vol_radius;
  /// Spatial shape parameter.
  double m_vol_theta;
  /// Chi2 cut.
  double m_ChiSqRedCut;
  /// Upper cut on the number of combinations to try in a TbTrackVolume.
  /// Useful for speed, and rarely used; set O(100).
  unsigned int m_nComboCut;
  /// Search algorithm used to fill TbTrackVolumes - {"seq", "adap_seq"}.
  /// "adap_seq" recommended.
  std::string m_ClusterFinderSearchAlgorithm;

  /// For certain volumes, it's advantageous to use seeds from the center of
  /// the telescope, so the order of the search can be specified here.
  std::vector<unsigned int> m_PlaneSearchOrder;
  /// Max. size of clusters on a track
  unsigned int m_clusterSizeCut;

  std::vector<std::vector<bool> > m_volumed;
  // Viewer options.
  bool m_viewerOutput;
  unsigned int m_viewerEvent;
  unsigned int m_event;
  bool m_combatRun;

  void outputViewerData();
  void performTracking();
  void fillTrackVolume(LHCb::TbCluster *seed, const unsigned int &planeLow,
                       const unsigned int &planeUp);
  void evaluateTrackVolume(TbTrackVolume *vol);
  void timeOrderTracks();
  void fillTrackVolPlots(TbTrackVolume *vol);
};
#endif
