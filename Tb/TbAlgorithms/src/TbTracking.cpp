#include <fstream>
#include <algorithm>

// Gaudi
#include "GaudiKernel/PhysicalConstants.h"

// Tb/TbKernel
#include "TbKernel/TbConstants.h"
#include "TbKernel/TbFunctors.h"

// Local
#include "TbTracking.h"

DECLARE_ALGORITHM_FACTORY(TbTracking)

//=============================================================================
// Standard constructor
//=============================================================================
TbTracking::TbTracking(const std::string& name, ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator),
      m_tracks(nullptr),
      m_trackFit(nullptr),
      m_clusterFinder(nullptr),
      m_trackVolume(nullptr),
      m_event(0) {

  // Declare algorithm properties - see header for description.
  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);
  declareProperty("TrackLocation",
                  m_trackLocation = LHCb::TbTrackLocation::Default);
  declareProperty("TrackFitTool", m_trackFitTool = "TbTrackFit");
  declareProperty("Monitoring", m_monitoring = false);
  declareProperty("TimeWindow", m_twindow = 150. * Gaudi::Units::ns);
  declareProperty("MinNClusters", m_MinNClusters = 7);
  declareProperty("SearchRadius", m_vol_radius = 1 * Gaudi::Units::mm);
  declareProperty("MaxChi2", m_ChiSqRedCut = 20000.);
  declareProperty("VolumeAngle", m_vol_theta = 0.015);
  declareProperty("SearchPlanes", m_PlaneSearchOrder = {4, 3, 5});
  declareProperty("ClusterSizeCut", m_clusterSizeCut = 1000);
  declareProperty("CombatRun", m_combatRun = false);

  // Options for the event viewer.
  declareProperty("ViewerOutput", m_viewerOutput = false);
  declareProperty("ViewerEventNum", m_viewerEvent = 100);

  // Default values.
  m_search_3vol = "diabolo";
  m_ClusterFinderSearchAlgorithm = "adap_seq";
  m_nComboCut = 300;
}

//=============================================================================
// Destructor
//=============================================================================
TbTracking::~TbTracking() {

  if (m_trackVolume) {
    delete m_trackVolume;
  }
}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbTracking::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  // Setup the track fit tool.
  m_trackFit = tool<ITbTrackFit>(m_trackFitTool, "Fitter", this);
  if (!m_trackFit) return Error("Cannot retrieve track fit tool.");
  // Set up the cluster finder.
  m_clusterFinder =
      tool<ITbClusterFinder>("TbClusterFinder", "ClusterFinder", this);
  if (!m_clusterFinder) return Error("Cannot retrieve cluster finder tool.");
  m_clusterFinder->setSearchAlgorithm(m_ClusterFinderSearchAlgorithm);
  m_volumed.resize(m_nPlanes);
  // Set up the track volume.
  m_trackVolume =
      new TbTrackVolume(m_search_3vol, m_nPlanes, m_vol_radius, m_vol_radius,
                        m_vol_theta, m_vol_theta, m_MinNClusters);
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbTracking::execute() {
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    if (m_event == m_viewerEvent && m_viewerOutput) {
      std::ofstream myfile;
      myfile.open("/afs/cern.ch/user/d/dsaunder/KeplerViewerData.dat");
      myfile << "# Output\n";
      myfile.close();
    }

    const std::string clusterLocation = m_clusterLocation + std::to_string(i);
    LHCb::TbClusters* clusters = getIfExists<LHCb::TbClusters>(clusterLocation);
    if (!clusters) {
      return Error("No clusters in " + clusterLocation);
    }
    // Store the cluster iterators in the cluster finder.
    m_clusterFinder->setClusters(clusters, i);
    m_volumed[i].clear();
    m_volumed[i].resize(clusters->size(), false);
  }

  // Check if there is already a track container.
  m_tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!m_tracks) {
    // Create a track container and transfer its ownership to the TES.
    m_tracks = new LHCb::TbTracks();
    put(m_tracks, m_trackLocation);
  }
  // Do the tracking and time order.
  performTracking();
  timeOrderTracks();

  counter("NumberOfTracks") += m_tracks->size();
  if (m_event == m_viewerEvent && m_viewerOutput) outputViewerData();
  m_event++;
  return StatusCode::SUCCESS;
}

