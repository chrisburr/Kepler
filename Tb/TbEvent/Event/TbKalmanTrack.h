#ifndef TBKALMANTRACK_H
#define TBKALMANTRACK_H

#include "Event/TbTrack.h"

namespace LHCb {
class TbKalmanNode;

class TbKalmanTrack : public TbTrack {
 public:
  /// Constructor
  TbKalmanTrack(const LHCb::TbTrack& track, const std::vector<double>& noise2);
  /// Destructor
  ~TbKalmanTrack();

  /// Get the nodes.
  const std::vector<TbKalmanNode*>& knodes() const { return m_knodes; }

  /// Do the fit.
  void fit();

  // Dump some debug info
  void print() const;

  // Add a node
  void addNode(TbKalmanNode* node);

  // Add a 'reference' node (without a measurement, just to have the state)
  void addReferenceNode(const double z);

  // Deactivate a measurement on the track
  void deactivateCluster(const TbCluster& clus);

 private:
  std::vector<TbKalmanNode*> m_knodes;
};
}

#endif
