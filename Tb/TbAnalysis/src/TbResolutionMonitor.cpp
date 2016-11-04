// Gaudi
#include "GaudiKernel/PhysicalConstants.h"

// Tb/TbEvent
#include "Event/TbTrack.h"
#include "Event/TbCluster.h"

// Tb/TbKernel
#include "TbKernel/TbConstants.h"

// Local
#include "TbResolutionMonitor.h"

DECLARE_ALGORITHM_FACTORY(TbResolutionMonitor)

//=============================================================================
// Standard constructor
//=============================================================================
TbResolutionMonitor::TbResolutionMonitor(const std::string& name, 
                                         ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator) {

  declareProperty("TrackLocation",
                  m_trackLocation = LHCb::TbTrackLocation::Default);
  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);

  declareProperty("DUTs", m_duts);
}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbResolutionMonitor::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbResolutionMonitor::execute() {

  // Grab the tracks.
  const LHCb::TbTracks* tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!tracks) {
    error() << "No tracks in " << m_trackLocation << endmsg;
    return StatusCode::FAILURE;
  }

  for (const auto dut : m_duts) {
    // Get the clusters for this plane.
    const std::string clusterLocation = m_clusterLocation + std::to_string(dut);
    const LHCb::TbClusters* clusters = getIfExists<LHCb::TbClusters>(clusterLocation);
    for (const LHCb::TbCluster* cluster : *clusters) {
      plot(cluster->charge(), "ChargeAll", "ChargeAll", 0., 50000., 200);
    }
    if (!clusters) continue;
    // Loop over the tracks.
    for (const LHCb::TbTrack* track : *tracks) {
      // Cut on track quality.
      if (track->chi2PerNdof() > 20.) continue;
      const Gaudi::XYZPoint pGlobal = geomSvc()->intercept(track, dut);
      const auto pLocal = geomSvc()->globalToLocal(pGlobal, dut);
      if (pLocal.x() < 0. || pLocal.y() < 0.) continue;
      const double fcol = pLocal.x() / Tb::PixelPitch;
      const double frow = pLocal.y() / Tb::PixelPitch;
      const int col = int(fcol);
      const int row = int(frow);
      if (col > 255 || row > 255) continue;
      // Calculate inter-pixel coordinates.
      const double xCell = 1.e3 * (fcol - col) * Tb::PixelPitch;
      const double yCell = 1.e3 * (frow - row) * Tb::PixelPitch;
      const LHCb::TbCluster* match = NULL;
      for (const LHCb::TbCluster* cluster : *clusters) {
        const double dt = cluster->htime() - track->htime();
        if (fabs(dt) > 200.) continue;
        const double dx = cluster->xloc() - pLocal.x();
        const double dy = cluster->yloc() - pLocal.y();
        if (fabs(dx) < 0.15 && fabs(dy) < 0.15) {
          match = cluster;
          break;
        }
      }
      plot2D(xCell, yCell, "NTracks", "NTracks",
             0., 55., 0., 55., 9, 9);
      if (!match) continue;
      plot2D(xCell, yCell, "NClusters", "NClusters",
             0., 55., 0., 55., 9, 9);
      plot(match->x() - pGlobal.x(), "ResGlobalX", "ResGlobalX", -0.2, 0.2, 100);
      plot(match->y() - pGlobal.y(), "ResGlobalY", "ResGlobalY", -0.2, 0.2, 100);
      plot(match->xloc() - pLocal.x(), "ResLocalX", "ResLocalX", -0.2, 0.2, 100);
      plot(match->yloc() - pLocal.y(), "ResLocalY", "ResLocalX", -0.2, 0.2, 100);
      plot(match->htime() - track->htime(), "ResTime", "ResTime", -200., 200., 400); 
      const unsigned int cls = match->size();
      if (cls > 4) continue;
      const std::string title = "InterceptCls" + std::to_string(cls);
      plot2D(xCell, yCell, title, title,
             0., 55., 0., 55., 50, 50);
    }
  }
  return StatusCode::SUCCESS;
}

