#include <algorithm>

// Gaudi
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiAlg/ISequencerTimerTool.h"

// Tb/TbKernel
#include "TbKernel/TbFunctors.h"
#include "TbKernel/TbConstants.h"

// Local
#include "TbEventBuilder.h"
#include "TbRawFile.h"

#include <chrono>
#include <ctime>

DECLARE_ALGORITHM_FACTORY(TbEventBuilder)

const int64_t TbEventBuilder::m_maxTimeDifference = std::pow(2, 40);

//=============================================================================
// Standard constructor
//=============================================================================
TbEventBuilder::TbEventBuilder(const std::string& name,
                               ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator), m_streams(), m_triggers(), m_hits() {
  declareProperty("HitLocation", m_hitLocation = LHCb::TbHitLocation::Default);
  declareProperty("TriggerLocation",
                  m_triggerLocation = LHCb::TbTriggerLocation::Default);

  // Length of event in global time units.
  declareProperty("EventLength", m_tick = 10 * Tb::SpidrTime);
  // Length to look into 'future' events to correct
  // for time misordering around event boundaries
  declareProperty("CacheLength", m_cachelength = 20 * Tb::SpidrTime);

  // The time by which events 'overlap' to prevent tracks / clusters
  // being cut between different events. Implementation is rather
  // convoluted, relies on track identification and global event defintion
  // via the TbTimingSvc
  declareProperty("OverlapTime", m_overlapTime = 0 * Tb::ToA);

  // Min. number of planes with at least one hit required to make an event
  declareProperty("MinPlanesWithHits", m_nMinPlanesWithHits = 1);
  // Print frequency
  declareProperty("PrintFreq", m_printFreq = 100);
  // Size of the header, if not set is read in
  declareProperty("HeaderSize", m_headerSize = 0);
  // Flag to print out the header information
  declareProperty("PrintHeader", m_printHeader = false);
  // Flag to switch monitoring print-out and histograms.
  declareProperty("Monitoring", m_monitoring = false);
  // Time to start data processing, in s
  declareProperty("StartTime", m_startTime = 0);
  // Time to end data processing, in ms
  declareProperty("EndTime", m_endTime = 0);
  // Maximum number of packets that can be lost before
  // considered a critical failure
  declareProperty("MaxLostPackets", m_maxLostPackets = 1000);
  // Maximum number of timing packets that not read
  // before considered a critical failure
  declareProperty("MaxLostTimers", m_maxLostTimers = 10);
  // Force the cache to update every cycle, as opposed to
  // waiting until the first packet is in view
  declareProperty("ForceCaching", m_forceCaching = false);
  declareProperty("IgnoreGlobalClock", m_ignoreGlobalClock = false);
}

//=============================================================================
// Destructor
//=============================================================================
TbEventBuilder::~TbEventBuilder() {}

