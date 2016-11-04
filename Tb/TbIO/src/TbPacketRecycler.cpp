#include "TbPacketRecycler.h"
#include "Event/TbTrack.h"

DECLARE_ALGORITHM_FACTORY(TbPacketRecycler)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TbPacketRecycler::TbPacketRecycler(const std::string& name,
                                   ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator) {

  // Declare algorithm properties - see header for description.
  declareProperty("HitLocation", m_hitLocation = LHCb::TbHitLocation::Default);
  declareProperty("TriggerLocation",
                  m_trgLocation = LHCb::TbTriggerLocation::Default);
  declareProperty("ClusterLocation",
                  m_clusLocation = LHCb::TbClusterLocation::Default);
}

//=============================================================================
// Initialisation
//=============================================================================
StatusCode TbPacketRecycler::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  IToolSvc* toolsvc = nullptr;
  sc = service("ToolSvc", toolsvc);
  if (!sc.isSuccess()) {
    return Error("Unable to get a handle to the tool service", sc);
  }
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    TbRawStream* foo = nullptr;
    sc = toolSvc()->retrieveTool("TbRawStream/" + std::to_string(i), foo);
    if (!sc.isSuccess()) {
      return Error("Unable to retrieve TbRawStream for plane " + 
                   std::to_string(i), sc);
    }
    m_streams.push_back(foo);
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbPacketRecycler::execute() {

  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    const std::string hitLocation = m_hitLocation + std::to_string(i);
    const std::string trgLocation = m_trgLocation + std::to_string(i);
    const std::string clusLocation = m_clusLocation + std::to_string(i);

    LHCb::TbHits* hits = getIfExists<LHCb::TbHits>(hitLocation);
    LHCb::TbTriggers* trgs = getIfExists<LHCb::TbTriggers>(trgLocation);
    LHCb::TbClusters* clusters = getIfExists<LHCb::TbClusters>(clusLocation);

    removeClusters(clusters);
    recycle(hits, m_streams[i]);
    recycle(trgs, m_streams[i]);
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Copy unused hits and triggers to the cache. 
//=============================================================================
template <typename TYPE>
void TbPacketRecycler::recycle(
    KeyedContainer<TYPE, Containers::HashMap>* container, TbRawStream* s) {
  if (!container) return;
  std::vector<TYPE*> tmp_cache;
  tmp_cache.clear();
  typename KeyedContainer<TYPE, Containers::HashMap>::reverse_iterator packet;
  for (packet = container->rbegin(); packet != container->rend(); ++packet) {
    // Break when outside the overlap time window.
    if (timingSvc()->beforeOverlap((*packet)->time())){
      break;
    }
    // If unused, add the hit/trigger to the temporary cache.
    if (!(*packet)->associated()) {
      tmp_cache.push_back(new TYPE(*packet));
      container->remove(*packet);
    }
  }
  std::reverse(tmp_cache.begin(), tmp_cache.end());
  s->cache<TYPE*>()->insert(s->cache<TYPE*>()->begin(), tmp_cache.begin(),
                            tmp_cache.end());
}

//=============================================================================
// Remove non-associated clusters from a container
//=============================================================================
void TbPacketRecycler::removeClusters(LHCb::TbClusters* clusters) {
  if (!clusters) return;
  LHCb::TbClusters::reverse_iterator it;
  for (it = clusters->rbegin(); it != clusters->rend(); ++it) {
    // Break when outside the overlap time window.
    if (timingSvc()->beforeOverlap((*it)->time())) break;
    // If unused, remove the cluster from the container.
    if (!(*it)->associated()) clusters->remove(*it); 
  }
}
