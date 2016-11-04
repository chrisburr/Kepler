// Local
#include "TbKernel/ITbGeometrySvc.h"
#include "TbClusterFinder.h"

DECLARE_TOOL_FACTORY(TbClusterFinder)

//=============================================================================
// Constructor
//=============================================================================
TbClusterFinder::TbClusterFinder(const std::string& type,
                                 const std::string& name,
                                 const IInterface* parent)
    : GaudiTool(type, name, parent), m_searchAlgo(1) {

  declareInterface<ITbClusterFinder>(this);
}

//=============================================================================
// Set the search algorithm
//=============================================================================
void TbClusterFinder::setSearchAlgorithm(const std::string& searchAlgorithm) {

  if (searchAlgorithm == "seq") {
    m_searchAlgo = SearchMethod::Seq;
  } else if (searchAlgorithm == "adap_seq") {
    m_searchAlgo = SearchMethod::AdapSeq;
  }
}

//=============================================================================
// Initialisation
//=============================================================================
StatusCode TbClusterFinder::initialize() {

  // Initialise the base class.
  StatusCode sc = GaudiTool::initialize();
  if (sc.isFailure()) return sc;

  // Get the number of telescope planes from the geometry service.
  ITbGeometrySvc* geomSvc = svc<ITbGeometrySvc>("TbGeometrySvc", true);
  if (!geomSvc) {
    error() << "Cannot access geometry service" << endmsg;
    return StatusCode::FAILURE;
  }
  const unsigned int nPlanes = geomSvc->modules().size();
  m_first.resize(nPlanes);
  m_last.resize(nPlanes);
  m_end.resize(nPlanes);
  m_nClusters.resize(nPlanes);
  m_empty.resize(nPlanes);
  m_prev_ts.resize(nPlanes);
  m_prev.resize(nPlanes);
  return StatusCode::SUCCESS;
}

//=============================================================================
// Set the iterators and other properties for a given plane.
//=============================================================================
void TbClusterFinder::setClusters(LHCb::TbClusters* cs,
                                  const unsigned int& plane) {

  m_nClusters[plane] = cs->size();
  m_empty[plane] = cs->empty();
  if (!cs->empty()) {
    m_first[plane] = cs->begin();
    m_last[plane] = cs->end() - 1;
    m_end[plane] = cs->end();
    m_prev[plane] = cs->begin();
    m_prev_ts[plane] = (*cs->begin())->htime();
  }
}

//=============================================================================
// Get iterator to first cluster on a given plane inside a given time window
//=============================================================================
TbClusterFinder::Iterator TbClusterFinder::getIterator(
    const double& t, const unsigned int& plane) {

  if (m_searchAlgo == SearchMethod::Seq) {
    return seq_search(t, plane);
  } else if (m_searchAlgo == SearchMethod::AdapSeq) {
    return adap_seq_search(t, plane);
  }
  warning() << "Unknown search algorithm" << endmsg;
  return adap_seq_search(t, plane);
}

//=============================================================================
// Sequential search
//=============================================================================
TbClusterFinder::Iterator TbClusterFinder::seq_search(
    const double& t, const unsigned int& plane) {

  Iterator first = m_first[plane];
  if (t <= (*first)->htime()) return first;
  Iterator last = m_last[plane];
  for (Iterator ic = first; ic != last; ++ic) {
    if ((*ic)->htime() >= t) return ic;
  }
  return last;
}

//=============================================================================
// Adaptive sequential search
//=============================================================================
TbClusterFinder::Iterator TbClusterFinder::adap_seq_search(
    const double& t, const unsigned int& plane) {
  // Returns the index in the clusters list of the cluster which arrived
  // CURRENTLY RETURNS SOMETHING NEAR, BUT ISNT PERFECT.

  // Get (and update) the previous timestamp.
  const auto tPrev = m_prev_ts[plane];
  m_prev_ts[plane] = t;

  Iterator first = m_first[plane];
  Iterator last = m_last[plane];
  // Handle cases where requested time is outside the range of this plane.
  if (t < (*first)->htime()) {
    m_prev[plane] = first;
    return first;
  } else if (t >= (*last)->htime()) {
    m_prev[plane] = last;
    return last;
  }

  // Handle cases where requested time is inside the range of the clusters.
  if (t == tPrev) {
    // Case where t = previous asked t
    // (Covers the case where t is equal to the first cluster, since this is
    // default).
    return m_prev[plane];
  } else if (t < tPrev) {
    // Case where t < previous asked t. (Unwise usage):
    m_prev[plane] = first;
    return first;
  }

  // Case where t > previous asked t (Majority of cases):
  for (Iterator ic = m_prev[plane]; ic != last; ++ic) {
    if ((*ic)->htime() >= t) {
      m_prev[plane] = ic;
      return ic;
    }
  }
  m_prev[plane] = first;
  return first;
}
