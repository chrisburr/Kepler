#pragma once

// AIDA
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"

/** @class TbDUTMonitor TbDUTMonitor.h
 *
 */

class TbDUTMonitor : public TbAlgorithm {
 public:
  /// Constructor
  TbDUTMonitor(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbDUTMonitor() {}

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  std::vector<unsigned int> m_duts;
  std::map<unsigned int, unsigned int> m_index;

  std::string m_trackLocation;

  /// Flag to switch on/off scan histograms
  bool m_scan;

  /// Parameters for x/y residual histograms
  Gaudi::Histo1DDef m_parResXY;
  Gaudi::Histo1DDef m_parXY;
  Gaudi::Histo1DDef m_parScanX;
  Gaudi::Histo1DDef m_parScanY;

  /// Parameters for time residual histograms
  Gaudi::Histo1DDef m_parResTime;

  std::vector<AIDA::IHistogram1D*> m_hResLocalX;
  std::vector<AIDA::IHistogram1D*> m_hResLocalY;
  std::vector<AIDA::IHistogram1D*> m_hResGlobalX;
  std::vector<AIDA::IHistogram1D*> m_hResGlobalY;
  std::vector<AIDA::IHistogram1D*> m_hResTime;

  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGlobalXvsGlobalX;
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGlobalYvsGlobalY;
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGlobalXvsTrackChi2;
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGlobalYvsTrackChi2;

  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGlobalXvsPixelX;
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGlobalXvsPixelY;
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGlobalYvsPixelX;
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGlobalYvsPixelY;

  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGlobalXvsTrackTx;
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGlobalXvsTrackTy;
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGlobalYvsTrackTx;
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGlobalYvsTrackTy;
  std::vector<AIDA::IHistogram2D*> m_UnbiasedResGlobalXvshUnbiasedResGlobalY;
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGlobalXvsClusterSize;
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGlobalYvsClusterSize;

  std::vector<AIDA::IHistogram2D*> m_hScanXvsX;
  std::vector<AIDA::IHistogram2D*> m_hScanYvsY;

  std::vector<TbModule*> m_testModules;
};
