// AIDA
#include "AIDA/IAxis.h"

// Gaudi
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiUtils/HistoLabels.h"

// Tb/TbEvent
#include "Event/TbTrack.h"
#include "Event/TbCluster.h"

// Tb/TbKernel
#include "TbKernel/TbConstants.h"
#include "TbKernel/TbModule.h"

// Local
#include "TbDUTMonitor.h"

using namespace Gaudi::Utils::Histos;

DECLARE_ALGORITHM_FACTORY(TbDUTMonitor)

//=============================================================================
// Standard constructor
//=============================================================================
TbDUTMonitor::TbDUTMonitor(const std::string& name, ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator),
      m_parResXY("", -0.2, 0.2, 200),
      m_parXY("", -20, 20, 500),
      m_parScanX("", 0, 0, 1),
      m_parScanY("", 0, 0, 1),
      m_parResTime("", -100., 100., 2000) {

  declareProperty("TrackLocation",
                  m_trackLocation = LHCb::TbTrackLocation::Default);
  declareProperty("DUTs", m_duts);

  declareProperty("Scan", m_scan = false); 
  declareProperty("ScanX", m_parScanX);
  declareProperty("ScanY", m_parScanY);
}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbDUTMonitor::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;

  // Setup the histograms.
  const unsigned int nDuts = m_duts.size();
  for (unsigned int i = 0; i < nDuts; ++i) {
    m_index[m_duts[i]] = i;
    const std::string plane = std::to_string(m_duts[i]);
    const std::string title = geomSvc()->modules().at(m_duts[i])->id();

    unsigned int bins = m_parResXY.bins();
    double low = m_parResXY.lowEdge();
    double high = m_parResXY.highEdge();
    std::string name = "ResidualsLocalX/Plane" + plane;
    m_hResLocalX.push_back(book1D(name, title, low, high, bins));
    name = "ResidualsLocalY/Plane" + plane;
    m_hResLocalY.push_back(book1D(name, title, low, high, bins));
    name = "ResidualsGlobalX/Plane" + plane;
    m_hResGlobalX.push_back(book1D(name, title, low, high, bins));
    name = "ResidualsGlobalY/Plane" + plane;
    m_hResGlobalY.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_hResLocalX[i], "#it{x} - #it{x}_{track} [mm]", "entries");
    setAxisLabels(m_hResLocalY[i], "#it{y} - #it{y}_{track} [mm]", "entries");

    unsigned int binsXY = m_parXY.bins();
    double lowXY = m_parXY.lowEdge();
    double highXY = m_parXY.highEdge();

    m_hUnbiasedResGlobalXvsGlobalX.push_back(
        book2D("GlobalResXvsGlobalX/Plane" + plane, title, lowXY, highXY,
               binsXY, low, high, bins));

    m_hUnbiasedResGlobalYvsGlobalY.push_back(
        book2D("GlobalResYvsGlobalY/Plane" + plane, title, lowXY, highXY,
               binsXY, low, high, bins));

    m_hUnbiasedResGlobalXvsTrackChi2.push_back(
        book2D("GlobalResXvsTrackChi2/Plane" + plane, title, 0, 100, 1000, low,
               high, bins));
    m_hUnbiasedResGlobalYvsTrackChi2.push_back(
        book2D("GlobalResYvsTrackChi2/Plane" + plane, title, 0, 100, 1000, low,
               high, bins));

    m_hUnbiasedResGlobalXvsTrackTx.push_back(
        book2D("GlobalResXvsTrackTx/Plane" + plane, title, -0.002, 0.002, 100,
               low, high, bins));
    m_hUnbiasedResGlobalXvsTrackTy.push_back(
        book2D("GlobalResXvsTrackTy/Plane" + plane, title, -0.002, 0.002, 100,
               low, high, bins));
    m_hUnbiasedResGlobalYvsTrackTx.push_back(
        book2D("GlobalResYvsTrackTx/Plane" + plane, title, -0.002, 0.002, 100,
               low, high, bins));
    m_hUnbiasedResGlobalYvsTrackTy.push_back(
        book2D("GlobalResYvsTrackTy/Plane" + plane, title, -0.002, 0.002, 100,
               low, high, bins));

    m_hUnbiasedResGlobalXvsPixelX.push_back(
        book2D("UnbiasedResGlobalXvsPixelX/Plane" + plane, title, -1.0, 1.0,
               200, low, high, bins));

    m_hUnbiasedResGlobalXvsPixelY.push_back(
        book2D("UnbiasedResGlobalXvsPixelY/Plane" + plane, title, -1.0, 1.0,
               200, low, high, bins));

    m_hUnbiasedResGlobalYvsPixelX.push_back(
        book2D("UnbiasedResGlobalYvsPixelX/Plane" + plane, title, -1.0, 1.0,
               200, low, high, bins));

    m_hUnbiasedResGlobalYvsPixelY.push_back(
        book2D("UnbiasedResGlobalYvsPixelY/Plane" + plane, title, -1.0, 1.0,
               200, low, high, bins));

    m_hUnbiasedResGlobalXvsClusterSize.push_back(
        book2D("GlobalResXvsClusterSize/Plane" + plane, title, 0.5, 10.5, 10,
               low, high, bins));
    m_hUnbiasedResGlobalYvsClusterSize.push_back(
        book2D("GlobalResYvsClusterSize/Plane" + plane, title, 0.5, 10.5, 10,
               low, high, bins));

    //   m_UnbiasedResGlobalXvshUnbiasedResGlobalY
    m_UnbiasedResGlobalXvshUnbiasedResGlobalY.push_back(
        book2D("UnbiasedResGlobalXvshUnbiasedResGlobalY/Plane" + plane, title,
               low, high, bins, low, high, bins));

    bins = m_parResTime.bins();
    low = m_parResTime.lowEdge();
    high = m_parResTime.highEdge();
    name = "ResidualsTime/Plane" + plane;
    m_hResTime.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_hResTime[i], "#it{t} - #it{t}_{track} [ns]", "entries");
  }
  std::vector<std::string> labels = {"X", "Y", "Z", "rotX", "rotY", "rotZ"};
  unsigned int binsXY = m_parXY.bins();
  double lowXY = m_parXY.lowEdge();
  double highXY = m_parXY.highEdge();
  unsigned int bins = m_parResXY.bins();
  double low = m_parResXY.lowEdge();
  double high = m_parResXY.highEdge();

  if (nDuts == 0) return sc;
  TbModule* mod = geomSvc()->module(m_duts[0]);
  std::vector<bool> xaxis(6, 0);
  std::vector<bool> yaxis(6, 0);
  std::vector<std::string>::iterator xL =
      std::find(labels.begin(), labels.end(), m_parScanX.title());
  std::vector<std::string>::iterator yL =
      std::find(labels.begin(), labels.end(), m_parScanY.title());
  if (xL != labels.end()) xaxis[xL - labels.begin()] = 1;
  if (yL != labels.end()) yaxis[yL - labels.begin()] = 1;

  if (xL != labels.end()) {
    for (int i = 0; i < m_parScanX.bins(); ++i) {
      const double px = m_parScanX.lowEdge() +
                        i * (m_parScanX.highEdge() - m_parScanX.lowEdge()) /
                            m_parScanX.bins();
      std::string label =
          "Scan #Delta" + m_parScanX.title() + " = " + std::to_string(px);
      if (yL == labels.end()) {
        info() << "Booking scan " << *xL << " = " << px << endmsg;
        m_hScanXvsX.push_back(book2D("XvsX/Scan" + std::to_string(i), label,
                                     lowXY, highXY, binsXY, low, high, bins));
        m_hScanYvsY.push_back(book2D("YvsY/Scan" + std::to_string(i), label,
                                     lowXY, highXY, binsXY, low, high, bins));
        m_testModules.push_back(new TbModule());
        (*m_testModules.rbegin())->setAlignment(
            mod->x() + px * xaxis[0], mod->y() + px * xaxis[1],
            mod->z() + px * xaxis[2], mod->rotX() + px * xaxis[3],
            mod->rotY() + px * xaxis[4], mod->rotZ() + px * xaxis[5], 0, 0, 0,
            0, 0, 0);
      } else {
        for (int j = 0; j < m_parScanY.bins(); ++j) {

          const double py = m_parScanY.lowEdge() +
                            j * (m_parScanY.highEdge() - m_parScanY.lowEdge()) /
                                m_parScanY.bins();

          info() << "Booking scan " << *xL << " = " << px << ", " << *yL
                 << " = " << py << endmsg;

          std::string l =
              label + ", " + m_parScanY.title() + " = " + std::to_string(py);
          m_hScanXvsX.push_back(
              book2D("XvsX/Scan" + std::to_string(i * m_parScanY.bins() + j), l,
                     lowXY, highXY, binsXY, low, high, bins));
          m_hScanYvsY.push_back(
              book2D("YvsY/Scan" + std::to_string(i * m_parScanY.bins() + j), l,
                     lowXY, highXY, binsXY, low, high, bins));
          m_testModules.push_back(new TbModule());

          (*m_testModules.rbegin())->setAlignment(
              mod->x() + px * xaxis[0] + py * yaxis[0],
              mod->y() + px * xaxis[1] + py * yaxis[1],
              mod->z() + px * xaxis[2] + py * yaxis[2],
              mod->rotX() + px * xaxis[3] + py * yaxis[3],
              mod->rotY() + px * xaxis[4] + py * yaxis[4],
              mod->rotZ() + px * xaxis[5] + py * yaxis[5], 0, 0, 0, 0, 0, 0);
        }
      }
    }
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbDUTMonitor::execute() {

  // Grab the tracks.
  const LHCb::TbTracks* tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!tracks) {
    return Error("No tracks in " + m_trackLocation);
  }

  // Loop over the tracks.
  for (const LHCb::TbTrack* track : *tracks) {
    auto clusters = track->associatedClusters();
    for (auto it = clusters.cbegin(), end = clusters.cend(); it != end; ++it) {
      const unsigned int plane = (*it)->plane();
      if (m_index.count(plane) < 1) continue;
      const Gaudi::XYZPoint pGlobal = geomSvc()->intercept(track, plane);
      const auto pLocal = geomSvc()->globalToLocal(pGlobal, plane);
      const unsigned int i = m_index[plane];
      const double dt = (*it)->htime() - track->htime();
      const double dx = (*it)->xloc() - pLocal.x();
      const double dy = (*it)->yloc() - pLocal.y();
      m_hResLocalX[i]->fill(dx);
      m_hResLocalY[i]->fill(dy);

      const double dxug = (*it)->x() - pGlobal.x();
      const double dyug = (*it)->y() - pGlobal.y();
      m_hResGlobalX[i]->fill(dxug);
      m_hResGlobalY[i]->fill(dyug);
      m_hUnbiasedResGlobalXvsGlobalX[i]->fill((*it)->x(), dxug);
      m_hUnbiasedResGlobalYvsGlobalY[i]->fill((*it)->y(), dyug);
      m_hResTime[i]->fill(dt);

      m_hUnbiasedResGlobalXvsTrackChi2[i]->fill(track->chi2PerNdof(), dxug);
      m_hUnbiasedResGlobalYvsTrackChi2[i]->fill(track->chi2PerNdof(), dyug);

      m_UnbiasedResGlobalXvshUnbiasedResGlobalY[i]->fill(dxug, dyug);

      m_hUnbiasedResGlobalXvsTrackTx[i]->fill(track->firstState().tx(), dxug);
      m_hUnbiasedResGlobalXvsTrackTy[i]->fill(track->firstState().ty(), dxug);

      m_hUnbiasedResGlobalYvsTrackTx[i]->fill(track->firstState().tx(), dyug);
      m_hUnbiasedResGlobalYvsTrackTy[i]->fill(track->firstState().ty(), dyug);

      m_hUnbiasedResGlobalXvsClusterSize[i]->fill((*it)->hits().size(), dxug);
      m_hUnbiasedResGlobalYvsClusterSize[i]->fill((*it)->hits().size(), dyug);

      unsigned int row, col;

      geomSvc()->pointToPixel(pLocal.x(), pLocal.y(), 4, col, row);
      double x0, y0;
      geomSvc()->pixelToPoint(col, row, 4, x0, y0);

      const double pixelX = (pLocal.x() - x0) / 0.055;
      const double pixelY = (pLocal.y() - y0) / 0.055;

      m_hUnbiasedResGlobalXvsPixelX[i]->fill(pixelX, dxug);
      m_hUnbiasedResGlobalXvsPixelY[i]->fill(pixelY, dxug);
      m_hUnbiasedResGlobalYvsPixelX[i]->fill(pixelX, dyug);
      m_hUnbiasedResGlobalYvsPixelY[i]->fill(pixelY, dyug);

      const unsigned int nTestModules = m_testModules.size();
      for (unsigned int j = 0; j < nTestModules; ++j) {

        const auto& p = track->firstState().position();
        const auto& t = track->firstState().slopes();
        const Gaudi::XYZVector n = m_testModules[j]->normal();
        const double s = n.Dot(m_testModules[j]->centre() - p) / n.Dot(t);
        const auto intercept = p + s * t;
        const auto cl = m_testModules[j]->transform() *
                        Gaudi::XYZPoint((*it)->xloc(), (*it)->yloc(), 0);
        const double dxug = cl.x() - intercept.x();
        const double dyug = cl.y() - intercept.y();
        m_hScanXvsX[j]->fill(cl.x(), dxug);
        m_hScanYvsY[j]->fill(cl.y(), dyug);
      }
    }
  }

  return StatusCode::SUCCESS;
}
