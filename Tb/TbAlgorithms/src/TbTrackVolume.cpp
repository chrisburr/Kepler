#include "Event/TbTrack.h"

#include "TbTrackVolume.h"

//=============================================================================
// Constructor
//=============================================================================
TbTrackVolume::TbTrackVolume(const std::string& vol_shape,
                             const unsigned int& nPlanes, const double& r,
                             const double& rY, const double& theta,
                             const double& thetaY,
                             const unsigned int& minNClusters)
    : m_r(r),
      m_rY(rY),
      m_r2(r * r),
      m_theta(theta),
      m_thetaY(thetaY),
      m_seed_cluster(NULL),
      m_nPlanes(nPlanes),
      m_MinNClusters(minNClusters) {

  if (vol_shape == "cylinder") {
    m_3vol_shape = VolShape::Cylinder;
  } else if (vol_shape == "diabolo") {
    m_3vol_shape = VolShape::Diabolo;
  } else if (vol_shape == "sqDiabolo") {
    m_3vol_shape = VolShape::SqDiabolo;
  } else {
    // Use default shape.
    m_3vol_shape = VolShape::Cylinder;
  }
}

//=============================================================================
// Initialise with a given seed cluster.
//=============================================================================
void TbTrackVolume::reset(LHCb::TbCluster* c) {

  m_clusters.clear();
  m_clusters.resize(m_nPlanes, std::vector<LHCb::TbCluster*>());
  m_clusters[c->plane()].push_back(c);
  m_seed_cluster = c;
  m_done = false;
}

//=============================================================================
// Check if a given cluster is within the volume.
//=============================================================================
void TbTrackVolume::consider_cluster(LHCb::TbCluster* c) {

  bool inside = false;
  switch (m_3vol_shape) {
    case VolShape::Cylinder:
      inside = consider_cluster_cylinder(c);
      break;
    case VolShape::Diabolo:
      inside = consider_cluster_diabolo(c);
      break;
    case VolShape::SqDiabolo:
      inside = consider_cluster_sqDiabolo(c);
      break;
    default:
      break;
  }
  if (inside) {
    m_clusters[c->plane()].push_back(c);
  }
}

//=============================================================================
bool TbTrackVolume::consider_cluster_sqDiabolo(const LHCb::TbCluster* c_out)
    const {

  const double delx = c_out->x() - seed()->x();
  const double dely = c_out->y() - seed()->y();
  const double dz = fabs(c_out->z() - seed()->z());
  const double r_x = dz * m_theta + m_r;
  const double r_y = dz * m_thetaY + m_rY;
  if (delx < r_x && dely < r_y) return true;
  return false;
}

//=============================================================================
bool TbTrackVolume::consider_cluster_diabolo(const LHCb::TbCluster* c) const {

  const double delx = c->x() - seed()->x();
  const double dely = c->y() - seed()->y();
  const double r_z = m_r + fabs(c->z() - m_seed_cluster->z()) * m_theta;
  return (delx * delx + dely * dely < r_z * r_z);
}

//=============================================================================
bool TbTrackVolume::notAlreadyConsidered(const LHCb::TbCluster* c) const {
  bool result = true;
  for (unsigned int i = 0; i < m_clusters[c->plane()].size(); i++) {
    if (m_clusters[c->plane()][i]->key() == c->key()) result = false;
  }
  return result;
}

//=============================================================================
bool TbTrackVolume::consider_cluster_cylinder(const LHCb::TbCluster* c) const {
  const double delx = c->x() - seed()->x();
  const double dely = c->y() - seed()->y();
  if ((delx * delx + dely * dely) < m_r2 && notAlreadyConsidered(c)) {
    return true;
  }
  return false;
}

//=============================================================================
// Return the number of track combinations that can be made from this volume
//=============================================================================
unsigned int TbTrackVolume::nCombos() {

  if (m_done) return m_nCombos;
  // Remove empty planes.
  for (unsigned int i = 0; i < m_clusters.size(); i++) {
    if (m_clusters[i].size() == 0) {
      m_clusters.erase(m_clusters.begin() + i);
      i--;
    }
  }
  m_combo_counters.assign(m_clusters.size(), 0);

  if (m_clusters.size() < m_MinNClusters) {
    m_nCombos = 0;
  } else {
    m_nCombos = 1;
    for (unsigned int i = 0; i < m_clusters.size(); i++) {
      m_nCombos *= m_clusters[i].size();
    }
  }
  m_done = true;
  return m_nCombos;
}

//=============================================================================
// Make a track from the current combination of clusters
//=============================================================================
void TbTrackVolume::get_track_combo(LHCb::TbTrack* track) {

  for (unsigned int iplane = 0; iplane < m_clusters.size(); iplane++) {
    const int ic = m_combo_counters[iplane];
    track->addToClusters(m_clusters[iplane][ic]);
  }
}

//=============================================================================
// Get the next combination of clusters
//=============================================================================
void TbTrackVolume::increment_combo_counters() {
  const int n = m_combo_counters.size() - 1;
  m_combo_counters[n] += 1;

  // Adaptive clock.
  for (int i = n; i >= 1; i--) {
    if (m_combo_counters[i] >= m_clusters[i].size()) {
      m_combo_counters[i] = 0;
      m_combo_counters[i - 1] += 1;
    }
  }
}
