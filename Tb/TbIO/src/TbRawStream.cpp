#include "TbRawStream.h"
#include <cmath>

DECLARE_TOOL_FACTORY(TbRawStream)

const int64_t TbRawStream::m_tenSeconds = 256 * std::pow(10, 11);
const int64_t TbRawStream::m_maxTimeDifference = pow(2, 40);

//=============================================================================
// Standard constructor
//=============================================================================
TbRawStream::TbRawStream(const std::string& type, const std::string& name,
                         const IInterface* parent)
    : GaudiTool(type, name, parent),
      m_hitCache(), m_trgCache() {
  declareInterface<TbRawStream>(this);
}

bool TbRawStream::setMSB(uint64_t msb) {
  uint64_t currentTime = (m_lsb + (msb << 32)) << 12;
  const int64_t timeDifference = currentTime - m_timer;
  if (timeDifference > m_tenSeconds) {
    // Detected jump of greater than 10s
    // warning() << "Jump detected - this is okay, probably means "
    // << "a bit has been updated between LSB and MSB" << endmsg;
    if (msb > 0) msb -= 1;
    currentTime = (m_lsb + (msb << 32)) << 12;
  }
  if (timeDifference > m_maxTimeDifference ||
      timeDifference < -m_maxTimeDifference) {
    // warning << "Current global time = " <<
    // m_timer*25*pow(10,-6)/4096 << " clock is trying to update to " <<
    // currentTime*25*pow(10,-6)/4096 << endmsg;
    return false;
  }
  m_timer = currentTime;
  // info() <<  "Current global time: " <<
  // currentTime*25/(4096*pow(10,9)) << " s" <<endmsg;
  return true;
}

void TbRawStream::fastForward(const uint64_t timeToSkipTo) {
  // Go one second before we need to
  const double coarse_time = timeToSkipTo * 25 / (4096 * pow(10, 9)); 
  coarseFastForward(coarse_time);
  fineFastForward(timeToSkipTo);
}

//=============================================================================
// Binary search of the stream to find a particular time
//=============================================================================
void TbRawStream::coarseFastForward(const double timeToSkipTo) {

  info() << "Skipping forward to time = " << timeToSkipTo << endmsg;
  // Need to work out which file to read first
  if (m_files.size() > 1) {
    for (auto it = m_files.begin(), end = m_files.end(); it != end; ++it) {
      m_currentFile = it;
      (*it)->initialise();
      const double timeInSeconds = getCurrentTime() * 25 / pow(10, 9);
      if (timeInSeconds > timeToSkipTo) {
        (*it)->reset();
        m_currentFile--;
        break;
      }
      (*it)->reset();
    }
  }

  (*m_currentFile)->initialise();
  uint64_t dt = (*m_currentFile)->nPackets() / 2;
  uint64_t pos = dt;
  uint64_t timer = 0;
  double timeInSeconds = 0;
  while (!eos() && fabs(timeToSkipTo - timeInSeconds) > 0.6 && dt > 1) {
    // Scroll to the new position
    (*m_currentFile)->setOffset(pos);  
    dt /= 2;
    timer = getCurrentTime();
    timeInSeconds = timer * 25 / pow(10, 9);
    pos = timeToSkipTo > timeInSeconds ? (pos + dt) : (pos - dt);
  }
  if (dt <= 1) info() << "Binary search has failed!" << endmsg;
  m_timer = timer << 12;
}

void TbRawStream::fineFastForward(const uint64_t timeToSkipTo) {
  uint64_t currentTime(0);
  while (!eos() && currentTime < timeToSkipTo) {
    const uint64_t data_packet = getNext();
    const unsigned int header = data_packet >> 60;
    if (header == 0xA || header == 0xB) {
      uint64_t global_time = timer();
      uint64_t packet_time = (0xFFFF & data_packet) << 26;
      const int diff =
          (0x3 & (global_time >> 40)) - (0x3 & (packet_time >> 40));
      constexpr uint64_t one = (uint64_t)(1) << 40;
      if (diff == 1 || diff == -3)
        global_time = global_time - one;
      else if (diff == -1 || diff == 3)
        global_time = global_time + one;
    } else if (header == 0x4) {
      addTimingPacket(data_packet);
      currentTime = m_timer;
    }
  }
}

uint64_t TbRawStream::getCurrentTime() {
  m_lsb = 0;
  uint64_t currentTime = 0;
  bool gotTime = false;
  while (!eos() && !gotTime) {
    // measures the current time in the stream//
    const uint64_t data_packet = getNext();
    const unsigned int header = data_packet >> 60;
    if (header == 0x4 && (0xF & (data_packet >> 54)) != 0xF) {
      const unsigned int subheader = 0xF & (data_packet >> 56);
      if (subheader == 0x4) {
        m_lsb = 0xFFFFFFFF & (data_packet >> 16);
      } else if (subheader == 0x5 && m_lsb != 0) {
        const uint64_t msb = 0xFFFFFFFF & (data_packet >> 16);
        currentTime = (m_lsb + (msb << 32));
        gotTime = true;
      }
    }
  }
  return currentTime;
}

int TbRawStream::addTimingPacket(const uint64_t data_packet) {
  const unsigned int subheader = 0xF & (data_packet >> 56);
  int state = 1;
  if (subheader == 0x5) {
    // info() << "Current msb = Setting msb of clock: 0x" <<
    // std::hex << data_packet <<  std::dec <<endmsg;
    if (setMSB(0xFFFFFFFF & (data_packet >> 16)) == 0) state = 2;
  } else if (subheader == 0x4) {
    // info() << "Setting lsb of stream: 0x" << std::hex <<
    // data_packet << ", current = " << m_lsb << std::dec << endmsg;
    setLSB(0xFFFFFFFF & (data_packet >> 16));
  } else {
    state = 0;
  }
  return state;
}

void TbRawStream::prepare() {
  m_currentFile = m_files.begin();
  (*m_currentFile)->initialise();
  m_size = 0;
  for (const auto& raw_file : m_files) m_size = m_size + raw_file->nPackets();
  //info() << "Stream = " << m_size << " 8-byte packets" << endmsg; 
  unsigned int header(0);
  // Find the first pixel hit
  unsigned int prep_packets(0);
  while (!eos() && !(header == 0xA || header == 0xB)) {
    uint64_t packet = getNext();
    // info() << std::hex << packet << endmsg;
    header = 0xF & (packet >> 60);
    ++prep_packets;
  }
  // for (unsigned int i = 0; i < 100; ++i) {
  //   info() << std::hex << "0x" << getNext() << std::dec << endmsg;
  // }
  // info() << "Number of prep packets skipped = " << prep_packets << endmsg;
  m_size = m_size - prep_packets + 1;
  getPrevious();
}

template <>
std::vector<LHCb::TbHit*>* TbRawStream::cache() {
  return &m_hitCache;
}
template <>
std::vector<LHCb::TbTrigger*>* TbRawStream::cache() {
  return &m_trgCache;
}

void TbRawStream::insert(LHCb::TbHit* packet) { m_hitCache.push_back(packet); }

void TbRawStream::insert(LHCb::TbTrigger* packet) {
  m_trgCache.push_back(packet);
}
