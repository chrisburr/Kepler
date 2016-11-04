#ifndef TB_TRIGGER_MONITOR_H
#define TB_TRIGGER_MONITOR_H 1

// AIDA
#include "AIDA/IHistogram1D.h"

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"

/** @class TbTriggerMonitor TbTriggerMonitor.h
 *
 *  Algorithm to produce monitoring histograms for scintillator triggers.
 *
 */

class TbTriggerMonitor : public TbAlgorithm {
 public:
  /// Standard constructor
  TbTriggerMonitor(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbTriggerMonitor();

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution
  virtual StatusCode finalize();    ///< Algorithm Finalize
 private:
  /// TES location of triggers.
  std::string m_triggerLocation;

  /// Parameters for hits / event distribution histograms
  Gaudi::Histo1DDef m_parTriggersInEvent;

  /// Event counter
  unsigned int m_nEvents;

  /// Last trigger counters
  std::vector<unsigned int> m_counter;

  std::vector<AIDA::IHistogram1D*> m_hTimeBetweenTriggers;
  std::vector<AIDA::IHistogram1D*> m_hTriggersInEvent;
  std::vector<AIDA::IHistogram1D*> m_hTriggersInEventTrend;
  std::vector<std::vector<std::pair<uint64_t, uint64_t>>> m_missedTriggers;
};

#endif
