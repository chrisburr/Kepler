#pragma once

// Gaudi
#include "GaudiAlg/GaudiTool.h"

// Local
#include "TbKernel/ITbClusterFinder.h"

/** @class TbClusterFinder TbClusterFinder.h
 *
 *  This small class is used to find a particular cluster (on a particular
 *  chip) nearest a particular time. This is done using a variety of funky
 *  algorithms and conditions - specifically, using the condition that during
 *  this object's lifetime, whenever asked to retrieve clusters for particular
 *  time, these times are always increasing. (ALWAYS). Otherwise, expect O(N)
 *  performance.
 *
 *  @author Dan Saunders
 */

class TbClusterFinder : public GaudiTool, virtual public ITbClusterFinder {
 public:
  typedef LHCb::TbClusters::const_iterator Iterator;
  enum SearchMethod {
    Seq = 1,
    AdapSeq = 2
  };

  /// Standard constructor
  TbClusterFinder(const std::string& type, const std::string& name,
                  const IInterface* parent);
  /// Destructor
  virtual ~TbClusterFinder() {}

  virtual StatusCode initialize();

  /// Find iterator to first cluster on a given plane above a given time.
  virtual Iterator getIterator(const double& t, const unsigned int& plane);
  /// (Re)set the stored iterators for a given plane.
  virtual void setClusters(LHCb::TbClusters* clusters,
                           const unsigned int& plane);
  /// Set the search algorithm to be used.
  virtual void setSearchAlgorithm(const std::string& alg);

  virtual Iterator first(const unsigned int& plane) const {
    return m_first[plane];
  }
  virtual Iterator end(const unsigned int& plane) const { return m_end[plane]; }
  virtual bool empty(const unsigned int& plane) const { return m_empty[plane]; }

 private:
  unsigned int m_searchAlgo;

  std::vector<Iterator> m_first;
  std::vector<Iterator> m_last;
  std::vector<Iterator> m_end;
  std::vector<unsigned int> m_nClusters;
  std::vector<bool> m_empty;
  std::vector<double> m_prev_ts;
  std::vector<Iterator> m_prev;

  /// Sequential search method
  Iterator seq_search(const double& t, const unsigned int& plane);
  /// Adaptive sequential search method
  Iterator adap_seq_search(const double& t, const unsigned int& plane);
};
