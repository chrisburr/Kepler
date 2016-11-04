#ifndef TB_VERTEXTRACKING_H
#define TB_VERTEXTRACKING_H 1

// AIDA
#include "AIDA/IHistogram2D.h"

// Tb/TbKernel
#include "TbKernel/ITbTrackFit.h"
#include "TbKernel/ITbClusterFinder.h"
#include "TbKernel/TbAlgorithm.h"

// Tb/TbEvent
#include "Event/TbCluster.h"
#include "Event/TbTrack.h"
#include "Event/TbVertex.h"

/** @class TbVertexTracking TbVertexTracking.h
 *
 *  Algorithm for tracks with vertices reconstruction
 *
 * @author Dan Saunders
 */

class TbVertexTracking : public TbAlgorithm {
 public:
  TbVertexTracking(const std::string &name, ISvcLocator *pSvcLocator);
  virtual ~TbVertexTracking();
  virtual StatusCode initialize();
  virtual StatusCode execute();

 private:
  LHCb::TbTracks *m_tracks;
  LHCb::TbVertices *m_vertices;

  /// Name of the track fit tool
  std::string m_trackFitTool;
  /// Track fit tool
  ITbTrackFit *m_trackFit;
  ITbClusterFinder *m_clusterFinder;
  std::string m_clusterLocation;
  std::string m_trackLocation;
  std::vector<std::vector<bool> > m_endCluster;
  std::vector<std::vector<bool> > m_vertexedCluster;
  std::vector<std::vector<bool> > m_volumed;

  double m_twindow;
  unsigned int m_MinNClusters;
  unsigned int m_MinNClustersRepeat;
  double m_ChiSqRedCut;
  std::string m_ClusterFinderSearchAlgorithm;
  std::vector<unsigned int> m_PlaneSearchOrder;
  unsigned int m_clusterSizeCut;

  bool m_viewerOutput;
  unsigned int m_viewerEvent;
  unsigned int m_event;
  bool m_combatRun;
  double m_radialCut;
  bool m_doVertexting;
  bool m_doRepeat;
  double m_angleCut;
  double m_currentAngleCut;
  double m_vertexDelR;
  double m_vertexDelT;

  // Historgrams ______________________________________________________________
  AIDA::IHistogram2D *initialStateVsFitStateTx;
  AIDA::IHistogram2D *initialStateVsFitStateTy;

  // Methods __________________________________________________________________
  bool fillTrack(LHCb::TbTrack *, LHCb::TbCluster *, LHCb::TbCluster *);
  void evalHoughState(LHCb::TbCluster *, LHCb::TbCluster *, LHCb::TbState *);
  void outputViewerData();
  void performVertexTracking();
  void timeOrderTracks();
  void formTrack(LHCb::TbCluster *);
  void outputPatternRecog(double, double, double, double);
  void outputHoughState(LHCb::TbCluster *, LHCb::TbCluster *);
  void collectIntoVertices();
  void outputVertices();
};
#endif
