// Tb/TbEvent
#include "Event/TbTrigger.h"
#include "Event/TbHit.h"
#include "Event/TbCluster.h"
#include "Event/TbTrack.h"

// Local
#include "TbTupleWriter.h"

DECLARE_ALGORITHM_FACTORY(TbTupleWriter)

//=============================================================================
// Standard constructor
//=============================================================================
TbTupleWriter::TbTupleWriter(const std::string& name, ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator) {

  declareProperty("WriteTriggers", m_writeTriggers = false);
  declareProperty("WriteHits", m_writeHits = false);
  declareProperty("WriteClusters", m_writeClusters = true);
  declareProperty("WriteTracks", m_writeTracks = false);

  declareProperty("MaxClusterSize", m_maxclustersize = 200);
  declareProperty("WriteClusterHits", m_writeClusterHits = true);

  declareProperty("TriggerLocation",
                  m_triggerLocation = LHCb::TbTriggerLocation::Default);
  declareProperty("HitLocation", m_hitLocation = LHCb::TbHitLocation::Default);
  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);
  declareProperty("TrackLocation",
                  m_trackLocation = LHCb::TbTrackLocation::Default);

  /// Stripped nTuples for low rate external users, 
  /// only output tracks that have associated triggers 
  declareProperty("StrippedNTuple", m_strippedNTuple = false);
  declareProperty("MaxTriggers", m_maxTriggers = 20 );
  // Switch off output during finalize.
  setProperty("NTuplePrint", false);
}

//=============================================================================
// Destructor
//=============================================================================
TbTupleWriter::~TbTupleWriter() {}

//=============================================================================
// Initialisation
//=============================================================================
StatusCode TbTupleWriter::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;

  if (m_writeTriggers) bookTriggers();
  if (m_writeHits) bookHits();
  if (m_writeTracks) bookTracks();
  if (m_writeClusters) bookClusters();
  m_evtNo = 0;
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbTupleWriter::execute() {

  if (m_writeTriggers) fillTriggers();
  if (m_writeHits) fillHits();
  if (m_writeTracks) fillTracks();
  if (m_writeClusters) fillClusters();
  ++m_evtNo;
  return StatusCode::SUCCESS;
}

//=============================================================================
// Book trigger tuple
//=============================================================================
void TbTupleWriter::bookTriggers() {

  NTupleFilePtr file1(ntupleSvc(), "/NTUPLES/FILE1");
  NTuplePtr nt(ntupleSvc(), "/NTUPLES/FILE1/TbTupleWriter/Trigger");
  // Check if already booked.
  if (nt) return;
  nt = ntupleSvc()->book("/NTUPLES/FILE1/TbTupleWriter/Trigger",
                         CLID_ColumnWiseTuple, "nTuple of Triggers");
  nt->addItem("TgID", m_TgID);
  nt->addItem("TgTime", m_TgTime);
  nt->addItem("TgHTime", m_TgHTime);
  nt->addItem("TgEvt", m_TgEvt);
  nt->addItem("TgPlane", m_TgPlane);
  nt->addItem("TgCounter",m_TgCounter);
}

//=============================================================================
// Book hit tuple
//=============================================================================
void TbTupleWriter::bookHits() {

  NTupleFilePtr file1(ntupleSvc(), "/NTUPLES/FILE1");
  NTuplePtr nt(ntupleSvc(), "/NTUPLES/FILE1/TbTupleWriter/Hits");
  // Check if already booked.
  if (nt) return;
  nt = ntupleSvc()->book("/NTUPLES/FILE1/TbTupleWriter/Hits",
                         CLID_ColumnWiseTuple, "nTuple of Hits");
  nt->addItem("hID", m_hID);
  nt->addItem("hCol", m_hCol);
  nt->addItem("hRow", m_hRow);
  nt->addItem("hScol", m_hScol);
  nt->addItem("hTime", m_hTime);
  nt->addItem("hHTime", m_hHTime);
  nt->addItem("hToT", m_hToT);
  nt->addItem("hPlane", m_hPlane);
}

