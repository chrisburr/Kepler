// Gaudi
#include "GaudiKernel/PhysicalConstants.h"

// Tb/TbEvent
#include "Event/TbTrack.h"
#include "Event/TbCluster.h"

// Tb/TbKernel
#include "TbKernel/TbConstants.h"

// Local
#include "TbClusterAssociator.h"

DECLARE_ALGORITHM_FACTORY(TbClusterAssociator)

//=============================================================================
// Standard constructor
//=============================================================================
TbClusterAssociator::TbClusterAssociator(const std::string& name,
                                         ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator) {

  declareProperty("TrackLocation",
                  m_trackLocation = LHCb::TbTrackLocation::Default);
  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);

  declareProperty("DUTs", m_duts);

  declareProperty("UseHits", m_useHits = true);
  declareProperty("ReuseClusters", m_reuseClusters = true);
  declareProperty("TimeWindow", m_twindow = 200. * Gaudi::Units::ns);
  declareProperty("XWindow", m_xwindow = 1. * Gaudi::Units::mm);
  declareProperty("MaxChi2", m_maxChi2 = 100.);
}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbClusterAssociator::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbClusterAssociator::execute() {

  // Grab the tracks.
  const LHCb::TbTracks* tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!tracks) {
    return Error("No tracks in " + m_trackLocation);
  }

  for (const auto dut : m_duts) {
    // Get the clusters for this plane.
    const std::string clusterLocation = m_clusterLocation + std::to_string(dut);
    const LHCb::TbClusters* clusters =
        getIfExists<LHCb::TbClusters>(clusterLocation);
    if (!clusters) continue;
    // Loop over the tracks.
    for (LHCb::TbTrack* track : *tracks) {
      // Skip low quality tracks.
      if (track->chi2PerNdof() > m_maxChi2) continue;
      const Gaudi::XYZPoint pGlobal = geomSvc()->intercept(track, dut);
      const auto pLocal = geomSvc()->globalToLocal(pGlobal, dut);
      const double tMin = track->htime() - m_twindow;
      const double tMax = track->htime() + m_twindow;
      for (LHCb::TbCluster* cluster : *clusters) {
        // Assume that the clusters are sorted by time.
        if (cluster->htime() < tMin) continue;
        if (cluster->htime() > tMax) break;
        // Skip used clusters (if requested).
        if (!m_reuseClusters && cluster->associated()) continue;
        if (m_useHits) {
          // Compare the coordinates of the pixels to the track intercept.
          if (!match(cluster, pLocal.x(), pLocal.y())) continue;
        } else {
          // Compare the cluster coordinates to the track intercept.
          const double dx = cluster->xloc() - pLocal.x();
          const double dy = cluster->yloc() - pLocal.y();
          if (fabs(dx) > m_xwindow || fabs(dy) > m_xwindow) continue;
        }
        // Associate the cluster to the track and tag it as used.
        track->addToAssociatedClusters(cluster);
        cluster->setAssociated(true);
      }
    }
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Check if the cluster has hits within the search window.
//=============================================================================
bool TbClusterAssociator::match(const LHCb::TbCluster* cluster, const double x,
                                const double y) const {

  const unsigned int plane = cluster->plane();
  // Loop over hits in the cluster.
  for (auto hit : cluster->hits()) {
    double xLocal = 0.;
    double yLocal = 0.;
    geomSvc()->pixelToPoint(hit->scol(), hit->row(), plane, xLocal, yLocal);
    const double dx = xLocal - x;
    const double dy = yLocal - y;
    if (fabs(dx) < m_xwindow && fabs(dy) < m_xwindow) return true;
  }
  return false;
}