//=============================================================================
// Initialisation
//=============================================================================
StatusCode TbEventBuilder::initialize() {
  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;

  // Prepare the hit containers.
  m_hits.resize(m_nPlanes, nullptr);
  m_triggers.resize(m_nPlanes, nullptr);
  // Setup the raw stream tools.
  for (unsigned int i = 0; i < m_nDevices; ++i) {
    const std::string toolname = "TbRawStream/TbRawStream" + std::to_string(i);
    m_streams.push_back(tool<TbRawStream>(toolname));
    if (!m_streams[i]) return Error("Cannot initialise " + toolname);
    m_streams[i]->setDevice(i);
  }
  // Setup the header decoder tool.
  m_headerDecoder = tool<TbHeaderDecoder>("TbHeaderDecoder");
  if (!m_headerDecoder) return Error("Cannot initialise header decoder.");
  m_headerDecoder->print(m_printHeader);
  // Propagate the overlap time to the timing service.
  timingSvc()->setOverlap(m_overlapTime);
  // Get the list of input data
  auto files = dataSvc()->getInputFiles();
  for (const auto& filename : files) {
    // Check the filename.
    const size_t pos = filename.find(".dat");
    const size_t dash = filename.find_last_of("-");
    if (pos == std::string::npos) {
      warning() << "Skipping " << filename << " (not a .dat file)" << endmsg;
      continue;
    }
    if (!(pos - dash > 1)) {
      warning() << "Unexpected filename (" << filename << ")" << endmsg;
      warning() << "Skipping " << filename << endmsg;
      continue;
    }
    TbRawFile* f = new TbRawFile(filename, m_headerDecoder);
    if (!f->good()) {
      if (!f->is_open()) {
        error() << "Cannot open " << filename << endmsg;
      } else {
        f->close();
      }
      warning() << "Skipping " << filename << endmsg;
      delete f;
      continue;
    }
    const std::string id = f->id();
    const unsigned int plane = geomSvc()->plane(id);
    if (plane == 999) {
      warning() << id << " is not listed in the alignment file" << endmsg;
      warning() << "Skipping " << filename << endmsg;
      f->close();
      delete f;
      continue;
    }
    const unsigned int deviceIndex = geomSvc()->deviceIndex(id);
    if (deviceIndex == 999) {
      warning() << id << " is not listed in the alignment file" << endmsg;
      warning() << "Skipping " << filename << endmsg;
      f->close();
      delete f;
      continue;
    }
    if (m_streams[deviceIndex]->files().empty()) {
      info() << id << " (device " << deviceIndex << ") is mapped to plane "
             << plane << endmsg;
    }
    unsigned int col = 0;
    auto chips = geomSvc()->module(plane)->chips();
    for (auto chip : chips) {
      if (chip.id == id) {
        col = chip.col;
      }
    }
    m_streams[deviceIndex]->setPlane(plane);
    m_streams[deviceIndex]->setDevice(deviceIndex);
    m_streams[deviceIndex]->setOffset(col);
    m_streams[deviceIndex]->addFile(f);
  }
  // Make sure that there are data files for all planes.
  for (unsigned int i = 0; i < m_nDevices; ++i) {
    if (m_streams[i]->files().empty()) {
      return Error("No input files for device " + std::to_string(i));
    }
  }

  // Convert start and end times to FToA.
  m_startTime *= Tb::millisecond * 1000;
  m_endTime *= Tb::millisecond;
  for (auto& f : m_streams) {
    std::sort(f->files().begin(), f->files().end(), lessBySplitIndex());
    f->prepare();
    m_nDataInFiles += f->size();
    // Convert starting time to FToA times
    if (m_startTime != 0) f->fastForward(m_startTime);
  }
  info() << "Total data size: " << m_nDataInFiles << " packets" << endmsg;
  m_clock = m_tick + m_startTime;
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbEventBuilder::execute() {
  std::clock_t c_end = std::clock();
  std::chrono::_V2::system_clock::time_point t_end =
      std::chrono::high_resolution_clock::now();
  if (m_nPackets != 0) {
    double time =
        std::chrono::duration<double, std::milli>(t_end - t_start).count();
    if (m_monitoring) {
      plot2D(m_nPackets, time, "RateVsTime", 0., 1000000., 0., 3000., 100, 100);
      plot2D(m_nPackets, time / (double)m_nPackets, "RateVsTimePerPacket", 0.,
             1000000., 0., 0.01, 100, 100);
    }
    m_nPackets = 0;
  }
  /*
  info() << std::fixed << std::setprecision(2) << "CPU time used: "
         << 1000.0 * (c_end-c_start) / CLOCKS_PER_SEC << " ms\n"
         << "Wall clock time passed: "
         << std::chrono::duration<double, std::milli>(t_end-t_start).count()
         << " ms\n";
   */
  c_start = c_end;
  t_start = t_end;

  // Create containers for hits and triggers.
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    const std::string ext = std::to_string(i);
    LHCb::TbHits* hits = new LHCb::TbHits();
    put(hits, m_hitLocation + ext);
    m_hits[i] = hits;
    LHCb::TbTriggers* triggers = new LHCb::TbTriggers();
    put(triggers, m_triggerLocation + ext);
    m_triggers[i] = triggers;
  }
  bool done = false;
  bool eof = true;
  bool eot = false;
  while (!done) {
    eof = true;
    // Update the event boundaries.
    if (UNLIKELY(msgLevel(MSG::DEBUG))) {
      debug() << "Event definition: " << m_clock - m_tick << " to "
              << m_clock + m_overlapTime << endmsg;
    }
    timingSvc()->setEventDefinition(m_clock - m_tick, m_clock + m_overlapTime);
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      m_hits[i]->clear();
      m_triggers[i]->clear();
    }
    for (auto f : m_streams) {
      eof &= f->eos();
      fill(f, m_hits[f->plane()], m_triggers[f->plane()], eot);
    }
    unsigned int nPlanesWithHits = 0;
    bool gotTrigger = false;
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      if (!m_triggers[i]->empty()) {
        gotTrigger = true;
        break;
      }
      if (!m_hits[i]->empty()) ++nPlanesWithHits;
    }
    if (gotTrigger || nPlanesWithHits >= m_nMinPlanesWithHits) {
      done = true;
    } else {
      ++m_nNoiseEvents;
    }
    m_clock += m_tick;
    if (eof) break;
  }
  // Sort hits and triggers by time.
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    std::sort(m_hits[i]->begin(), m_hits[i]->end(),
              TbFunctors::LessByTime<const LHCb::TbHit*>());
    std::sort(m_triggers[i]->begin(), m_triggers[i]->end(),
              TbFunctors::LessByTime<const LHCb::TbTrigger*>());
    m_nPackets += m_hits[i]->size();
  }
  // Increment the event counter.
  ++m_nEvents;
  if (m_nEvents % m_printFreq == 0) {
    info() << format(" %8u events read, %12u hits, %12u triggers", m_nEvents,
                     m_nHitsRead, m_nTriggersRead) << endmsg;
    if (m_monitoring) {
      info() << "Hits:     ";
      for (unsigned int i = 0; i < m_nPlanes; ++i) {
        info() << format(" %12d", m_hits[i]->size());
      }
      info() << endmsg << "Cache:    ";
      for (unsigned int i = 0; i < m_nPlanes; ++i) {
        info() << format(" %12d", m_streams[i]->hitCache().size());
      }
      info() << endmsg << "Triggers: ";
      for (unsigned int i = 0; i < m_nPlanes; ++i) {
        info() << format(" %12d", m_triggers[i]->size());
      }
      info() << endmsg << "Cache:    ";
      for (unsigned int i = 0; i < m_nPlanes; ++i) {
        info() << format(" %12d", m_streams[i]->trgCache().size());
      }
      info() << endmsg;
    }
  }
  bool emptyCache = true;
  for (const auto& f : m_streams) {
    if (!f->hitCache().empty() || !f->trgCache().empty()) {
      emptyCache = false;
      break;
    }
  }
  // Check if there are any events left to process.
  if (((m_nData == m_nDataInFiles || eof) && emptyCache) || eot) {
    // Terminate the application.
    SmartIF<IEventProcessor> app(serviceLocator()->service("ApplicationMgr"));
    if (app) app->stopRun();
  }
  if (m_nLostTimers > m_maxLostTimers) {
    return Error("More than " + std::to_string(m_maxLostTimers) +
                 " clock packets dropped. Critical problem with global clock");
  }
  if (m_nLostPackets > m_maxLostPackets) {
    return Error("More than " + std::to_string(m_maxLostPackets) +
                 " packets  dropped. Critical problem with timing");
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Add hits and triggers to the containers
//=============================================================================
bool TbEventBuilder::fill(TbRawStream* f, LHCb::TbHits* hits,
                          LHCb::TbTriggers* triggers, bool& eot) {
  if (!dumpCache(f, hits) && !m_forceCaching) {
    if (UNLIKELY(msgLevel(MSG::DEBUG))) {
      debug() << "Event boundary: " << m_clock << endmsg;
      if (f->hitCache().size() > 0) {
        debug() << "First hit in cache:" << f->hitCache().front()->time()
                << endmsg;
      }
      if (f->trgCache().size() > 0) {
        debug() << "First trigger in cache: " << f->trgCache().front()->time()
                << endmsg;
      }
    }
    dumpCache(f, triggers);
    return false;
  }
  dumpCache(f, triggers);
  const unsigned int device = f->device();
  uint64_t currentTime = 0;
  while (likely(currentTime < m_clock + m_cachelength && !f->eos())) {
    const uint64_t packet = f->getNext();
    ++m_nData;
    const unsigned int header = 0xF & (packet >> 60);
    if (likely(header == 0xA || header == 0xB)) {
      // Pixel packets.
      ++m_nHitsRead;
      // Get the pixel adress.
      const unsigned int pixelAddress = 0xFFFF & (packet >> 44);
      // Skip masked pixels.
      if (unlikely(pixelSvc()->isMasked(pixelAddress, device))) continue;
      LHCb::TbHit* hit = decodeTPX3Hit(packet, pixelAddress, device, f->col());
      hit->setCharge(pixelSvc()->charge(hit->ToT(), pixelAddress, device));
      extendTimeStamp(hit, m_ignoreGlobalClock ? f->m_tpx3Timer : f->timer());
      pixelSvc()->applyPhaseCorrection(hit);
      writePacket(hit, f, hits);
      currentTime = hit->time();
      syncTPX3(currentTime, f);
      if (UNLIKELY(m_monitoring)) {
        plot(hit->time() / Tb::millisecond, "HitDataRate",
             "Rate of hit packets", 0.0, 620000.0, 6200000);
      }
    } else if ((header == 0x6 || header == 0x4) &&
               ((packet >> 56) & 0xF) == 0xF) {
      // New trigger packets
      LHCb::TbTrigger* trigger = new LHCb::TbTrigger(packet);
      extendTimeStamp(trigger, f->timer());
      trigger->setPlane(f->plane());
      writePacket(trigger, f, triggers);
      ++m_nTriggersRead;
      if (UNLIKELY(m_monitoring)) {
        plot(trigger->time() / Tb::millisecond, "TriggerDataRate",
             "Rate of trigger packets", 0.0, 620000.0, 620000);
      }
    } else if (header == 0x4 && !m_ignoreGlobalClock) {
      // Timing packets
      int state = f->addTimingPacket(packet);
      if (state == 2) {
        warning() << "Jump in timing of greater than 6.7s detected."
                  << format("Dropping timing packet: 0x%x", packet) << endmsg;
        warning() << "Will attempt to resynchronise using a different SPIDR's"
                  << " global clock" << endmsg;
        attemptResync(f, packet);
        if (m_nLostTimers > m_maxLostTimers) return true;
        // If fail to add as a timing packet, data packet unknown
      } else if (state == 0) {
        ++m_unknownPackets;
        if (UNLIKELY(msgLevel(MSG::DEBUG))) {
          debug() << format("Timing packet with subheader 0x%x, packet = 0x%x",
                            ((packet >> 56) & 0xF), packet) << endmsg;
        }
      }
    } else {
      // Packet is neither hit, nor a trigger, nor a timing packet.
      ++m_unknownPackets;
      if (UNLIKELY(msgLevel(MSG::DEBUG))) {
        warning() << format("Packet with header 0x%x, packet = 0x%x", header,
                            packet) << endmsg;
      }
    }
  }
  std::sort(f->hitCache().begin(), f->hitCache().end(),
            TbFunctors::LessByTimeBP<const LHCb::TbHit*>());
  std::sort(f->trgCache().begin(), f->trgCache().end(),
            TbFunctors::LessByTimeBP<const LHCb::TbTrigger*>());

  if (m_endTime != 0 && currentTime > m_endTime) eot = true;
  if (!hits->empty() || !triggers->empty() || f->eos()) return true;
  return false;
}

//=============================================================================
// Update the clock.
//=============================================================================
void TbEventBuilder::syncTPX3(const uint64_t thisPacketTime, TbRawStream* f) {
  const int timerMsbs = 0x3 & (f->m_tpx3Timer >> 40);
  int thisMsb = 0x3 & (thisPacketTime >> 40);
  constexpr uint64_t one = (uint64_t)(1) << 40;

  if (thisMsb == timerMsbs + 1 || thisMsb == timerMsbs - 3) {
  //  info() << "Updating clock "
  //         << (double)(f->m_tpx3Timer + one) / (double)Tb::second << endmsg;
    f->m_tpx3Timer += one;
  }
  if (thisMsb > timerMsbs) {
 //   info() << thisMsb << "   " << timerMsbs << endmsg;
  }
}

//=============================================================================
// Resynchronise a plane.
//=============================================================================
bool TbEventBuilder::attemptResync(TbRawStream* f, const uint64_t packet) {
  for (unsigned int chip = 0; chip != m_nPlanes; ++chip) {
    int64_t dt = (int64_t)f->timer() - (int64_t)m_streams[chip]->timer();
    if (std::abs(dt) > m_maxTimeDifference) {
      f->setLSB(m_streams[chip]->lsb());
      f->setGlobalClock(m_streams[chip]->timer());
      const int state = f->addTimingPacket(packet);
      if (state != 2) {
        info() << "Resync of device " << chip << " successful!" << endmsg;
        return true;
      }
    }
  }
  warning() << "Resynchronisation of device fails" << endmsg;
  // Empty the cache.
  for (auto it = f->hitCache().begin(); it != f->hitCache().end(); ++it) {
    if (*it) delete *it;
  }
  f->hitCache().clear();
  for (auto it = f->trgCache().begin(); it != f->trgCache().end(); ++it) {
    if (*it) delete *it;
  }
  f->trgCache().clear();
  m_nLostTimers++;
  return false;
}

//=============================================================================
// Finalization
//=============================================================================
StatusCode TbEventBuilder::finalize() {
  for (auto& f : m_streams) {
    info() << format("Plane %2u: %12u packets in file, %12u hits in cache",
                     f->plane(), f->size(), f->hitCache().size()) << endmsg;
    for (auto it = f->hitCache().begin(); it != f->hitCache().end(); ++it) {
      if (*it) delete *it;
    }
    for (auto it = f->trgCache().begin(); it != f->trgCache().end(); ++it) {
      if (*it) delete *it;
    }
    f->close();
  }
  info() << "Fraction of data read: "
         << (double)m_nData / (double)m_nDataInFiles << endmsg;
  info() << "Number of packets lost: " << m_nLostPackets << endmsg;
  info() << "Unknown packets: " << m_unknownPackets << endmsg;
  if (m_nNoiseEvents > 0) {
    info() << "Skipped " << m_nNoiseEvents << " noise events." << endmsg;
  }
  // Finalise the base class.
  return TbAlgorithm::finalize();
}

//=============================================================================
// Dump cached packets into the current event
//=============================================================================
template <typename T>
bool TbEventBuilder::dumpCache(
    TbRawStream* stream, KeyedContainer<T, Containers::HashMap>* container) {
  // Load the hits/triggers stored in the cache.
  std::vector<T*>* cache = stream->cache<T*>();
  auto it = cache->begin();
  const auto end = cache->end();
  for (; it != end; ++it) {
    const auto time = (*it)->time();
    if (unlikely(time >= m_clock + m_overlapTime)) break;
    if (unlikely(time < m_clock - m_tick)) {
      // Packet is earlier than the current event.
      // If this happens, the cache may be too short.
      warning() << format("Packet 0x%016llX", (*it)->data()) << " is "
                << " ns before event low edge! Current event definition: "
                << (m_clock - m_tick) / Tb::SpidrTime << " to "
                << (m_clock + m_overlapTime) / Tb::SpidrTime << endmsg;
      ++m_nLostPackets;
      delete *it;
      if (m_nLostPackets > m_maxLostPackets) {
        error() << "Large number of packets lost -> critical failure" << endmsg;
        return false;
      }
      continue;
    }
    (*it)->setHtime(timingSvc()->globalToLocal(time));
    // Move packet to the TES.
    container->insert(*it);
  }
  cache->erase(cache->begin(), it);
  if (container->empty() && !cache->empty()) return false;
  return true;
}

//=============================================================================
// Extend the timestamp of a packet
//=============================================================================
template <typename T>
void TbEventBuilder::extendTimeStamp(T* packet, uint64_t global_time) {
  const uint64_t packet_time = packet->time();
  const int diff = (0x3 & (global_time >> 40)) - (0x3 & (packet_time >> 40));
  constexpr uint64_t one = (uint64_t)(1) << 40;
  // Calculate the difference between the bits that should match between
  // the spidr time and the global time, if they do not match, increment or
  // decrement the global time so that they match and add the 18 m.s.f.
  // of the global time to the packet time
  if (diff == 1 || diff == -3) {
    global_time = global_time - one;
  } else if (diff == -1 || diff == 3) {
    global_time = global_time + one;
  }
  packet->setTime((0x3FFFFFFFFFF & packet_time) +
                  (global_time & 0xFFFFC0000000000));
}

//=============================================================================
// Decode a Timepix3 pixel packet and create a TbHit.
//=============================================================================
LHCb::TbHit* TbEventBuilder::decodeTPX3Hit(const uint64_t packet,
                                           const unsigned int pixelAddress,
                                           const unsigned int device,
                                           const unsigned int fCol) {
  LHCb::TbHit* hit = new LHCb::TbHit();
  hit->setDevice(device);
  hit->setData(packet);
  hit->setPixelAddress(pixelAddress);
  // Decode the pixel address, first get the double column.
  const unsigned int dcol = (0xFE00 & pixelAddress) >> 8;
  // Get the super pixel address.
  const unsigned int spix = (0x01F8 & pixelAddress) >> 1;
  // Get the address of the pixel within the super pixel.
  const unsigned int pix = (0x0007 & pixelAddress);
  // Calculate and store the row and column numbers.
  const unsigned int col = dcol + pix / 4;
  const unsigned int row = spix + (pix & 0x3);
  hit->setCol(col);
  hit->setRow(row);
  hit->setScol(col + fCol);
  const unsigned int data = (packet & 0x00000FFFFFFF0000) >> 16;
  // Extract and store the ToT and the corresponding charge.
  const unsigned int tot = (data & 0x00003FF0) >> 4;
  hit->setToT(tot);
  // Get the time stamps.
  const uint64_t spidrTime = packet & 0x000000000000FFFF;
  const uint64_t ftoa = data & 0x0000000F;
  const uint64_t toa = (data & 0x0FFFC000) >> 14;
  // Calculate the global timestamp.
  const uint64_t fulltime = ((spidrTime << 18) + (toa << 4) + (15 - ftoa)) << 8;
  hit->setTime(fulltime);

  return hit;
}
