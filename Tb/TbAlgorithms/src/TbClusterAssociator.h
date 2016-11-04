#pragma once

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"

/** @class TbClusterAssociator TbClusterAssociator.h
 *
 */

class TbClusterAssociator : public TbAlgorithm {
 public:
  /// Constructor
  TbClusterAssociator(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbClusterAssociator() {}

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  std::vector<unsigned int> m_duts;
  std::string m_trackLocation;
  std::string m_clusterLocation;

  /// Flag to use individual pixel hits or cluster coordinate for association.
  bool m_useHits;
  /// Flag to associate clusters to more than one track or not.
  bool m_reuseClusters;
  /// Time window
  double m_twindow;
  /// Spatial window
  double m_xwindow;
  /// Chi2 cut
  double m_maxChi2;

  /// Check if a cluster has hits within the tolerance window.
  bool match(const LHCb::TbCluster* cluster, const double x,
             const double y) const;
};
