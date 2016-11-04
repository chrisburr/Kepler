#include <algorithm>
#include <fstream>

// Gaudi
#include "GaudiKernel/PhysicalConstants.h"

// Tb/TbKernel
#include "TbKernel/TbFunctors.h"
#include "TbKernel/TbConstants.h"

// Local
#include "TbVertexTracking.h"

DECLARE_ALGORITHM_FACTORY(TbVertexTracking)

//=============================================================================
// Standard constructor
//=============================================================================
TbVertexTracking::TbVertexTracking(const std::string& name,
                                   ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator),
      m_tracks(nullptr),
      m_trackFit(nullptr),
      m_clusterFinder(nullptr),
      m_event(0) {

  // Declare algorithm properties - see header for description.
  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);
  declareProperty("TrackLocation",
                  m_trackLocation = LHCb::TbTrackLocation::Default);
  declareProperty("TrackFitTool", m_trackFitTool = "TbTrackFit");

  declareProperty("TimeWindow", m_twindow = 150. * Gaudi::Units::ns);
  declareProperty("MinNClusters", m_MinNClusters = 7);
  declareProperty("SearchVolumeFillAlgorithm",
                  m_ClusterFinderSearchAlgorithm = "adap_seq");
  declareProperty("SearchPlanes", m_PlaneSearchOrder = {4, 3, 5});
  declareProperty("ClusterSizeCut", m_clusterSizeCut = 1000);
  declareProperty("MinNClustersRepeat", m_MinNClustersRepeat = 3);
  declareProperty("RadialCut", m_radialCut = 0.2);

  declareProperty("ViewerOutput", m_viewerOutput = false);
  declareProperty("ViewerEventNum", m_viewerEvent = 100);
  declareProperty("CombatRun", m_combatRun = false);
  declareProperty("MaxChi2", m_ChiSqRedCut = 200.);
  declareProperty("DoVertexting", m_doVertexting = false);
  declareProperty("DoRepeat", m_doRepeat = true);
  declareProperty("AngleCut", m_angleCut = 0.2);

  declareProperty("VertexDelR", m_vertexDelR = 0.1);
  declareProperty("VertexDelT", m_vertexDelT = 10);
  m_currentAngleCut = m_angleCut;
}

//=============================================================================
// Destructor
//=============================================================================
TbVertexTracking::~TbVertexTracking() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbVertexTracking::initialize() {

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
  m_endCluster.resize(m_nPlanes);
  m_vertexedCluster.resize(m_nPlanes);
  m_volumed.resize(m_nPlanes);
  initialStateVsFitStateTx =
      book2D("initialStateVsFitStateTx", "initialStateVsFitStateTx", -0.001,
             0.001, 200, -0.001, 0.001, 200);
  initialStateVsFitStateTy =
      book2D("initialStateVsFitStateTy", "initialStateVsFitStateTy", -0.001,
             0.001, 200, -0.001, 0.001, 200);
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbVertexTracking::execute() {
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
      error() << "No clusters in " << clusterLocation << endmsg;
      return StatusCode::FAILURE;
    }
    m_endCluster[i].clear();
    m_endCluster[i].resize(clusters->size(), false);
    m_vertexedCluster[i].clear();
    m_vertexedCluster[i].resize(clusters->size(), false);
    m_volumed[i].clear();
    m_volumed[i].resize(clusters->size(), false);
    // Store the cluster iterators in the cluster finder.
    m_clusterFinder->setClusters(clusters, i);
  }

  // Check if there is already a track container.
  m_tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!m_tracks) {
    // Create a track container and transfer its ownership to the TES.
    m_tracks = new LHCb::TbTracks();
    put(m_tracks, m_trackLocation);
  }
  m_vertices = new LHCb::TbVertices();
  put(m_vertices, LHCb::TbVertexLocation::Default);

  // Advantagous to filter out the thin tracks.
  m_currentAngleCut = 0.01;
  performVertexTracking();
  m_currentAngleCut = m_angleCut;
  performVertexTracking();

  if (m_doRepeat) {
    // Allow lower demands on track size.
    unsigned int tempSizeHolder = m_MinNClusters;
    m_MinNClusters = m_MinNClustersRepeat;
    performVertexTracking();
    m_MinNClusters = tempSizeHolder;
  }

  timeOrderTracks();

  if (m_doVertexting) collectIntoVertices();
  /*
  LHCb::TbTracks::iterator itrack;
  for (itrack = m_tracks->begin(); itrack < m_tracks->end() - 1; itrack++) {
    plot((*(itrack + 1))->htime() - (*itrack)->htime(), "TimeBetweenTracks",
         "TimeBetweenTracks", 0.0, 5000, 500);
  }
  */
  counter("NumberOfTracks") += m_tracks->size();
  counter("NumberOfVertices") += m_vertices->size();
  if (m_event == m_viewerEvent && m_viewerOutput) outputViewerData();
  m_event++;
  return StatusCode::SUCCESS;
}