//=============================================================================
// Book cluster tuple
//=============================================================================
void TbTupleWriter::bookClusters() {

  NTupleFilePtr file1(ntupleSvc(), "/NTUPLES/FILE1");
  NTuplePtr nt(ntupleSvc(), "/NTUPLES/FILE1/TbTupleWriter/Clusters");
  // Check if already booked.
  if (nt) return;
  nt = ntupleSvc()->book("/NTUPLES/FILE1/TbTupleWriter/Clusters",
                         CLID_ColumnWiseTuple, "nTuple of Clusters");
  nt->addItem("clID", m_clID);
  nt->addItem("clGx", m_clGx);
  nt->addItem("clGy", m_clGy);
  nt->addItem("clGz", m_clGz);
  nt->addItem("clLx", m_clLx);
  nt->addItem("clLy", m_clLy);
  nt->addItem("clTime", m_clTime);
  nt->addItem("clHTime", m_clHTime);
  nt->addItem("clSize", m_clSize);
  nt->addItem("clCharge", m_clCharge);
  nt->addItem("clIsTracked", m_clTracked);
  nt->addItem("clPlane", m_clPlane);
  nt->addItem("clEvtNo", m_clEvtNo);
  nt->addItem("clNHits", m_clN, 0, (int)m_maxclustersize);
  nt->addIndexedItem("hRow", m_clN, m_clhRow);
  nt->addIndexedItem("hCol", m_clN, m_clhCol);
  nt->addIndexedItem("sCol", m_clN, m_clsCol);
  nt->addIndexedItem("hHTime", m_clN, m_clhHTime);
  nt->addIndexedItem("hToT", m_clN, m_clhToT);
}

//=============================================================================
// Book track tuple
//=============================================================================
void TbTupleWriter::bookTracks() {

  NTupleFilePtr file1(ntupleSvc(), "/NTUPLES/FILE1");
  NTuplePtr nt(ntupleSvc(), "/NTUPLES/FILE1/TbTupleWriter/Tracks");
  // Check if already booked.
  if (nt) return;
  nt = ntupleSvc()->book("/NTUPLES/FILE1/TbTupleWriter/Tracks",
                         CLID_ColumnWiseTuple, "nTuple of Tracks");
  nt->addItem("TkID", m_TkID);
  nt->addItem("TkTime", m_TkTime);
  nt->addItem("TkHTime", m_TkHTime);
  nt->addItem("TkNCl", m_TkNCl, 0, 10);
  nt->addItem("TkX", m_TkX0);
  nt->addItem("TkY", m_TkY0);
  nt->addItem("TkTx", m_TkTx);
  nt->addItem("TkTy", m_TkTy);
  nt->addItem("TkChi2PerNdof", m_TkChi2ndof);
  nt->addIndexedItem("TkClId", m_TkNCl, m_TkClId);
  nt->addItem("TkEvt",m_TkEvt);
  nt->addItem("TkNTg",m_TkNTg,0,(int)m_maxTriggers);
  nt->addIndexedItem("TkTgId",m_TkNTg,m_TkTgId);
  nt->addIndexedItem("TkTgPlane",m_TkNTg,m_TkTgPlane);
  nt->addIndexedItem("TkXResidual",m_TkNCl, m_TkXResidual); 
  nt->addIndexedItem("TkYResidual",m_TkNCl, m_TkYResidual); 

}

//=============================================================================
// Fill trigger tuple
//=============================================================================
void TbTupleWriter::fillTriggers() {

  const uint64_t evtOffset = (uint64_t)m_evtNo << 36;
  for (unsigned int i = 0 ; i < m_nPlanes; ++i) {
    const std::string location = m_triggerLocation + std::to_string(i);
    const LHCb::TbTriggers* triggers = getIfExists<LHCb::TbTriggers>(location);
    if (!triggers) continue;
    for (const LHCb::TbTrigger* trigger : *triggers) {
      // if (m_triggerChannel != 999 && m_triggerChannel != trigger->plane()) continue; 
      const uint64_t offset = ((uint64_t)trigger->plane() << 32) + evtOffset;
      m_TgID = trigger->index() + offset;
      m_TgTime = trigger->time();
      m_TgHTime = trigger->htime();
      m_TgPlane = i;
      m_TgCounter = trigger->counter();
      ntupleSvc()->writeRecord("/NTUPLES/FILE1/TbTupleWriter/Trigger");
      m_TgEvt = m_evtNo;
    }
  }
}

//=============================================================================
// Fill hit tuple
//=============================================================================
void TbTupleWriter::fillHits() {

  const uint64_t evtOffset = (uint64_t)m_evtNo << 36;
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    const std::string location = m_hitLocation + std::to_string(i);
    const LHCb::TbHits* hits = getIfExists<LHCb::TbHits>(location);
    if (!hits) continue;
    const uint64_t offset = ((uint64_t)i << 32) + evtOffset;
    for (const LHCb::TbHit* hit : *hits) {
      m_hID = hit->index() + offset;
      m_hCol = hit->col();
      m_hRow = hit->row();
      m_hScol = hit->scol();
      m_hTime = hit->time();
      m_hHTime = hit->htime();
      m_hToT = hit->ToT();
      m_hPlane = i;
      ntupleSvc()->writeRecord("/NTUPLES/FILE1/TbTupleWriter/Hits");
    }
  }
}

