// Gaudi
#include "GaudiKernel/IToolSvc.h"

// Tb/TbEvent
#include "Event/TbHit.h"
#include "Event/TbTrigger.h"
#include "Event/TbCluster.h"

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"
#include "TbKernel/TbConstants.h"

// Local
#include "TbRawStream.h"

/** @class TbPacketRecycler TbPacketRecycler.h
 *
 *  Algorithm to move unprocessed data in the overlap into the next event
 *  designed to prevent clusters / tracks being cut between events
 *
 *  @author Tim Evans (timothy.david.evans@cern.ch)
 *  @date   2014-04-01
 */

class TbPacketRecycler : public TbAlgorithm {
 public:
  /// Constructor
  TbPacketRecycler(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbPacketRecycler() {}

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  std::vector<TbRawStream*> m_streams;
  std::string m_hitLocation;
  std::string m_trgLocation;
  std::string m_clusLocation;
  template <typename TYPE>
  void recycle(KeyedContainer<TYPE, Containers::HashMap>* container,
               TbRawStream* s);
  void removeClusters(LHCb::TbClusters* clusters);
  template <typename TYPE>
  void addToCache(std::vector<TYPE*> cache, TYPE* packet) {
    cache.push_back(packet);
  }
};

template <>
void TbPacketRecycler::addToCache(std::vector<LHCb::TbHit*> cache,
                                  LHCb::TbHit* packet) {
  cache.push_back(packet);
}
