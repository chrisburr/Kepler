#ifndef TBKALMANNODE_H
#define TBKALMANNODE_H 1

// Tb/TbEvent
#include "Event/TbState.h"
#include "Event/TbCluster.h"
#include "Event/TbNode.h"
#include "Event/ChiSquare.h"

namespace LHCb { class TbKalmanTrack; }

namespace LHCb {

/** @class TbKalmanNode TbKalmanNode.h Event/TbKalmanNode.h
  *
  *  A TbKalmanNode represents a node in the kalman fit.
  *
  *  @author: W. Hulsbergen and P. Tsopelas (based on LHCb::FitNode)
  */

class TbKalmanNode : virtual public TbNode {
 public:
  // Important note: 
  // For the Forward fit, smoothed means 'classical'. 
  // For the backward fit, it means 'bidirectional'.
  enum FilterStatus {
    Uninitialized,
    Initialized,
    Predicted,
    Filtered,
    Smoothed
  };
  enum Direction {
    Forward = 0,
    Backward = 1
  };
  enum Type {
    Reference,
    Measurement,
    Outlier
  };
  enum CachedBool {
    False = 0,
    True = 1,
    Unknown = 2
  };

  /// Default constructor
  TbKalmanNode();
  /// Constructor from a z position
  TbKalmanNode(TbKalmanTrack& parent, const double z);
  /// Constructor from a cluster
  TbKalmanNode(TbKalmanTrack& parent, const TbCluster& cluster);

  /// Destructor
  virtual ~TbKalmanNode();

  /// Set the seed.
  void setSeed(const LHCb::TbState& seedstate) {
    // just copy covariance (assuming it to be rather meaningless), but
    // transport the state
    LHCb::TbState astate(seedstate);
    const double dz = z() - seedstate.z();
    astate.parameters()(0) += dz * astate.parameters()(2);
    astate.parameters()(1) += dz * astate.parameters()(3);
    astate.setZ(z());
    m_predictedState[Forward] = astate;
    m_predictedState[Backward] = astate;
    resetFilterStatus(Forward);
    resetFilterStatus(Backward);
  }

  /// Retrieve chisq contribution in upstream filter
  const LHCb::ChiSquare& deltaChi2(const int direction) const {
    filteredState(direction);
    return m_deltaChi2[direction];
  }

  /// Return whether or not this node has active nodes upstream.
  bool hasInfoUpstream(const int direction) const;

  /// Unlink this node
  void unLink() {
    m_prevNode = m_nextNode = nullptr;
    m_parent = nullptr;
  }
  /// Link this node
  void link(TbKalmanNode* prevnode) {
    m_prevNode = prevnode;
    if (m_prevNode) m_prevNode->m_nextNode = this;
    m_nextNode = nullptr;
  }

  const TbKalmanNode* prevNode(const int direction) const {
    return direction == Forward ? m_prevNode : m_nextNode;
  }
  const TbKalmanNode* nextNode(const int direction) const {
    return direction == Forward ? m_nextNode : m_prevNode;
  }

  /// Retrieve the predicted state
  const LHCb::TbState& predictedState(const int dir) const {
    if (m_filterStatus[dir] < Predicted) unConst().computePredictedState(dir);
    return m_predictedState[dir];
  }
  /// Retrieve the filtered state
  const LHCb::TbState& filteredState(const int dir) const {
    if (m_filterStatus[dir] < Filtered) unConst().computeFilteredState(dir);
    return m_filteredState[dir];
  }
  /// Retrieve the bismoothed state
  virtual const LHCb::TbState& state() const {
    if (m_filterStatus[Backward] < Smoothed) unConst().computeBiSmoothedState();
    return m_state;
  }

  /// Set the noise term.
  void setNoise2(const double noise2) { m_Q = noise2; }

  /// Turn this node into an outlier.
  void deactivate(const bool deactivate);

  /// Get the index of this node. For debugging only.
  int index() const;

 private:
  /// Filter this hit.
  LHCb::ChiSquare filter(LHCb::TbState& state) const;

  void computePredictedState(const int direction);
  void computeFilteredState(const int direction);
  void computeBiSmoothedState();
  TbKalmanNode& unConst() const { return const_cast<TbKalmanNode&>(*this); }

  /// reset the cache for the previous function
  void resetHasInfoUpstream(const int direction);
  /// Reset the filter status
  void resetFilterStatus(const int direction, FilterStatus s = Initialized);

 private:
  /// Owner
  TbKalmanTrack* m_parent;

  /// Status of the node in the fit process
  FilterStatus m_filterStatus[2] = {Uninitialized, Uninitialized};
  /// Are there nodes with active measurement upstream of this node?
  CachedBool m_hasInfoUpstream[2] = {Unknown, Unknown}; 

  /// Predicted state of forward/backward filter
  LHCb::TbState m_predictedState[2];
  /// Filtered state of forward filter
  LHCb::TbState m_filteredState[2]; 
  /// Chisq contribution in forward filter
  LHCb::ChiSquare m_deltaChi2[2];  

  /// Previous node
  TbKalmanNode* m_prevNode = nullptr;
  /// Next node
  TbKalmanNode* m_nextNode = nullptr;

  /// Noise between this node and the next node
  double m_Q = 0.;  

};

}  // namespace LHCb

#endif
