// Gaudi
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiUtils/HistoLabels.h"
#include "GaudiUtils/Aida2ROOT.h"

// Tb/TbKernel
#include "TbKernel/TbConstants.h"

// Local
#include "TbTrackingEfficiency.h"

using namespace Gaudi::Utils::Histos;

DECLARE_ALGORITHM_FACTORY(TbTrackingEfficiency)

//=============================================================================
// Standard constructor
//=============================================================================
TbTrackingEfficiency::TbTrackingEfficiency(const std::string& name, ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator),
      m_parCentral("", 4, 10, 1) {

  declareProperty("TrackLocation",
                  m_trackLocation = LHCb::TbTrackLocation::Default);
  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);
  declareProperty("ParametersCentral", m_parCentral);
  declareProperty("ChargeCutLow", m_chargeCutLow = 0);
}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbTrackingEfficiency::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  setupPlots();
  return StatusCode::SUCCESS;
}

//=============================================================================
// Finalizer
//=============================================================================
StatusCode TbTrackingEfficiency::finalize() {

  // Fill plots used after all events have been evaluated.
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    double num =
        m_hnTrackedClusters->binHeight(m_hnTrackedClusters->coordToIndex(i));
    double denom =
        m_hnClustersPerPlane->binHeight(m_hnClustersPerPlane->coordToIndex(i));
    double frac = num / denom;
    m_hFractionTrackedClusters->fill(i, frac);

    num = m_hnTracksInterceptCentral->binHeight(
        m_hnTracksInterceptCentral->coordToIndex(i));
    denom = m_hnClustersPerPlaneCentral->binHeight(
        m_hnClustersPerPlaneCentral->coordToIndex(i));
    frac = num / denom;
    m_hRatioTracksClustersCentral->fill(i, frac);
  }

  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    double num = m_nTrackedClusters_vs_telHitOccupancy->binHeight(
        m_nTrackedClusters_vs_telHitOccupancy->coordToIndex(i));
    double denom = m_nClusters_vs_telHitOccupancy->binHeight(
        m_nClusters_vs_telHitOccupancy->coordToIndex(i));
    double frac = num / denom;
    if (denom > 0) m_fractionTrackedClusters_vs_telHitOccupancy->fill(i, frac);

    num = m_nTrackedClusters_vs_telCharge->binHeight(
        m_nTrackedClusters_vs_telCharge->coordToIndex(i));
    denom = m_nClusters_vs_telCharge->binHeight(
        m_nClusters_vs_telCharge->coordToIndex(i));
    frac = num / denom;
    if (denom > 0) m_fractionTrackedClusters_vs_telCharge->fill(i, frac);
  }

  return TbAlgorithm::finalize();
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbTrackingEfficiency::execute() {

  // Grab the tracks.
  LHCb::TbTracks* tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!tracks) return Error("No tracks in " + m_trackLocation);

  for (LHCb::TbTrack* track : *tracks) {
    // Calculate the intercept of the track with the detector planes.
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      const Gaudi::XYZPoint intercept = geomSvc()->intercept(track, i);
      if (intercept.x() > m_parCentral.lowEdge() &&
          intercept.x() < m_parCentral.highEdge() &&
          intercept.y() > m_parCentral.lowEdge() &&
          intercept.y() < m_parCentral.highEdge())
        m_hnTracksInterceptCentral->fill(i);
    }
  }

  // Fill plots that loop over clusters.
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    // Get the clusters for this plane.
    const std::string clusterLocation = m_clusterLocation + std::to_string(i);
    LHCb::TbClusters* clusters = getIfExists<LHCb::TbClusters>(clusterLocation);
    if (!clusters) continue;
    fillClusterLoopPlots(clusters, i);
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Fill cluster loop plots.
//=============================================================================
void TbTrackingEfficiency::fillClusterLoopPlots(const LHCb::TbClusters* clusters,
                                        const unsigned int plane) {
  unsigned int nTrackedClusters = 0;
  unsigned int nClustersCentral = 0;
  for (const LHCb::TbCluster* cluster : *clusters) {
    if (cluster->associated()) ++nTrackedClusters;
    if (cluster->x() > m_parCentral.lowEdge() &&
        cluster->x() < m_parCentral.highEdge() &&
        cluster->y() > m_parCentral.lowEdge() &&
        cluster->y() < m_parCentral.highEdge())
      nClustersCentral++;
  }
  m_hnTrackedClusters->fill(plane, nTrackedClusters);
  m_hnClustersPerPlane->fill(plane, clusters->size());
  m_hnClustersPerPlaneCentral->fill(plane, nClustersCentral);
}

