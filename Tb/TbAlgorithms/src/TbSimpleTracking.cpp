// Gaudi
#include "GaudiKernel/PhysicalConstants.h"

// Tb/TbKernel
#include "TbKernel/TbFunctors.h"
#include "TbKernel/TbModule.h"

// Local
#include "TbSimpleTracking.h"

DECLARE_ALGORITHM_FACTORY(TbSimpleTracking)

//=============================================================================
// Standard constructor
//=============================================================================
TbSimpleTracking::TbSimpleTracking(const std::string& name,
                                   ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator) {

  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);
  declareProperty("TrackLocation",
                  m_trackLocation = LHCb::TbTrackLocation::Default);
  declareProperty("TrackFitTool", m_trackFitTool = "TbTrackFit");
  declareProperty("TimeWindow", m_timeWindow = 10. * Gaudi::Units::ns);
  declareProperty("MinPlanes", m_nMinPlanes = 8);
  declareProperty("MaxDistance", m_maxDist = 0. * Gaudi::Units::mm);
  declareProperty("MaxOpeningAngle", m_maxAngle = 0.01);
  declareProperty("RecheckTrack", m_recheckTrack = true);
  declareProperty("RemoveOutliers", m_removeOutliers = true);
  declareProperty("ChargeCutLow", m_chargeCutLow = 0);
  declareProperty("MaxClusterSize", m_maxClusterSize = 10);
  declareProperty("MaxClusterWidth", m_maxClusterWidth = 3);
  declareProperty("MaxOccupancy", m_maxOccupancy = 8);
  declareProperty("MaxChi2", m_maxChi2 = 10.);
  declareProperty("DoOccupancyCut", m_doOccupancyCut = false);
  declareProperty("Monitoring", m_monitoring = false);
}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbSimpleTracking::initialize() {
  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;

  m_clusters.resize(m_nPlanes, nullptr);
  // Setup the track fit tool.
  m_trackFit = tool<ITbTrackFit>(m_trackFitTool, "Fitter", this);
  if (!m_trackFit) return Error("Cannot retrieve track fit tool.");

  unsigned int nNonMasked = 0;
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    if (!masked(i)) ++nNonMasked;
  }
  if (nNonMasked < m_nMinPlanes) {
    return Error("Requested " + std::to_string(m_nMinPlanes) + 
                 " planes on  a track but only " + std::to_string(nNonMasked) +
                 " planes are not masked.");
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbSimpleTracking::execute() {

  // Get the clusters on each plane.
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    if (masked(i)) continue;
    const std::string clusterLocation = m_clusterLocation + std::to_string(i);
    LHCb::TbClusters* clusters = getIfExists<LHCb::TbClusters>(clusterLocation);
    if (!clusters) return Error("No clusters in " + clusterLocation);
    m_clusters[i] = clusters;
  }

  if (m_doOccupancyCut) findHighOccupancies();
  unsigned int nKilledOccupancyCut = 0;
  // Check if there is already a track container.
  LHCb::TbTracks* tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!tracks) {
    // Create a track container and transfer its ownership to the TES.
    tracks = new LHCb::TbTracks();
    put(tracks, m_trackLocation);
  }
  LHCb::TbTrack* track = nullptr;
  const unsigned int lastPlane = m_nPlanes - 2;
  for (unsigned int i = 0; i < lastPlane; ++i) {
    if (masked(i)) continue;
    // Get the next valid plane.
    unsigned int j = i + 1;
    while (masked(j) && j <= lastPlane) ++j;
    if (j > lastPlane) break;
    LHCb::TbClusters::iterator first0 = m_clusters[i]->begin();
    LHCb::TbClusters::iterator first1 = m_clusters[j]->begin();
    LHCb::TbClusters::iterator end0 = m_clusters[i]->end();
    LHCb::TbClusters::iterator end1 = m_clusters[j]->end();
    for (LHCb::TbClusters::iterator it0 = first0; it0 != end0; ++it0) {
      // Skip clusters already assigned to tracks.
      if ((*it0)->associated()) continue;
      // Skip clusters with too low ToT.
      if ((*it0)->ToT() < m_chargeCutLow) continue;
      // Skip large clusters.
      if ((*it0)->size() > m_maxClusterSize) continue;
      if (((*it0)->cols() > m_maxClusterWidth) || 
          ((*it0)->rows() > m_maxClusterWidth)) continue;
      const double t0 = (*it0)->htime();
      const double tMin = t0 - m_timeWindow;
      const double tMax = t0 + m_timeWindow;
      // Loop over the hits in the second plane.
      for (LHCb::TbClusters::iterator it1 = first1; it1 != end1; ++it1) {
        const double t1 = (*it1)->htime();
        // Skip hits below the time limit.
        if (t1 < tMin) {
          // Update the search range.
          first1 = it1 + 1;
          continue;
        }
        // Stop search when above the time limit.
        if (t1 > tMax) break;
        // Skip clusters already assigned to tracks.
        if ((*it1)->associated()) continue;
        // Skip clusters with too low charge.
        if ((*it1)->ToT() < m_chargeCutLow) continue;
        // Skip large clusters.
        if ((*it1)->size() > m_maxClusterSize) continue;
        if (((*it1)->cols() > m_maxClusterWidth) || 
            ((*it1)->rows() > m_maxClusterWidth)) continue;
        if (!track)
          track = new LHCb::TbTrack();
        else
          track->clearClusters();
        track->addToClusters(*it0);
        track->addToClusters(*it1);
        if (!extendTrack(track, true)) continue;
        // Fit the track.
        m_trackFit->fit(track);
        if (m_recheckTrack) recheckTrack(track);
        // Check if there are enough measurements on the track.
        if (track->clusters().size() < m_nMinPlanes) continue;
        // Check the track chi2.
        if (track->chi2PerNdof() > m_maxChi2) continue;
        // Check if there are enough unused clusters on the track.
        /*
        unsigned int nUnused = 0;
        for (const LHCb::TbCluster* cluster : track->clusters()) {
          if (!cluster->associated()) ++nUnused;
        }
        if (nUnused < 5) continue;
        */
        if (!lowClusterOccupancy(track->htime())) {
          ++nKilledOccupancyCut;
          continue;
        }
        auto clusters = const_cast<SmartRefVector<LHCb::TbCluster>&>(track->clusters());
        // Sort the clusters by plane.
        std::sort(clusters.begin(), clusters.end(),
                  TbFunctors::LessByZ<const LHCb::TbCluster*>());
        // Tag the clusters as used.
        for (auto itc = clusters.begin(), end = clusters.end(); itc != end;
             ++itc) {
          (*itc)->setAssociated(true);
        }
        // Calculate the global timestamp and add the track to the container.
        track->setTime(timingSvc()->localToGlobal(track->htime()));
        tracks->insert(track);
        track = nullptr;
      }
    }
  }
  if (track) delete track;

  // Sort the tracks by time.
  std::sort(tracks->begin(), tracks->end(),
            TbFunctors::LessByTime<const LHCb::TbTrack*>());
  // Fill the counters.
  counter("Number of tracks") += tracks->size();
  counter("Number of tracks removed by occupancy cut") += nKilledOccupancyCut;
  appendTrackingEfficiencies();

  return StatusCode::SUCCESS;
}

