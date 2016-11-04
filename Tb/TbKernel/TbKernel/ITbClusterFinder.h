#pragma once

// Gaudi
#include "GaudiKernel/IAlgTool.h"

// Tb/TbEvent
#include "Event/TbCluster.h"

/** @class ITbClusterFinder ITbClusterFinder.h
 *
 *  Interface for cluster finder tool.
 *
 */

static const InterfaceID IID_ITbClusterFinder("ITbClusterFinder", 1, 0);

class ITbClusterFinder : virtual public IAlgTool {
 public:
  typedef LHCb::TbClusters::const_iterator Iterator;

  /// Return the interface ID
  static const InterfaceID& interfaceID() { return IID_ITbClusterFinder; }

  /// Find iterator to first cluster on a given plane above a given time.
  virtual Iterator getIterator(const double& t, const unsigned int& plane) = 0;
  /// (Re)set the stored iterators for a given plane.
  virtual void setClusters(LHCb::TbClusters* clusters,
                           const unsigned int& plane) = 0;
  /// Set the search algorithm to be used.
  virtual void setSearchAlgorithm(const std::string& alg) = 0;

  virtual Iterator first(const unsigned int& plane) const = 0;
  virtual Iterator end(const unsigned int& plane) const = 0;
  virtual bool empty(const unsigned int& plane) const = 0;
};
