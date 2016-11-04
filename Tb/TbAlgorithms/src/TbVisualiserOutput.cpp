#include <fstream>
#include <algorithm>

// Gaudi
#include "GaudiKernel/PhysicalConstants.h"

// Tb/TbEvent
#include "Event/TbCluster.h"

// Tb/TbKernel
#include "TbKernel/TbConstants.h"
#include "TbKernel/TbFunctors.h"

// Local
#include "TbVisualiserOutput.h"

DECLARE_ALGORITHM_FACTORY(TbVisualiserOutput)

//=============================================================================
// Standard constructor
//=============================================================================
TbVisualiserOutput::TbVisualiserOutput(const std::string& name,
                                       ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator), m_event(0) {
  declareProperty("ViewerEvent", m_viewerEvent = 7);
  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);
  declareProperty("TrackLocation",
                  m_trackLocation = LHCb::TbTrackLocation::Default);
}

//=============================================================================
// Destructor
//=============================================================================
TbVisualiserOutput::~TbVisualiserOutput() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbVisualiserOutput::initialize() {

  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  m_clusterFinder =
      tool<ITbClusterFinder>("TbClusterFinder", "ClusterFinder", this);

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbVisualiserOutput::execute() {
  m_tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!m_tracks) {
    return Error("No tracks in " + m_trackLocation);
  }
  if (m_event == m_viewerEvent) {

    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      const std::string clusterLocation = m_clusterLocation + std::to_string(i);
      LHCb::TbClusters* clusters =
          getIfExists<LHCb::TbClusters>(clusterLocation);
      m_clusterFinder->setClusters(clusters, i);
    }
    outputViewerData();
  }

  m_event++;
  return StatusCode::SUCCESS;
}

//=============================================================================
// Viewer output
//=============================================================================
void TbVisualiserOutput::outputViewerData() {
  std::ofstream myfile;
  myfile.open("KeplerViewerData.dat", std::ofstream::app);
  myfile << "# Output\n";
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
      const int tag = (*ic)->associated();
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