//=============================================================================
// Initialization
//=============================================================================
void TbVertexTracking::collectIntoVertices() {
  // Iterate over tracks.
  LHCb::TbVertex* tempVertex = nullptr;
  LHCb::TbTracks::iterator itrack;
  for (itrack = m_tracks->begin(); itrack < m_tracks->end(); itrack++) {
    tempVertex = nullptr;
    if ((*itrack)->vertexed()) continue;
    LHCb::TbTracks::iterator jtrack;
    for (jtrack = itrack; jtrack < m_tracks->end(); jtrack++) {
      if ((*jtrack)->vertexed() || itrack == jtrack) continue;

      // Check time difference.
      if (fabs((*itrack)->htime() - (*jtrack)->htime()) < m_vertexDelT) {
        // Assume decay happened near a detector.
        for (unsigned int iplane = 0; iplane < m_nPlanes; iplane++) {
          // Work out the spatial separation.
          Gaudi::XYZPoint intercept1 = geomSvc()->intercept((*itrack), iplane);
          Gaudi::XYZPoint intercept2 = geomSvc()->intercept((*jtrack), iplane);
          double delr2 = pow(intercept1.x() - intercept2.x(), 2);
          delr2 += pow(intercept1.y() - intercept2.y(), 2);
          if (pow(delr2, 0.5) < m_vertexDelR) {

            // We have a vertex, although should also add some cluster counting.
            if (tempVertex == nullptr) {
              tempVertex = new LHCb::TbVertex();
              tempVertex->addToTracks(*itrack);
              (*itrack)->setVertexed(true);
              tempVertex->setX(intercept1.x());
              tempVertex->setY(intercept1.y());
              tempVertex->setZ(intercept1.z());
              tempVertex->setHtime((*itrack)->htime());
              tempVertex->setInteractionPlane(iplane);

              SmartRefVector<LHCb::TbCluster>& clusters =
                  const_cast<SmartRefVector<LHCb::TbCluster>&>(
                      (*itrack)->clusters());
              for (unsigned int i = 0; i < clusters.size(); i++) {
                m_vertexedCluster[clusters[i]->plane()][clusters[i]->key()] =
                    true;
                unsigned int plane = clusters[i]->plane();
                if (plane == 0 || plane == 1 || plane == 2)
                  (*itrack)->setParentVertex(true);
              }
            }

            tempVertex->addToTracks(*jtrack);
            (*jtrack)->setVertexed(true);
            SmartRefVector<LHCb::TbCluster>& clusters =
                const_cast<SmartRefVector<LHCb::TbCluster>&>(
                    (*jtrack)->clusters());
            for (unsigned int i = 0; i < clusters.size(); i++) {
              m_vertexedCluster[clusters[i]->plane()][clusters[i]->key()] =
                  true;
              unsigned int plane = clusters[i]->plane();
              if (plane == 0 || plane == 1 || plane == 2)
                (*jtrack)->setParentVertex(true);
            }
            break;
          }
        }
      }
      if ((*jtrack)->htime() - (*itrack)->htime() > m_vertexDelT) break;
    }
    if (tempVertex != nullptr) m_vertices->insert(tempVertex);
  }

  // Remove tracks forming vertices from m_tracks.
  LHCb::TbVertices::iterator ivertex;
  for (ivertex = m_vertices->begin(); ivertex != m_vertices->end(); ivertex++) {
    SmartRefVector<LHCb::TbTrack>& tracks =
        const_cast<SmartRefVector<LHCb::TbTrack>&>((*ivertex)->tracks());
    for (unsigned int i = 0; i < tracks.size(); i++)
      m_tracks->remove(tracks[i]);
  }
}

//=============================================================================
// Initialization
//=============================================================================
void TbVertexTracking::performVertexTracking() {
  // Iterate over search planes.
  for (const auto& plane : m_PlaneSearchOrder) {

    if (masked(plane) || m_clusterFinder->empty(plane)) continue;
    auto ic = m_clusterFinder->first(plane);
    const auto end = m_clusterFinder->end(plane);
    for (; ic != end; ++ic) {
      // Check if too big, and if already tracked.
      if ((*ic)->size() > m_clusterSizeCut) continue;
      if ((*ic)->associated() && !m_endCluster[plane][(*ic)->key()]) continue;
      formTrack(*ic);
    }
  }
}

