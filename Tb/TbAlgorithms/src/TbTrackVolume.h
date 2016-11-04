#ifndef TB_TRACKVOLUME_H
#define TB_TRACKVOLUME_H 1

#include <iostream>

// Tb/TbEvent
#include "Event/TbCluster.h"
#include "Event/TbTrack.h"

/** @class TbTrackVolume TbTrackVolume.h
 *
 *  Container for clusters given a set of spatial and time cuts. Used for
 *  tracking. Offers easy access to particular formations of tracks.
 *
 *  @author Dan Saunders
 */

class TbTrackVolume {
 public:
  enum VolShape {
    Cylinder = 1,
    Diabolo = 2,
    SqDiabolo = 3
  };
  /// Constructor
  TbTrackVolume(const std::string& volshape, const unsigned int& nPlanes,
                const double& r, const double& ry, const double& theta,
                const double& thetay, const unsigned int& minNClusters);
  /// Destructor
  ~TbTrackVolume() {}

  void reset(LHCb::TbCluster* c);
  void consider_cluster(LHCb::TbCluster* c);

  unsigned int nCombos();
  void get_track_combo(LHCb::TbTrack* t);
  void increment_combo_counters();

  // Setters and getters ______________________________________________________
  LHCb::TbCluster* seed() const { return m_seed_cluster; }

  /// Overload ostream operator <<
  friend std::ostream& operator<<(std::ostream& s, const TbTrackVolume& v) {
    s << v.m_theta << " " << v.m_r << " " << v.m_thetaY << " " << v.m_rY << " "
      << v.seed()->x() << " " << v.seed()->y() << " " << v.seed()->z() << " ";
    return s;
  }

  std::vector<std::vector<LHCb::TbCluster*> > m_clusters;

 private:
  double m_r;
  double m_rY;
  double m_r2;
  double m_theta;
  double m_thetaY;

  LHCb::TbCluster* m_seed_cluster;
  unsigned int m_3vol_shape;

  unsigned int m_nPlanes;
  unsigned int m_MinNClusters;
  std::vector<unsigned int> m_combo_counters;
  unsigned int m_nCombos;
  bool m_done;

  bool consider_cluster_cylinder(const LHCb::TbCluster* cl) const;
  bool consider_cluster_diabolo(const LHCb::TbCluster* cl) const;
  bool consider_cluster_sqDiabolo(const LHCb::TbCluster* cl) const;
  bool notAlreadyConsidered(const LHCb::TbCluster* cl) const;
};
#endif