//=============================================================================
// Extrapolate and try to add clusters to a given seed track
//=============================================================================
bool TbSimpleTracking::extendTrack(LHCb::TbTrack* track, const bool fwd) {

  unsigned int nAdded = 0;
  // Count planes without matching clusters.
  unsigned int nMissed = 0;
  const LHCb::TbCluster* c1 = track->clusters().front();
  const LHCb::TbCluster* c2 = track->clusters().back();
  if (!fwd) {
    c2 = c1;
    c1 = track->clusters().at(1);
  }
  double t = c2->htime();
  // Calculate the track slopes based on the first two clusters on the track.
  double td = 1. / (c2->z() - c1->z());
  double tx = (c2->x() - c1->x()) * td;
  double ty = (c2->y() - c1->y()) * td;
  unsigned int plane = c2->plane();
  if (fwd) {
    plane += 1;
  } else {
    plane -= 1;
  }
  while (plane < m_nPlanes) {
    // Calculate the extrapolated position on this plane. 
    const double dz = geomSvc()->modules().at(plane)->z() - c1->z();
    const double xPred = c1->x() + tx * dz;
    const double yPred = c1->y() + ty * dz;
    // Calculate the tolerance window.
    const double tol = fabs(dz * m_maxAngle) + m_maxDist;
    // Try adding clusters on this plane.
    const LHCb::TbCluster* c3 = bestCluster(plane, xPred, yPred, t, tol);
    if (c3) {
      // Add the cluster.
      track->addToClusters(c3);
      ++nAdded;
      // Reset the missed plane counter.
      nMissed = 0;
      // Update the pair of clusters to be used for extrapolating.
      if ((c3->cols() <= m_maxClusterWidth) && 
          (c3->rows() <= m_maxClusterWidth)) {
        c1 = c2;
        c2 = c3;
        // Update the track slopes.
        td = 1. / (c2->z() - c1->z());
        tx = (c2->x() - c1->x()) * td;
        ty = (c2->y() - c1->y()) * td;
      }
      // Update the predicted timestamp.
      t = c3->htime();
    } else {
      // No matching cluster.
      ++nMissed;
      if (nMissed > 1) break;
    }
    if (fwd) {
      ++plane;
    } else {
      if (plane == 0) break;
      --plane;
    }
  }
  return nAdded > 0;
}

