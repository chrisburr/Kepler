#ifndef TB_NODE
#define TB_NODE 1

#include "Event/TbCluster.h"
#include "Event/TbState.h"
#include "Event/ChiSquare.h"

namespace LHCb {

class TbNode {
 public:
  /// Default constructor
  TbNode() {}

  /// Constructor from cluster
  TbNode(const TbCluster& cluster) : m_cluster(&cluster) {
    m_state.setZ(cluster.z());
    m_plane = cluster.plane();
    m_x = cluster.x();
    m_y = cluster.y();
    m_covx = cluster.xErr() * cluster.xErr();
    m_covy = cluster.yErr() * cluster.yErr();
  }

  /// Constructor from cluster and state
  TbNode(const TbCluster& cluster, const TbState& state) 
      : m_cluster(&cluster), m_state(state) {
    m_plane = cluster.plane();
    m_x = cluster.x();
    m_y = cluster.y();
    m_covx = cluster.xErr() * cluster.xErr();
    m_covy = cluster.yErr() * cluster.yErr();
    updateResidual(state);
  }

  /// Compute the residual with respect to a given state.
  void updateResidual(const TbState& state);

  double residualX() const { return m_residualX; }
  double residualCovX() const { return m_residualCovX; }
  double residualY() const { return m_residualY; }
  double residualCovY() const { return m_residualCovY; }
  double covX() const { return m_covx; }
  double covY() const { return m_covy; }
  /// Return whether this is an active node.
  bool active() const { return m_active; }
  /// Retrieve the cluster.
  const LHCb::TbCluster* cluster() const { return m_cluster; }
  /// Return the plane of the measurement.
  unsigned int plane() const { return m_plane; }
  /// Retrieve the track state.
  virtual const LHCb::TbState& state() const { return m_state; }
  /// Return the z coordinate.
  double z() const { return m_state.z(); }

  LHCb::ChiSquare chi2() const {
    return LHCb::ChiSquare(m_residualX * m_residualX / m_residualCovX +
                           m_residualY * m_residualY / m_residualCovY,
                           2);
  }

 protected:
  /// Flag whether this measurement is used in the fit.
  bool m_active = true;
  /// Cluster
  const TbCluster* m_cluster;
  /// Plane
  unsigned int m_plane;
  /// (Smoothed) state
  TbState m_state;

  /// Measured X coordinate
  double m_x;
  /// Measured Y coordinate
  double m_y;
  /// Error^2 in X
  double m_covx;
  /// Error^2 in Y
  double m_covy;
  /// Residual in X 
  double m_residualX;
  /// Residual in Y 
  double m_residualY;
  double m_residualCovX;
  double m_residualCovY;
};
}

#endif