//=============================================================================
// Tracking
//=============================================================================
void TbTracking::performTracking() {
  // The working of this algorithm is similar to the cluster maker, such that:
  // - Loop over some set of seed clusters (those on the m_SeedPlanes).
  // - Create a container (TbTrackVolume) centered on each seed cluster
  //   (in space and time).
  // - Impose the condition that any formed track must contain this seed.
  // - Evaluate that 4-volume, to see if it contained a track.
  // - If a choice (e.g. more than one possible combination of clusters),
  //   take the best fitting. Priority is given to more complete tracks.
  // - If another track happened to be in the 4volume, but not
  //   containing this seed, then it will be found later.

  // Iterate over planes in the specified order.
  for (const auto& plane : m_PlaneSearchOrder) {
    // Skip masked and empty planes.
    if (masked(plane) || m_clusterFinder->empty(plane)) continue;
    // Iterate over the clusters on this plane.
    const auto end = m_clusterFinder->end(plane);
    for (auto ic = m_clusterFinder->first(plane); ic != end; ++ic) {
      // Check if the seed has already been used.
      if ((*ic)->associated() || (*ic)->size() > m_clusterSizeCut) continue;
      // Initialise the track volume container and find clusters
      // that fall in its space-time volume.
      fillTrackVolume(*ic, 0, m_nPlanes);
      // Tag the clusters as "volumed".
      for (unsigned int i = 0; i < m_trackVolume->m_clusters.size(); ++i) {
        for (const auto cl : m_trackVolume->m_clusters[i]) {
          m_volumed[cl->plane()][cl->key()] = true;
        }
      }
      // Look for a track - automatically keeps if suitable.
      evaluateTrackVolume(m_trackVolume);
    }
  }
}

//=============================================================================
// Fill the given track volume with clusters in its space-time volume.
//=============================================================================
void TbTracking::fillTrackVolume(LHCb::TbCluster* seed,
                                 const unsigned int& planeLow,
                                 const unsigned int& planeUp) {

  m_trackVolume->reset(seed);
  const double tMin = seed->htime() - 0.5 * m_twindow;
  const double tMax = seed->htime() + 0.5 * m_twindow;
  for (unsigned int i = planeLow; i < planeUp; ++i) {
    // Skip empty planes, masked planes and the seed plane.
    if (m_clusterFinder->empty(i) || i == seed->plane() || masked(i)) {
      continue;
    }
    // Start with the first cluster in the time window
    // and loop until the end of the time window.
    const auto end = m_clusterFinder->end(i);
    for (auto ic = m_clusterFinder->getIterator(tMin, i); ic != end; ++ic) {
      if ((*ic)->htime() < tMin) continue;
      if ((*ic)->htime() > tMax) break;
      // Skip clusters which are already used or exceed the cluster size cut.
      if (!(*ic)->associated() && (*ic)->size() <= m_clusterSizeCut) {
        m_trackVolume->consider_cluster(*ic);
      }
    }
  }
  if (m_event == m_viewerEvent && m_viewerOutput) {
    std::ofstream file;
    file.open("KeplerViewerData.dat", std::ofstream::app);
    file << "SqTrackArea " << m_trackVolume << tMin << " " << tMax << "\n";
  }
}