//=============================================================================
// Initialization
//=============================================================================
void TbVertexTracking::evalHoughState(LHCb::TbCluster* seed,
                                      LHCb::TbCluster* cluster2,
                                      LHCb::TbState* tempInitialState) {
  if (m_event == m_viewerEvent && m_viewerOutput)
    outputHoughState(seed, cluster2);
  LHCb::TbTrack* track = new LHCb::TbTrack();
  track->addToClusters(seed);
  track->addToClusters(cluster2);
  track->setFirstState(*tempInitialState);
  bool sizeSuitable = fillTrack(track, seed, cluster2);
  if (!sizeSuitable) {
    counter("NumberOfSizeRejectedTracks") += 1;
    delete track;
  } else {
    m_trackFit->fit(track);
    initialStateVsFitStateTx->fill(track->firstState().tx(),
                                   tempInitialState->tx());
    initialStateVsFitStateTy->fill(track->firstState().ty(),
                                   tempInitialState->ty());
    plot(track->firstState().tx() - tempInitialState->tx(),
         "initialStateMinusFittedStateX", "initialStateMinusFittedStateX",
         -0.005, 0.005, 200);

    // Consider popping one clusters if its going to fail.
    int popID = -1;
    double chi = track->chi2PerNdof();
    if (track->clusters().size() > m_MinNClusters + 1 &&
        track->chi2PerNdof() > m_ChiSqRedCut) {
      for (unsigned int i = 0; i < m_nPlanes; i++) {
        m_trackFit->maskPlane(i);
        m_trackFit->fit(track);
        if (track->chi2PerNdof() < chi) {
          chi = track->chi2PerNdof();
          popID = i;
        }
        m_trackFit->unmaskPlane(i);
      }
      if (popID != -1) {
        for (auto cluster : track->clusters()) {
          if (int(cluster->plane()) == popID) {
            track->removeFromClusters(cluster);
          }
        }
      }
      m_trackFit->fit(track);
      if (track->chi2PerNdof() < m_ChiSqRedCut)
        counter("NumberOfPopRecoveredTracks") += 1;
    }

    if (track->chi2PerNdof() < m_ChiSqRedCut) {
      SmartRefVector<LHCb::TbCluster>& clusters =
          const_cast<SmartRefVector<LHCb::TbCluster>&>(track->clusters());
      auto earliest_hit =
          std::min_element(clusters.begin(), clusters.end(),
                           TbFunctors::LessByTime<const LHCb::TbCluster*>());

      if (timingSvc()->beforeOverlap((*earliest_hit)->time()) || m_combatRun) {
        auto farthest_hit =
            std::max_element(clusters.begin(), clusters.end(),
                             TbFunctors::LessByZ<const LHCb::TbCluster*>());
        if ((*farthest_hit)->plane() != m_nPlanes - 1) {
          m_endCluster[(*farthest_hit)->plane()][(*farthest_hit)->key()] = true;
        }
        auto nearest_hit =
            std::min_element(clusters.begin(), clusters.end(),
                             TbFunctors::LessByZ<const LHCb::TbCluster*>());
        if ((*nearest_hit)->plane() != 0) {
          m_endCluster[(*nearest_hit)->plane()][(*nearest_hit)->key()] = true;
        }

        m_tracks->insert(track);
        track->setTime(timingSvc()->localToGlobal(track->htime()));
        track->setAssociated(true);
      }
    } else {
      delete track;
      counter("NumberOfChiRejectedTracks") += 1;
    }
  }
}

