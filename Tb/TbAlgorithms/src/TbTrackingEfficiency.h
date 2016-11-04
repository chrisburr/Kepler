#ifndef TB_TRACKPLOTS_H
#define TB_TRACKPLOTS_H 1

// AIDA
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"

#include "AIDA/IProfile2D.h"
#include "AIDA/IProfile1D.h"

// Tb/TbEvent
#include "Event/TbTrack.h"

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"

/** @class TbTrackingEfficiency TbTrackingEfficiency.h
 *
 *  @author Dan Saunders
 */

class TbTrackingEfficiency : public TbAlgorithm {
 public:
  /// Constructor
  TbTrackingEfficiency(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbTrackingEfficiency() {}

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution
  virtual StatusCode finalize();    ///< Algorithm finalization

 private:
  /// TES location of tracks
  std::string m_trackLocation;
  /// TES location of clusters
  std::string m_clusterLocation;

  AIDA::IHistogram1D* m_hRatioTracksClustersCentral;
  AIDA::IHistogram1D* m_hnClustersPerPlaneCentral;
  AIDA::IHistogram1D* m_hnTracksInterceptCentral;
  AIDA::IHistogram1D* m_hFractionTrackedClusters;
  AIDA::IHistogram1D* m_hnClustersPerPlane;
  AIDA::IHistogram1D* m_hnTrackedClusters;

  AIDA::IHistogram1D* m_telHitOccupancy;
  AIDA::IHistogram1D* m_telHitOccupancy_tracked;
  AIDA::IHistogram1D* m_nClusters_vs_telHitOccupancy;
  AIDA::IHistogram1D* m_nTrackedClusters_vs_telHitOccupancy;
  AIDA::IHistogram1D* m_fractionTrackedClusters_vs_telHitOccupancy;

  AIDA::IHistogram1D* m_telCharge;
  AIDA::IHistogram1D* m_nClusters_vs_telCharge;
  AIDA::IHistogram1D* m_nTrackedClusters_vs_telCharge;
  AIDA::IHistogram1D* m_fractionTrackedClusters_vs_telCharge;

  AIDA::IHistogram1D* m_telClusterOccupancy;
  AIDA::IHistogram1D* m_telClusterOccupancy_tracked;
  AIDA::IHistogram1D* m_nClusters_vs_telClusterOccupancy;
  AIDA::IHistogram1D* m_nTrackedClusters_vs_telClusterOccupancy;
  AIDA::IHistogram1D* m_fractionTrackedClusters_vs_telClusterOccupancy;

  /// Parameters for central region cuts.
  Gaudi::Histo1DDef m_parCentral;
  double m_chargeCutLow;

  void setupPlots();
  void fillClusterLoopPlots(const LHCb::TbClusters* clusters,
                            const unsigned int plane);
  void fillTrackingEfficiency();
};
#endif