//=============================================================================
// Finding the best tracks
//=============================================================================
void TbTracking::evaluateTrackVolume(TbTrackVolume* track_volume) {

  LHCb::TbTrack* trialTrack = NULL;
  LHCb::TbTrack* best_track = NULL;
  double best_chi_dof = -1.;
  // Get the number of combinations to check (depends on the size of the track
  // to be formed).
  const unsigned int ncombos = std::min(track_volume->nCombos(), m_nComboCut);
  // Do the search over this number of combinations - the TbTrackVolume has
  // method to retrieve a particular combination of clusters forming a track.
  for (unsigned int i = 0; i < ncombos; ++i) {
    // Create or reset the trial track.
    if (!trialTrack) {
      trialTrack = new LHCb::TbTrack();
    } else {
      trialTrack->clearClusters();
    }
    // Get the ith track combination.
    track_volume->get_track_combo(trialTrack);
    // Sort the clusters by z-position.
    SmartRefVector<LHCb::TbCluster>& clusters =
        const_cast<SmartRefVector<LHCb::TbCluster>&>(trialTrack->clusters());
    std::sort(clusters.begin(), clusters.end(),
              TbFunctors::LessByZ<const LHCb::TbCluster*>());
    // Evaluate this track.
    m_trackFit->fit(trialTrack);
    const double chi2_per_dof = trialTrack->chi2PerNdof();
    if (i == 0) {
      // First combination tried case.
      best_chi_dof = chi2_per_dof;
      best_track = trialTrack;
      trialTrack = NULL;
    } else if (chi2_per_dof < best_chi_dof) {
      // Case of better combination.
      delete best_track;
      best_chi_dof = chi2_per_dof;
      best_track = trialTrack;
      trialTrack = NULL;
    }
    // Prepare for next combination.
    track_volume->increment_combo_counters();
  }

  if (m_monitoring) fillTrackVolPlots(track_volume);
  if (!best_track) return;

  // If the best chi2 is too high, consider popping one cluster.
  if (best_track->clusters().size() > m_MinNClusters + 1 &&
      best_chi_dof > m_ChiSqRedCut) {
    int popID = -1;
    for (unsigned int i = 0; i < m_nPlanes; i++) {
      m_trackFit->maskPlane(i);
      m_trackFit->fit(best_track);
      if (best_track->chi2PerNdof() < best_chi_dof) {
        best_chi_dof = best_track->chi2PerNdof();
        popID = i;
      }
      m_trackFit->unmaskPlane(i);
    }
    if (popID != -1) {
      for (auto cluster : best_track->clusters()) {
        if (int(cluster->plane()) == popID) {
          best_track->removeFromClusters(cluster);
        }
      }
    }
    m_trackFit->fit(best_track);
    counter("NumberOfPopRecoveredTracks") += 1;
  }

  // At this point, found the best fitting track in the volume.
  // Apply chi2 cut, and save.
  if (best_chi_dof > m_ChiSqRedCut) {
    counter("NumberOfChiRejectedVols") += 1;
    delete best_track;
    return;
  }

  SmartRefVector<LHCb::TbCluster>& clusters =
      const_cast<SmartRefVector<LHCb::TbCluster>&>(best_track->clusters());
  auto earliest_hit =
      std::min_element(clusters.begin(), clusters.end(),
                       TbFunctors::LessByTime<const LHCb::TbCluster*>());

  if (timingSvc()->beforeOverlap((*earliest_hit)->time()) || m_combatRun) {
    for (auto itc = clusters.begin(), end = clusters.end(); itc != end; ++itc) {
      (*itc)->setAssociated(true);
    }
    m_tracks->insert(best_track);
    best_track->setTime(timingSvc()->localToGlobal(best_track->htime()));
    if (m_monitoring) fillTrackVolPlots(track_volume);
  } else {
    info() << "Overlapping track" << endmsg;
    delete best_track;
  }
}

//=============================================================================
// Monitoring plots
//=============================================================================
void TbTracking::fillTrackVolPlots(TbTrackVolume* vol) {
  plot(vol->nCombos(), "nComboDist_of_TrackVolumes",
       "nComboDist_of_TrackVolumes", -0.5, 1099.5, 1100);
  unsigned int nclusters = 0;
  for (unsigned int i = 0; i < vol->m_clusters.size(); i++) {
    nclusters += vol->m_clusters[i].size();
    if (vol->m_clusters[i].size() > 0) {
      const std::string plane = std::to_string(vol->m_clusters[i][0]->plane());
      plot(vol->m_clusters[i].size(),
           "nVolClusters/nClusterPerVolPlane" + plane,
           "nClusterPerVolPlane" + plane, 0.5, 10.5, 10);
    }
  }
  plot(nclusters, "nVolClusters/nCluster_in_TrackVolumes",
       "nCluster_in_TrackVolumes", -0.5, 99.5, 100);
}