//=============================================================================
//
//=============================================================================
bool TbVertexTracking::fillTrack(LHCb::TbTrack* track,
                                 LHCb::TbCluster* seedCluster,
                                 LHCb::TbCluster* cluster2) {
  unsigned int nAllowedGaps = m_nPlanes - m_MinNClusters;
  unsigned int nGaps = 0;
  for (unsigned int iplane = 0; iplane < m_nPlanes; iplane++) {
    bool foundCluster = false;
    if (m_clusterFinder->empty(iplane) || masked(iplane) ||
        iplane == seedCluster->plane() || iplane == cluster2->plane()) {
      // nGaps++;
      continue;
    }
    auto ic =
        m_clusterFinder->getIterator(seedCluster->htime() - m_twindow, iplane);
    const auto end = m_clusterFinder->end(iplane);
    for (; ic != end; ++ic) {
      if ((*ic)->size() > m_clusterSizeCut) continue;
      Gaudi::XYZPoint intercept = geomSvc()->intercept(track, iplane);
      double delr2 = pow(intercept.y() - (*ic)->y(), 2);
      delr2 += pow(intercept.x() - (*ic)->x(), 2);

      plot(intercept.x() - (*ic)->x(), "initialResidualX", "initialResidualX",
           -0.05, 0.05, 200);
      plot(intercept.y() - (*ic)->y(), "initialResidualY", "initialResidualY",
           -0.05, 0.05, 200);

      if (m_event == m_viewerEvent && m_viewerOutput)
        outputPatternRecog(intercept.x(), intercept.y(), intercept.z(),
                           seedCluster->htime());

      if (pow(delr2, 0.5) < m_radialCut) {
        if (!(*ic)->associated() ||
            m_endCluster[(*ic)->plane()][(*ic)->key()]) {
          m_volumed[(*ic)->plane()][(*ic)->key()] = true;
          track->addToClusters(*ic);
          m_trackFit->fit(track);
          foundCluster = true;
          break;
        }
      }
      if ((*ic)->htime() > seedCluster->htime() + m_twindow) break;
    }
    if (!foundCluster) nGaps++;
    if (nGaps == nAllowedGaps) return false;
  }
  if (track->clusters().size() < m_MinNClusters) return false;
  return true;
}

//=============================================================================
//
//=============================================================================
void TbVertexTracking::formTrack(LHCb::TbCluster* seedCluster) {
  bool madeTrack = false;
  // Form pairs of clusters from either side of seed plane.
  // Eval each pair as its formed.
  for (unsigned int iplane = seedCluster->plane() - 1;
       iplane != seedCluster->plane() + 2; iplane++) {
    if (m_clusterFinder->empty(iplane) || iplane == seedCluster->plane() ||
        masked(iplane))
      continue;
    auto ic =
        m_clusterFinder->getIterator(seedCluster->htime() - m_twindow, iplane);
    const auto end = m_clusterFinder->end(iplane);
    for (; ic != end; ++ic) {

      // Find the gradients between the pair.
      if ((*ic)->size() > m_clusterSizeCut || (*ic)->associated()) continue;
      double tx =
          (seedCluster->x() - (*ic)->x()) / (seedCluster->z() - (*ic)->z());
      double ty =
          (seedCluster->y() - (*ic)->y()) / (seedCluster->z() - (*ic)->z());
      double x0 = seedCluster->x() - tx * seedCluster->z();
      double y0 = seedCluster->y() - ty * seedCluster->z();
      Gaudi::SymMatrix4x4 cov;
      LHCb::TbState fstate(Gaudi::Vector4(x0, y0, tx, ty), cov, 0., 0);
      plot(tx, "initialTx", "initialTx", -0.1, 0.1, 200);
      plot(ty, "initialTy", "initialTy", -0.1, 0.1, 200);

      counter("NumberOfFormedHoughStates") += 1;
      if (fabs(tx) < 0.01 && fabs(ty) < 0.01) counter("nThinStates") += 1;
      if (fabs(tx) < m_currentAngleCut && fabs(ty) < m_currentAngleCut) {
        evalHoughState(seedCluster, (*ic), &fstate);
        if (seedCluster->associated() &&
            !m_endCluster[seedCluster->plane()][seedCluster->key()]) {
          madeTrack = true;
          break;
        }
      } else
        counter("nAngleCutStates") += 1;
      if ((*ic)->htime() > seedCluster->htime() + m_twindow) break;
    }
    if (madeTrack) break;
  }
}

//=============================================================================
// Viewer outputs
//=============================================================================
void TbVertexTracking::outputVertices() {
  std::ofstream myfile;
  myfile.open("/afs/cern.ch/user/d/dsaunder/KeplerViewerData.dat",
              std::ofstream::app);
  for (LHCb::TbVertex* vertex : *m_vertices) {
    myfile << "Vertex " << vertex->x() << " " << vertex->y() << " "
           << vertex->z() << " " << vertex->htime() << " "
           << vertex->tracks().size() << " ";
    for (unsigned int i = 0; i < vertex->tracks().size(); i++) {
      myfile << vertex->tracks()[i]->firstState().tx() << " "
             << vertex->tracks()[i]->firstState().x() << " "
             << vertex->tracks()[i]->firstState().ty() << " "
             << vertex->tracks()[i]->firstState().y() << " ";
      if (vertex->tracks()[i]->parentVertex())
        myfile << "1 ";
      else
        myfile << "0 ";
    }
    myfile << "\n";
  }
  myfile.close();
}