//=============================================================================
// Setup plots
//=============================================================================
void TbTrackingEfficiency::setupPlots() {

  m_hnClustersPerPlane = book1D("TrackingEfficiency/nClustersPerPlane",
                                "nClustersPerPlane", -0.5, 12.5, 13);
  setAxisLabels(m_hnClustersPerPlane, "plane", "clusters");

  m_hnTrackedClusters = book1D("TrackingEfficiency/nTrackedClusters",
                               "nTrackedClusters", -0.5, 12.5, 13);
  setAxisLabels(m_hnTrackedClusters, "plane", "clusters");

  m_hFractionTrackedClusters =
      book1D("TrackingEfficiency/FractionTrackedClusters",
             "FractionTrackedClusters", -0.5, 12.5, 13);
  setAxisLabels(m_hFractionTrackedClusters, "plane", "Fraction");

  m_hnClustersPerPlaneCentral =
      book1D("TrackingEfficiency/nClustersPerPlaneCentral",
             "nClustersPerPlaneCentral", -0.5, 12.5, 13);
  setAxisLabels(m_hnClustersPerPlaneCentral, "plane", "clusters");

  m_hnTracksInterceptCentral =
      book1D("TrackingEfficiency/nTracksInterceptCentral",
             "nTracksInterceptCentral", -0.5, 12.5, 13);
  setAxisLabels(m_hnTracksInterceptCentral, "plane", "tracks");

  m_hRatioTracksClustersCentral =
      book1D("TrackingEfficiency/RatioTracksClustersCentral",
             "RatioTracksClustersCentral", -0.5, 12.5, 13);
  setAxisLabels(m_hRatioTracksClustersCentral, "plane", "Fraction");

  unsigned int nBins = 60;
  m_nTrackedClusters_vs_telHitOccupancy =
      book1D("TrackingEfficiency/nTrackedClusters_vs_telHitOccupancy",
             "nTrackedClusters_vs_telHitOccupancy", 0, 250, nBins);
  m_fractionTrackedClusters_vs_telHitOccupancy =
      book1D("TrackingEfficiency/fractionTrackedClusters_telHitOccupancy",
             "ffractionTrackedClusters_telHitOccupancy", 0, 250, nBins);
  m_nClusters_vs_telHitOccupancy =
      book1D("TrackingEfficiency/nClusters_vs_telHitOccupancy",
             "nClusters_vs_telHitOccupancy", 0, 250, nBins);
  m_telHitOccupancy = book1D("TrackingEfficiency/telHitOccupancy",
                             "telHitOccupancy", 0, 250, nBins);
  m_telHitOccupancy_tracked =
      book1D("TrackingEfficiency/telHitOccupancy_tracked",
             "telHitOccupancy_tracked", 0, 250, nBins);

  m_nTrackedClusters_vs_telCharge =
      book1D("TrackingEfficiency/nTrackedClusters_vs_telCharge",
             "nTrackedClusters_vs_telCharge", 0, 8000, nBins);
  m_fractionTrackedClusters_vs_telCharge =
      book1D("TrackingEfficiency/fractionTrackedClusters_telCharge",
             "ffractionTrackedClusters_telCharge", 0, 8000, nBins);
  m_nClusters_vs_telCharge = book1D("TrackingEfficiency/nClusters_vs_telCharge",
                                    "nClusters_vs_telCharge", 0, 8000, nBins);
  m_telCharge =
      book1D("TrackingEfficiency/telCharge", "telCharge", 0, 8000, nBins);

  m_nTrackedClusters_vs_telClusterOccupancy =
      book1D("TrackingEfficiency/nTrackedClusters_vs_telClusterOccupancy",
             "nTrackedClusters_vs_telClusterOccupancy", 0, 60, nBins);
  m_fractionTrackedClusters_vs_telClusterOccupancy =
      book1D("TrackingEfficiency/fractionTrackedClusters_telClusterOccupancy",
             "ffractionTrackedClusters_telClusterOccupancy", 0, 60, nBins);
  m_nClusters_vs_telClusterOccupancy =
      book1D("TrackingEfficiency/nClusters_vs_telClusterOccupancy",
             "nClusters_vs_telClusterOccupancy", 0, 60, nBins);
  m_telClusterOccupancy = book1D("TrackingEfficiency/telClusterOccupancy",
                                 "telClusterOccupancy", 0, 60, nBins);
  m_telClusterOccupancy_tracked =
      book1D("TrackingEfficiency/telClusterOccupancy_tracked",
             "telClusterOccupancy_tracked", 0, 60, nBins);
}

