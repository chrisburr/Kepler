#include "Event/TbTrack.h"

using namespace LHCb;

//=============================================================================
// Track copy constructor 
//=============================================================================
TbTrack::TbTrack(const LHCb::TbTrack& rhs) :
  KeyedObject<int>(),
  m_time(rhs.m_time),
  m_htime(rhs.m_htime),
  m_firstState(rhs.m_firstState),
  m_chi2PerNdof(rhs.m_chi2PerNdof),
  m_ndof(rhs.m_ndof),
  m_clusters(rhs.m_clusters),
  m_triggers(rhs.m_triggers),
  m_associatedClusters(rhs.m_associatedClusters) {

  // Copy the nodes.
  for (auto it = rhs.m_nodes.begin(), end = rhs.m_nodes.end(); it != end; ++it) {
    addToNodes(*it);
  }
}


//=============================================================================
// Track clone method
//=============================================================================
TbTrack* TbTrack::clone() {
  TbTrack* track = new TbTrack();
  track->setTime(time());
  track->setHtime(htime());
  track->setFirstState(firstState());
  track->setChi2PerNdof(chi2PerNdof());
  track->setNdof(ndof());

  // Copy the nodes.
  for (auto it = m_nodes.begin(), end = m_nodes.end(); it != end; ++it) {
    track->addToNodes(*it);
  }
  return track;
}

//=============================================================================
// Add a node
//=============================================================================
void TbTrack::addToNodes(const TbNode& node) {
  m_nodes.push_back(node);
}

//=============================================================================
// Clear the nodes
//=============================================================================
void TbTrack::clearNodes() {
  m_nodes.clear() ;
}