//=============================================================================
// Viewer output
//=============================================================================
void TbVertexTracking::outputPatternRecog(double x, double y, double z,
                                          double ht) {
  std::ofstream myfile;
  myfile.open("/afs/cern.ch/user/d/dsaunder/KeplerViewerData.dat",
              std::ofstream::app);
  myfile << "PatternRecogCircle " << x << " " << y << " " << z << " " << ht
         << " " << m_radialCut << "\n";
  myfile.close();
}

//=============================================================================
// Viewer output
//=============================================================================
void TbVertexTracking::outputHoughState(LHCb::TbCluster* c1,
                                        LHCb::TbCluster* c2) {
  std::ofstream myfile;
  myfile.open("/afs/cern.ch/user/d/dsaunder/KeplerViewerData.dat",
              std::ofstream::app);
  myfile << "HoughState " << c1->x() << " " << c1->y() << " " << c1->z() << " "
         << c2->x() << " " << c2->y() << " " << c2->z() << " " << c1->htime()
         << " " << c2->htime() << " \n";
  myfile.close();
}

//=============================================================================
// Viewer output
//=============================================================================
void TbVertexTracking::outputViewerData() {
  std::ofstream myfile;
  myfile.open("/afs/cern.ch/user/d/dsaunder/KeplerViewerData.dat",
              std::ofstream::app);
  // First output the chips.
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
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
  for (LHCb::TbTrack* track : *m_tracks) {
    myfile << "Track " << track->firstState().tx() << " "
           << track->firstState().x() << " " << track->firstState().ty() << " "
           << track->firstState().y() << " " << track->htime() << "\n";
  }

  // Clusters.
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    const auto end = m_clusterFinder->end(i);
    for (auto it = m_clusterFinder->first(i); it != end; ++it) {
      myfile << "Cluster " << (*it)->x() << " " << (*it)->y() << " "
             << (*it)->z() << " " << (*it)->htime() << " ";
      const bool endCluster = m_endCluster[(*it)->plane()][(*it)->key()];
      const bool vertexed = m_vertexedCluster[(*it)->plane()][(*it)->key()];
      if (endCluster && vertexed) {
        myfile << "5 \n";
      } else if (vertexed) {
        myfile << "4 \n";
      } else if (endCluster) {
        myfile << "3 \n";
      } else if ((*it)->associated()) {
        myfile << "2 \n";
      } else if (m_volumed[(*it)->plane()][(*it)->key()]) {
        myfile << "1 \n";
      } else {
        myfile << "0 \n";
      }

      // Its hits.
      for (auto hit : (*it)->hits()) {
        myfile << "Pixel ";
        double xLocal = 0.;
        double yLocal = 0.;
        geomSvc()->pixelToPoint(hit->scol(), hit->row(), i, xLocal, yLocal);
        Gaudi::XYZPoint pLocal(xLocal - 0.5 * Tb::PixelPitch,
                               yLocal - 0.5 * Tb::PixelPitch, 0.);
        Gaudi::XYZPoint posn = geomSvc()->localToGlobal(pLocal, i);
        myfile << posn.x() << " " << posn.y() << " " << posn.z() << " ";

        Gaudi::XYZPoint posn2(pLocal.x() + 0.055, pLocal.y(), 0.);
        posn = geomSvc()->localToGlobal(posn2, i);
        myfile << posn.x() << " " << posn.y() << " " << posn.z() << " ";

        Gaudi::XYZPoint posn3(pLocal.x() + 0.055, pLocal.y() + 0.055, 0.);
        posn = geomSvc()->localToGlobal(posn3, i);
        myfile << posn.x() << " " << posn.y() << " " << posn.z() << " ";

        Gaudi::XYZPoint posn4(pLocal.x(), pLocal.y() + 0.055, 0.);
        posn = geomSvc()->localToGlobal(posn4, i);
        myfile << posn.x() << " " << posn.y() << " " << posn.z() << " ";

        myfile << hit->htime() << " " << hit->ToT() << "\n";
      }
    }
  }
  myfile.close();
  outputVertices();
}

//=============================================================================
// Track time ordering - honeycomb
//=============================================================================
void TbVertexTracking::timeOrderTracks() {

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