//=============================================================================
// Get the closest cluster to a given point on the plane
//=============================================================================
const LHCb::TbCluster* TbSimpleTracking::bestCluster(const unsigned int plane, 
    const double xPred, const double yPred,
    const double tPred, const double tol) {
  if (masked(plane) || m_clusters[plane]->empty()) return nullptr;
  // Get the first cluster within the time window.
  const double tMin = tPred - m_timeWindow;
  LHCb::TbClusters::iterator end = m_clusters[plane]->end();
  LHCb::TbClusters::iterator it =
      std::lower_bound(m_clusters[plane]->begin(), end, tMin, lowerBound());
  if (it == end) return nullptr;
  const double tMax = tPred + m_timeWindow;
  LHCb::TbCluster* best = nullptr;
  double dtBest = m_timeWindow;
  for (; it != end; ++it) {
    const double t = (*it)->htime();
    if ((*it)->ToT() < m_chargeCutLow) continue;
    if ((*it)->size() > m_maxClusterSize) continue;
    // Stop searching when outside the time window.
    if (t > tMax) break;
    const double dt = fabs(t - tPred);
    if (m_monitoring) {
      const std::string title = "Plane" + std::to_string(plane);
      plot(t - tPred, "delT" + title, title, -100, 100, 200);
    }
    if (dt < dtBest) {
      // Check if the cluster is within the spatial tolerance window.
      const double dx = xPred - (*it)->x();
      const double dy = yPred - (*it)->y();
      if (m_monitoring) {
        const std::string title = "Plane " + std::to_string(plane); 
        plot(dx, "delXY/" + title, title, -3, 3, 200);
        plot(dy, "delXY/" + title, title, -3, 3, 200);
      }
      if (fabs(dx) > tol || fabs(dy) > tol) continue;
      // Update the best cluster.
      dtBest = dt;
      best = *it;
    }
  }
  return best;
}

//=============================================================================
// Remove outliers and try to find additional clusters on a track candidate.
//=============================================================================
void TbSimpleTracking::recheckTrack(LHCb::TbTrack* track) {

  // Spatial window for adding clusters to the track.
  // TODO: make this a parameter instead of hardcoding it.
  const double tol = 0.1;
  // Keep track of the planes that already have a cluster on the track.
  std::vector<bool> planeAssociated(m_nPlanes, false);
  const unsigned int nSizeBefore = track->size();
  for (auto& node : track->nodes()) {
    const unsigned int plane = node.plane();
    // Remove outliers if requested.
    if (m_removeOutliers && 
        (fabs(node.residualX()) > tol || fabs(node.residualY()) > tol)) {
      track->removeFromClusters(node.cluster());
    } else {
      planeAssociated[plane] = true;
    }
  }
  const unsigned int nSizeAfter = track->size();
  if (m_monitoring) {
    const std::string title = "NOutliersRemoved";
    plot(nSizeBefore - nSizeAfter, title, title, -0.5, 10.5, 11);
  }
  if (nSizeAfter < 3) return;
  // Refit the track after outlier removal.
  if (nSizeAfter != nSizeBefore) m_trackFit->fit(track);
  // Try to add clusters on planes that don't have a measurement yet.
  for (unsigned int i = m_nPlanes; i-- > 0;) {
    if (masked(i) || planeAssociated[i]) continue;
    // Calculate the track intercept on the plane.
    const Gaudi::XYZPoint interceptG = geomSvc()->intercept(track, i);
    const LHCb::TbCluster* cluster = bestCluster(
        i, interceptG.x(), interceptG.y(), track->htime(), tol);
    if (cluster) track->addToClusters(cluster);
  }
  if (track->size() > nSizeAfter) m_trackFit->fit(track);
}

//=============================================================================
// Find time windows with high occupancy.
//=============================================================================
void TbSimpleTracking::findHighOccupancies() {

  m_htimesHighOccupancy.clear();
  // Find the first non-masked plane.
  unsigned int seedPlane = 0;
  while (masked(seedPlane)) ++seedPlane;
  for (const LHCb::TbCluster* seed : *m_clusters[seedPlane]) {
    const double tSeed = seed->htime();
    unsigned int nClustersPerSeed = 0;
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      if (masked(i)) continue;
      for (const LHCb::TbCluster* cluster : *m_clusters[i]) {
        if (fabs(tSeed - cluster->htime()) < m_timeWindow) ++nClustersPerSeed;
      }
    }
    if (nClustersPerSeed > m_maxOccupancy)
      m_htimesHighOccupancy.push_back(tSeed);
  }
}

//=============================================================================

bool TbSimpleTracking::lowClusterOccupancy(const double t) const {
  if (!m_doOccupancyCut) return true;
  for (unsigned int i = 0; i < m_htimesHighOccupancy.size(); ++i) {
    if (fabs(t - m_htimesHighOccupancy[i]) < m_timeWindow) return false;
  }
  return true;
}

//=============================================================================

void TbSimpleTracking::appendTrackingEfficiencies() {
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    if (masked(i)) continue;
    for (LHCb::TbClusters::const_iterator it = m_clusters[i]->begin();
         it != m_clusters[i]->end(); ++it) {
      counter("nClusters")++;
      if ((*it)->size() <= m_maxClusterSize &&
          (*it)->ToT() >= m_chargeCutLow &&
          lowClusterOccupancy((*it)->htime())) {
        counter("nClusters selected for tracking")++;
        if ((*it)->associated()) counter("nClusters associated")++;
      }
    }
  }
}

//=============================================================================
