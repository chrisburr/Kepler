#ifndef TB_EVENTBUILDER_H
#define TB_EVENTBUILDER_H 1

#include <ctime>
#include <chrono>

// Tb/TbEvent
#include "Event/TbHit.h"
#include "Event/TbTrigger.h"

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"
#include "TbKernel/TbModule.h"

// Local
#include "TbRawStream.h"
#include "TbRawFile.h"
#include "TbHeaderDecoder.h"

/** @class TbEventBuilder TbEventBuilder.h
 *
 *  Algorithm to populate TES with time ordered gaudi events with data read in
 *  in the SPIDR data format
 *
 *  @author Tim Evans (timothy.david.evans@cern.ch)
 *  @date   2014-04-01
 */

class TbEventBuilder : public TbAlgorithm {
 public:
  /// Standard constructor
  TbEventBuilder(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbEventBuilder();

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution
  virtual StatusCode finalize();    ///< Algorithm termination

 private:
  /// Max. time difference for resynchronisation
  static const int64_t m_maxTimeDifference;
  std::clock_t c_start;
  std::chrono::_V2::system_clock::time_point t_start;
  /// Input data files
  std::vector<std::string> m_input;
  /// TES location of output hits
  std::string m_hitLocation;
  /// TES location of output triggers
  std::string m_triggerLocation;

  /// The length of the time window used by the event definition
  uint64_t m_tick;
  /// The length of time looking in the the next event
  uint64_t m_cachelength;
  /// The length of time by which event definitions 'overlap',
  /// to be sorted out by tracking
  uint64_t m_overlapTime;

  /// Time at which to start reading (in trigger FToA times)
  uint64_t m_startTime;
  /// Time at which to stop reading (specified in ms, converted later to FToA)
  uint64_t m_endTime;

  /// Min. number of non-empty planes required to make an event
  unsigned int m_nMinPlanesWithHits;

  /// Frequency to print event count.
  unsigned int m_printFreq;
  /// The total header size. If set to 0, is read from header.
  unsigned int m_headerSize;
  /// Flag to dump header or not.
  bool m_printHeader;
  /// Flag to activate detailed print-out and histograms.
  bool m_monitoring;
  /// Flag to ignore the global clock packets
  bool m_ignoreGlobalClock;
  /// The end of the time window used by the event definition
  uint64_t m_clock;

  /// Number of processed events
  unsigned int m_nEvents = 0;
  /// Number of skipped noise events
  unsigned int m_nNoiseEvents = 0;
  /// Number of created hits
  uint64_t m_nData = 0;
  /// Number of read hit packets
  uint64_t m_nHitsRead = 0;
  /// Number of read trigger packets
  unsigned int m_nTriggersRead = 0;
  /// Number of unknown packets
  unsigned int m_unknownPackets = 0;
  /// Number of out-of-time packets
  unsigned int m_nLostPackets = 0;
  unsigned int m_nLostTimers = 0;
  /// Maximum number of lost packets / clock packets
  unsigned int m_maxLostPackets;
  unsigned int m_maxLostTimers;
  /// Another eof checksum
  uint64_t m_nDataInFiles = 0;
  /// Number of packets per event
  unsigned int m_nPackets = 0;

  /// Forces the cache to update every cycle, useful
  /// for runs where a bad packet has confused the caching
  bool m_forceCaching;
  /// TbRawStreams for each plane
  std::vector<TbRawStream*> m_streams;
  /// Triggers in each plane
  std::vector<LHCb::TbTriggers*> m_triggers;
  /// Hits in each plane
  std::vector<LHCb::TbHits*> m_hits;

  /// Header decoder tool
  TbHeaderDecoder* m_headerDecoder = nullptr;

  bool fill(TbRawStream* f, LHCb::TbHits* hits, LHCb::TbTriggers* triggers,
            bool& eot);

  /// Templated cache dump for triggers and hits (extendible to other
  /// data packets with a time field and a htime field
  template <typename T>
  bool dumpCache(TbRawStream* stream,
                 KeyedContainer<T, Containers::HashMap>* container);

  /// Extend the timestamp of a packet to include the global time
  template <typename T>
  void extendTimeStamp(T* packet, uint64_t global_time);

  /// Write a data packet to either a cache or the TES, depending on
  /// whether it is contained within the current event definition
  template <typename T>
  void writePacket(T* packet, TbRawStream* stream,
                   KeyedContainer<T, Containers::HashMap>* container) {
    const uint64_t time = packet->time();
    if (time < m_clock + m_overlapTime && time >= m_clock - m_tick) {
      // Timestamp is inside the current event. Add the packet to the TES.
      packet->setHtime(timingSvc()->globalToLocal(time));
      container->insert(packet);
    } else {
      stream->insert(packet);
    }
  }
  LHCb::TbHit* decodeTPX3Hit(const uint64_t packet,
                             const unsigned int pixelAddress,
                             const unsigned int device,
                             const unsigned int fCol);
  void syncTPX3(const uint64_t thisPacketTime, TbRawStream* f);
  bool attemptResync(TbRawStream* f, const uint64_t packet);
  /// Functor for sorting files by split index
  class lessBySplitIndex {
   public:
    bool operator()(const TbRawFile* a, const TbRawFile* b) const {
      return a->splitIndex() < b->splitIndex();
    }
  };
};

#endif
