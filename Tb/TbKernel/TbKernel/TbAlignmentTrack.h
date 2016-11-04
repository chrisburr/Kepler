#ifndef TBKERNEL_TBALIGNMENTTRACK_H
#define TBKERNEL_TBALIGNMENTTRACK_H 1

#include "Event/TbTrack.h"
#include "Event/TbCluster.h"

/** @class TbAlignmentTrack TbAlignmentTrack.h TbKernel/TbAlignmentTrack.h
 *
 *
 *  @author Angelo Di Canto
 *  @date   2014-05-09
 */

class TbAlignmentTrack {
 public:
  /// Standard constructor
  TbAlignmentTrack(LHCb::TbTrack *t) : 
      m_track(nullptr), m_xref(0.), m_yref(0.) {
    // Clone track without the clusters.
    m_track = t->clone();
    // Clone each cluster on the track.
    SmartRefVector<LHCb::TbCluster> clusters = t->clusters();
    for (auto it = clusters.begin(), end = clusters.end(); it != end; ++it) {
      LHCb::TbCluster *c = (*it)->clone();
      m_track->addToClusters(c);
      m_clusters.push_back(c);
    }
  }

  /// Destructor
  virtual ~TbAlignmentTrack() {
    if (m_track) delete m_track;
    // Delete the associated clusters.
    for (auto it = m_clusters.begin(); it != m_clusters.end(); ++it) {
      if (*it) delete *it;
    }
    m_clusters.clear();
  }

  /// Setters
  void setXOnReferencePlane(const double x) { m_xref = x; }
  void setYOnReferencePlane(const double y) { m_yref = y; }
  void addToClusters(LHCb::TbCluster *c) { m_clusters.push_back(c); }

  /// Accessors
  LHCb::TbTrack *track() const { return m_track; }
  double xOnReferencePlane() const { return m_xref; }
  double yOnReferencePlane() const { return m_yref; }
  const std::vector<LHCb::TbCluster *> &clusters() { return m_clusters; }

 protected:
  LHCb::TbTrack *m_track;
  double m_xref;
  double m_yref;
  std::vector<LHCb::TbCluster *> m_clusters;
};

#endif  // TBKERNEL_TBALIGNMENTTRACK_H
