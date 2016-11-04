#pragma once

#include "TbRawFile.h"
#include "Event/TbHit.h"
#include "Event/TbTrigger.h"
#include "TbKernel/TbFunctors.h"
#include "GaudiAlg/GaudiTool.h"
#include "TbKernel/TbConstants.h"

static const InterfaceID IID_TbRawStream("TbRawStream", 1, 0);

class TbRawStream : public GaudiTool {

 public:
  /// Constructor
  TbRawStream(const std::string& type, const std::string& name,
              const IInterface* parent);

  static const InterfaceID& interfaceID() { return IID_TbRawStream; }

  uint64_t timer() const { return m_timer; }
  unsigned int lsb() const { return m_lsb; }
  // this is for explicitly setting the clock, if device sync fails
  void setGlobalClock(const uint64_t timer) { m_timer = timer; }
  unsigned int size() const { return m_size; }
  void setLSB(const unsigned int lsb) { m_lsb = lsb; }
  bool setMSB(uint64_t msb);

  std::vector<LHCb::TbHit*>& hitCache() { return m_hitCache; }
  std::vector<LHCb::TbTrigger*>& trgCache() { return m_trgCache; }
  void addFile(TbRawFile* file) { m_files.push_back(file); }
  void close() {
    for (auto& raw_file : m_files) {
      if (raw_file->is_open()) raw_file->close();
    }
  }

  uint64_t getNext() { return (*m_currentFile)->getNext(); }
  uint64_t getPrevious() { return (*m_currentFile)->getPrevious(); }

  bool eos() {
    if (unlikely( m_currentFile == m_files.end() ) ) return true;
    if (unlikely((*m_currentFile)->eof() == true)) {
      (*m_currentFile)->close();
      m_currentFile++;
      if (m_currentFile != m_files.end()) (*m_currentFile)->initialise();
    }
    return unlikely(m_currentFile == m_files.end());
  }
  double hClock() const {
    return (double)m_timer / (double) Tb::second; 
  }
  void prepare();

  // these functions collectively control fast forwarding to some point in the
  // stream
  void fastForward(const uint64_t timeToSkipTo);
  void coarseFastForward(const double timeToSkipTo);
  void fineFastForward(const uint64_t timeToSkipTo);
  uint64_t getCurrentTime();

  int addTimingPacket(const uint64_t data_packet);
  std::vector<TbRawFile*>& files() { return m_files; }
  unsigned int plane() const { return m_plane; }
  void setOffset(const int colOffset) {
    m_colOffset = colOffset;
  }
  void setDevice(const unsigned int device) { m_device = device; }
  void setPlane(const unsigned int plane) { m_plane = plane; }
  unsigned int col() const { return m_colOffset; }
  unsigned int device() const { return m_device; }

  template <typename T>
  std::vector<T>* cache();

  void insert(LHCb::TbTrigger* packet);
  void insert(LHCb::TbHit* packet);
  uint64_t m_tpx3Timer = 0; 
 private:
  static const int64_t m_tenSeconds;
  static const int64_t m_maxTimeDifference;

  std::vector<TbRawFile*>::iterator m_currentFile;
  std::vector<TbRawFile*> m_files;
  std::vector<LHCb::TbHit*> m_hitCache;
  std::vector<LHCb::TbTrigger*> m_trgCache;
  unsigned int m_plane;
  unsigned int m_device;
  uint64_t m_size = 0;
  /// Temporary lsb of the global timer
  unsigned int m_lsb = 0;
  uint64_t m_timer = 0;
  unsigned int m_colOffset;
};
