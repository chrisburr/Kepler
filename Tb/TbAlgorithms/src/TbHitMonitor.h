#ifndef TB_HIT_MONITOR_H
#define TB_HIT_MONITOR_H 1

// AIDA
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
#include "AIDA/IProfile1D.h"

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"

/** @class TbHitMonitor TbHitMonitor.h
 *
 *  Algorithm to produce monitoring histograms for Timepix3 pixel hits.
 *
 *  @author Tim Evans (timothy.david.evans@cern.ch)
 *  @date   2014-04-01
 */

class TbHitMonitor : public TbAlgorithm {
 public:
  /// Standard constructor
  TbHitMonitor(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbHitMonitor();

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  /// TES location of hits.
  std::string m_hitLocation;
  /// Parameters for ToT distribution histograms
  Gaudi::Histo1DDef m_parToT;
  /// Parameters for charge distribution histograms
  Gaudi::Histo1DDef m_parCharge;
  /// Parameters for hits / event distribution histograms
  Gaudi::Histo1DDef m_parHitsInEvent;
  /// Parameters for time difference histograms
  Gaudi::Histo1DDef m_parDeltaT;

  /// Event counter
  unsigned int m_nEvents = 0;

  std::vector<AIDA::IHistogram2D*> m_hHitMap;
  std::vector<AIDA::IHistogram1D*> m_hToT;
  std::vector<AIDA::IHistogram1D*> m_hCharge;
  std::vector<AIDA::IProfile1D*> m_hToTvsCol;
  std::vector<AIDA::IProfile1D*> m_hChargevsCol;
  std::vector<AIDA::IHistogram1D*> m_hHitsInEvent;
  std::vector<AIDA::IHistogram1D*> m_hHitsInEventTrend;
  std::vector<AIDA::IHistogram1D*> m_hTimeBetweenHits;
};

#endif
