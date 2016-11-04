#ifndef TB_EFFICIENCY_H
#define TB_EFFICIENCY_H 1

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"
#include "AIDA/IHistogram1D.h"
#include "TEfficiency.h"
#include "TH2.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"

/** @class TbEfficiency TbEfficiency.h
 *
 */

class TbEfficiency : public TbAlgorithm {
 public:
  /// Constructor
  TbEfficiency(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbEfficiency() {}

  bool m_checkHitDUT;
  bool m_checkHitAlivePixel;
  double m_pointingResAllowance;
  uint m_nTracksConsidered;
  uint m_nTracksAssociated;
  uint m_event;
  int m_pointingResAllowance_deadPixels;
  bool m_takeDeadPixelsFromFile;
  TH2F * m_deadPixelMap;
  double m_pitch;
  uint m_nTotalTracks;
  double m_maxChi;

  bool passedThroughDUT(Gaudi::XYZPoint interceptUL);
  bool passedThroughAlivePixel(Gaudi::XYZPoint interceptUL);

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution
  virtual StatusCode finalize();
  bool passSelectionCriteria(LHCb::TbTrack * track, Gaudi::XYZPoint interceptUL);

  TEfficiency * h_row;
  TEfficiency * h_col;
  TEfficiency * m_eff;
  TEfficiency * h_hitmap;
  TEfficiency * h_pixel;
  TEfficiency * h_pixel2x2;

 private:
  uint m_dut;
  std::string m_trackLocation;
  std::string m_clusterLocation;
};
#endif