//=============================================================================
// Fill cluster tuple
//=============================================================================
void TbTupleWriter::fillClusters() {

  const uint64_t evtOffset = (uint64_t)m_evtNo << 36;
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    const std::string location = m_clusterLocation + std::to_string(i);
    const LHCb::TbClusters* clusters = getIfExists<LHCb::TbClusters>(location);
    if (!clusters) continue;
    const uint64_t offset = ((uint64_t)i << 32) + evtOffset;
    for (const LHCb::TbCluster* cluster : *clusters) {
      const unsigned long long clid = cluster->index() + offset;
      m_clID = clid;
      m_clGx = cluster->x();
      m_clGy = cluster->y();
      m_clGz = cluster->z();
      m_clLx = cluster->xloc();
      m_clLy = cluster->yloc();
      m_clTime = cluster->time();
      m_clHTime = cluster->htime();
      m_clSize = cluster->hits().size();
      m_clCharge = cluster->charge();
      m_clTracked = cluster->associated();
      m_clPlane = cluster->plane();
      m_clEvtNo = m_evtNo;
      m_clN = m_clSize > m_maxclustersize ? m_maxclustersize : m_clSize;
      for (unsigned int j = 0; j < m_clN; ++j) {
        const auto& h = cluster->hits()[j];
        m_clhRow[j] = h->row();
        m_clhCol[j] = h->col();
        m_clsCol[j] = h->scol();
        m_clhToT[j] = h->ToT();
        m_clhHTime[j] = h->htime();
      }
      ntupleSvc()->writeRecord("/NTUPLES/FILE1/TbTupleWriter/Clusters");
    }
  }
}

//=============================================================================
// Fill track tuple
//=============================================================================
void TbTupleWriter::fillTracks() {

  const LHCb::TbTracks* tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!tracks) return;
  const uint64_t evtOffset = (uint64_t)m_evtNo << 36;
  for (const LHCb::TbTrack* track : *tracks) {
    if (m_strippedNTuple && track->triggers().empty()) continue;
    m_TkX0 = track->firstState().x();
    m_TkY0 = track->firstState().y();
    m_TkTx = track->firstState().tx();
    m_TkTy = track->firstState().ty();
    m_TkHTime = track->htime();
    m_TkTime = track->time();
    m_TkID = track->index() + evtOffset;
    m_TkNCl = track->clusters().size();
    m_TkChi2ndof = track->chi2PerNdof();
    m_TkEvt = m_evtNo;
    unsigned int i = 0;
    const SmartRefVector<LHCb::TbCluster>& clusters = track->clusters();
    for (auto it = clusters.cbegin(), end = clusters.cend(); it != end; ++it) {
      const uint64_t offset = ((uint64_t)(*it)->plane() << 32) + evtOffset;
      const unsigned long long clid = (*it)->index() + offset;
      m_TkClId[i] = clid;
      auto intercept = geomSvc()->intercept( track, (*it)->plane() );
      m_TkXResidual[(*it)->plane()] = (*it)->x() - intercept.x() ;
      m_TkYResidual[(*it)->plane()] = (*it)->y() - intercept.y() ;
      ++i;
    }
    const SmartRefVector<LHCb::TbCluster>& associatedClusters = track->associatedClusters();
    for( auto it = associatedClusters.cbegin(), end = associatedClusters.cend(); it != end; ++it ){
      const uint64_t offset = ((uint64_t)(*it)->plane() << 32) + evtOffset;
      const unsigned long long clid = (*it)->index() + offset;
      m_TkClId[i] = clid;
      auto intercept = geomSvc()->intercept( track, (*it)->plane() );
      m_TkXResidual[(*it)->plane()] = (*it)->x() - intercept.x() ;
      m_TkYResidual[(*it)->plane()] = (*it)->y() - intercept.y() ;
      ++i;
    }
    m_TkNTg = track->triggers().size();
    i = 0;
    const SmartRefVector<LHCb::TbTrigger>& triggers = track->triggers();
    for (auto it = triggers.cbegin(), end = triggers.cend(); it != end; ++it) {
      const uint64_t offset = ((uint64_t)(*it)->plane() << 32) + evtOffset;
      const unsigned long long clid = (*it)->index() + offset;
      if (i == 20) {
        warning() << "More than 20 triggers associated to track: skipping" 
          << endmsg;
        break;
      }
      m_TkTgId[i] = clid;
      m_TkTgPlane[i] = (*it)->plane();
      ++i;
    }
    ntupleSvc()->writeRecord("/NTUPLES/FILE1/TbTupleWriter/Tracks");
  }
}
