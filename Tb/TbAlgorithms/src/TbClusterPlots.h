#ifndef TB_CLUSTERPLOTS_H
#define TB_CLUSTERPLOTS_H 1

// AIDA
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"

// Tb/TbEvent
#include "Event/TbCluster.h"

// Tb/TbKernel
#include "TbKernel/ITbClusterFinder.h"
#include "TbKernel/TbAlgorithm.h"

/** @class TbClusterPlots TbClusterPlots.h
 *
 *  Algorithm to produce monitoring histograms for Timepix3 clusters.
 *
 *  @author Dan Saunders
 */

class TbClusterPlots : public TbAlgorithm {
 public:
  /// Standard constructor
  TbClusterPlots(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbClusterPlots();

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  /// TES location prefix of clusters
  std::string m_clusterLocation;
  /// Index of reference plane
  unsigned int m_referencePlane;
  /// Time window (in ns) for correlation/difference plots
  double m_twindow;

  /// Parameters for ToT distribution histograms
  Gaudi::Histo1DDef m_parToT;
  /// Parameters for charge distribution histograms
  Gaudi::Histo1DDef m_parCharge;
  /// Parameters for x/y histograms (hitmaps and correlations)
  Gaudi::Histo1DDef m_parXY;
  /// Parameters for time histograms
  Gaudi::Histo1DDef m_parTime;
  /// Parameters for x/y difference histograms
  Gaudi::Histo1DDef m_parDifferenceXY;
  /// Parameters for phi difference histograms
  Gaudi::Histo1DDef m_parDifferenceRot;
  /// Parameters for time difference histograms
  Gaudi::Histo1DDef m_parDifferenceT;
  Gaudi::Histo1DDef m_parSamples;


  bool m_fillSamples;
  bool m_fillComparisonPlots;     // Much faster to turn off.
  /// Cluster finder helper class
  ITbClusterFinder* m_clusterFinder = nullptr;
  unsigned int m_event = 0;


  // Histograms
  // ToT distribution
  std::vector<AIDA::IHistogram1D*> m_hToT;
  std::vector<AIDA::IHistogram1D*> m_hToTOnePixel;
  std::vector<AIDA::IHistogram1D*> m_hToTTwoPixel;
  std::vector<AIDA::IHistogram1D*> m_hToTThreePixel;
  std::vector<AIDA::IHistogram1D*> m_hToTFourPixel;
  std::vector<AIDA::IHistogram1D*> m_hToTAssociated;
  std::vector<AIDA::IHistogram1D*> m_hToTNonAssociated;
  // Charge distribution
  std::vector<AIDA::IHistogram1D*> m_hCharge;
  std::vector<AIDA::IHistogram1D*> m_hChargeOnePixel;
  std::vector<AIDA::IHistogram1D*> m_hChargeTwoPixel;
  std::vector<AIDA::IHistogram1D*> m_hChargeThreePixel;
  std::vector<AIDA::IHistogram1D*> m_hChargeFourPixel;
  std::vector<AIDA::IHistogram1D*> m_hChargeAssociated;
  std::vector<AIDA::IHistogram1D*> m_hChargeNonAssociated;
  // Cluster size
  std::vector<AIDA::IHistogram1D*> m_hSize;
  std::vector<AIDA::IHistogram1D*> m_hSizeAssociated;
  std::vector<AIDA::IHistogram1D*> m_hSizeNonAssociated;
  std::vector<AIDA::IHistogram1D*> m_hWidthCol;
  std::vector<AIDA::IHistogram1D*> m_hWidthRow;
  // Cluster time
  std::vector<AIDA::IHistogram1D*> m_hTime;
  std::vector<AIDA::IHistogram1D*> m_hTimeAssociated;
  std::vector<AIDA::IHistogram1D*> m_hTimeNonAssociated;
  std::vector<AIDA::IHistogram1D*> m_hTimeBetweenClusters;
  // Time spread of pixel hits in a cluster
  std::vector<AIDA::IHistogram1D*> m_hTimeSeedMinusHit;

  // Hitmaps
  std::vector<AIDA::IHistogram2D*> m_hHitMap;
  std::vector<AIDA::IHistogram2D*> m_hHitMapAssociated;
  std::vector<AIDA::IHistogram2D*> m_hHitMapNonAssociated;

  AIDA::IHistogram2D* m_hGlobalXvsZ = nullptr;
  AIDA::IHistogram2D* m_hGlobalYvsZ = nullptr;

  // Global correlations.
  std::vector<AIDA::IHistogram2D*> m_gx_correls;
  std::vector<AIDA::IHistogram2D*> m_gy_correls;
  std::vector<AIDA::IHistogram2D*> m_gt_correls;

  // Global differences.
  std::vector<AIDA::IHistogram1D*> m_gx_diffs;
  std::vector<AIDA::IHistogram1D*> m_gy_diffs;
  std::vector<AIDA::IHistogram1D*> m_gt_diffs;

  std::vector<AIDA::IHistogram2D*> m_clusterVisuals;

  void setupPlots();
  void fillPerChipPlots(const LHCb::TbClusters* clusters);
  void fillComparisonPlots();
  void fillSamples(const LHCb::TbClusters* clusters);
  void fillClusterVisuals(const LHCb::TbClusters* clusters);

};
#endif