void TbTrackingEfficiency::fillTrackingEfficiency() {
  std::string clusterLocation = m_clusterLocation + "0";
  LHCb::TbClusters* clusters_zero =
      getIfExists<LHCb::TbClusters>(clusterLocation);

  // Loop over clusters on plane 0.
  LHCb::TbClusters::const_iterator begin = clusters_zero->begin();
  LHCb::TbClusters::const_iterator end = clusters_zero->end();
  for (LHCb::TbClusters::const_iterator iSeed = begin; iSeed != end; ++iSeed) {
    double timeSeed = (*iSeed)->htime();
    double tWindow = 10;

    unsigned int nClusters = 0;
    unsigned int nTrackedClusters = 0;
    unsigned int nTelHits = 0;
    unsigned int nTelHits_tracked = 0;
    double telCharge = 0;

    for (unsigned int i = 1; i < m_nPlanes; ++i) {
      if (i == 4) continue;
      // Get the clusters for this plane.
      clusterLocation = m_clusterLocation + std::to_string(i);
      LHCb::TbClusters* clusters =
          getIfExists<LHCb::TbClusters>(clusterLocation);

      LHCb::TbClusters::const_iterator begin = clusters->begin();
      LHCb::TbClusters::const_iterator end = clusters->end();
      for (LHCb::TbClusters::const_iterator it = begin; it != end; ++it) {
        double delT = timeSeed - (*it)->htime();
        if (fabs(delT) < tWindow) {
          nTelHits += (*it)->size();
          telCharge += (*it)->charge();
          if ((*it)->charge() > m_chargeCutLow) {
            nClusters++;
            if ((*it)->associated()) {
              nTrackedClusters++;
              nTelHits_tracked += (*it)->size();
            }
          }
        }
      }
    }
    //		if (nClustersPerPlane > 4 && m_event == 1050)
    //std::cout<<nClustersPerPlane<<"\t"<<m_event<<"\t"<<(*iSeed)->htime()<<std::endl;
    m_telHitOccupancy->fill(nTelHits);
    m_telHitOccupancy_tracked->fill(nTelHits_tracked);
    m_nClusters_vs_telHitOccupancy->fill(nTelHits, nClusters);
    m_nTrackedClusters_vs_telHitOccupancy->fill(nTelHits, nTrackedClusters);

    m_telCharge->fill(telCharge);
    m_nClusters_vs_telCharge->fill(telCharge, nClusters);
    m_nTrackedClusters_vs_telCharge->fill(telCharge, nTrackedClusters);

    m_telClusterOccupancy->fill(nClusters);
    m_telClusterOccupancy_tracked->fill(nTrackedClusters);
    m_nClusters_vs_telClusterOccupancy->fill(nClusters, nClusters);
    m_nTrackedClusters_vs_telClusterOccupancy->fill(nClusters,
                                                    nTrackedClusters);

  }
}