//=============================================================================
// Track time ordering - honeycomb
//=============================================================================
void TbTracking::timeOrderTracks() {

  const double s_factor = 1.3;
  LHCb::TbTrack* track1;
  LHCb::TbTrack* track2;
  unsigned int gap = m_tracks->size() / s_factor;
  bool swapped = false;

  // Start the swap loops.
  while (gap > 1 || swapped) {
    if (gap > 1) gap /= s_factor;
    swapped = false;  // Reset per swap loop.

    // Do the swaps.
    LHCb::TbTracks::iterator itt;
    for (itt = m_tracks->begin(); itt < m_tracks->end() - gap; ++itt) {
      track1 = *itt;
      track2 = *(itt + gap);
      if (track1->time() > track2->time()) {
        // Do the swap.
        std::iter_swap(itt, itt + gap);
        swapped = true;
      }
    }
  }
}

//=============================================================================
// Viewer output
//=============================================================================
void TbTracking::outputViewerData() {
  std::ofstream myfile;
  myfile.open("KeplerViewerData.dat", std::ofstream::app);

  // First output the chips.
  for (unsigned int i = 0; i < m_nPlanes; i++) {
    myfile << "Chip ";
    Gaudi::XYZPoint posn1(0., 14.08, 0.);
    Gaudi::XYZPoint posn = geomSvc()->localToGlobal(posn1, i);
    myfile << posn.x() << " " << posn.y() << " " << posn.z() << " ";

    Gaudi::XYZPoint posn2(14.08, 14.08, 0.);
    posn = geomSvc()->localToGlobal(posn2, i);
    myfile << posn.x() << " " << posn.y() << " " << posn.z() << " ";

    Gaudi::XYZPoint posn3(14.08, 0., 0.);
    posn = geomSvc()->localToGlobal(posn3, i);
    myfile << posn.x() << " " << posn.y() << " " << posn.z() << " ";

    Gaudi::XYZPoint posn4(0., 0., 0.);
    posn = geomSvc()->localToGlobal(posn4, i);
    myfile << posn.x() << " " << posn.y() << " " << posn.z() << " ";

    myfile << "\n";
  }

  // Tracks.
  for (const LHCb::TbTrack* track : *m_tracks) {
    myfile << "Track ";
    myfile << track->firstState().tx() << " " << track->firstState().x() << " "
           << track->firstState().ty() << " " << track->firstState().y() << " "
           << track->htime() << "\n";
  }

  // Clusters.
  for (unsigned int i = 0; i < m_nPlanes; i++) {
    auto ic = m_clusterFinder->first(i);
    const auto end = m_clusterFinder->end(i);
    for (; ic != end; ++ic) {
      const int tag = m_volumed[i][(*ic)->key()] + (*ic)->associated();
      myfile << "Cluster ";
      myfile << (*ic)->x() << " " << (*ic)->y() << " " << (*ic)->z() << " "
             << (*ic)->htime() << " " << tag << " \n";
      // Its hits.
      for (const auto hit : (*ic)->hits()) {
        myfile << "Pixel ";
        double xLocal = 0.;
        double yLocal = 0.;
        geomSvc()->pixelToPoint(hit->scol(), hit->row(), i, xLocal, yLocal);
        Gaudi::XYZPoint pLocal(xLocal - 0.5 * Tb::PixelPitch,
                               yLocal - 0.5 * Tb::PixelPitch, 0.);
        Gaudi::XYZPoint posn = geomSvc()->localToGlobal(pLocal, i);
        myfile << posn.x() << " " << posn.y() << " " << posn.z() << " ";
        Gaudi::XYZPoint posn2(pLocal.x() + Tb::PixelPitch, pLocal.y(), 0.);
        posn = geomSvc()->localToGlobal(posn2, i);
        myfile << posn.x() << " " << posn.y() << " " << posn.z() << " ";
        Gaudi::XYZPoint posn3(pLocal.x() + Tb::PixelPitch,
                              pLocal.y() + Tb::PixelPitch, 0.);
        posn = geomSvc()->localToGlobal(posn3, i);
        myfile << posn.x() << " " << posn.y() << " " << posn.z() << " ";
        Gaudi::XYZPoint posn4(pLocal.x(), pLocal.y() + Tb::PixelPitch, 0.);
        posn = geomSvc()->localToGlobal(posn4, i);
        myfile << posn.x() << " " << posn.y() << " " << posn.z() << " ";
        myfile << hit->htime() << " " << hit->ToT() << "\n";
      }
    }
  }
  myfile.close();
}
