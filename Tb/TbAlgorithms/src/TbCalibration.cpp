#include <fstream>

// Tb/TbKernel
#include "TbKernel/TbModule.h"

// Local
#include "TbCalibration.h"

// ROOT
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TH1D.h"

/// GAUDI
#include "GaudiUtils/Aida2ROOT.h"

DECLARE_ALGORITHM_FACTORY(TbCalibration)

//=============================================================================
// Standard constructor
//=============================================================================
TbCalibration::TbCalibration(const std::string& name, ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator) {
  declareProperty("PixelConfigFile", m_pixelSvcConfig = "PixelConfig.dat");
  declareProperty("TimingConfigFile", m_timingSvcConfig = "TimingConfig.dat");
  declareProperty("CheckHotPixels", m_checkHotPixels = false);
  declareProperty("CheckSynchronisation", m_checkSyncronisation = false);
  declareProperty("SyncMethod", m_syncMethod = 1);
  declareProperty("CheckColumnOffsets", m_checkColumnOffsets = false);
  declareProperty("HitLocation", m_hitLocation = LHCb::TbHitLocation::Default);
  declareProperty("DuT", m_dut = 9999);
  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);
}

//=============================================================================
// Initialisation
//=============================================================================
StatusCode TbCalibration::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;

  if (m_checkColumnOffsets) m_offsets.resize(m_nPlanes, PROFILE1D(128));
  if (m_checkHotPixels) m_hitMaps.resize(m_nDevices, PROFILE2D(256, 256));
  if (m_checkSyncronisation) m_sync.resize(m_nPlanes);
  return sc;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbCalibration::execute() {

  if (m_checkColumnOffsets) {
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      LHCb::TbClusters* clusters =
          getIfExists<LHCb::TbClusters>(m_clusterLocation + std::to_string(i));
      if (!clusters) continue;
      columnOffset_execute(clusters);
    }
  }
  if (m_checkHotPixels) {
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      LHCb::TbHits* hits =
          getIfExists<LHCb::TbHits>(m_hitLocation + std::to_string(i));
      if (!hits) continue;
      hotPixel_execute(hits);
    }
  }
  if (m_checkSyncronisation) {
    if (m_syncMethod == 0) {
      std::vector<LHCb::TbClusters*> clusters;
      for (unsigned int i = 0; i < m_nPlanes; ++i)
        clusters.push_back(getIfExists<LHCb::TbClusters>(m_clusterLocation +
                                                         std::to_string(i)));
      sync_execute(clusters);
    }
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Finalisation
//=============================================================================
StatusCode TbCalibration::finalize() {

  std::ofstream pixelFile(m_pixelSvcConfig.c_str());
  std::ofstream timingFile(m_timingSvcConfig.c_str());

  if (m_checkHotPixels) {
    pixelFile << "Mask" << std::endl;
    for (unsigned int i = 0; i < m_nDevices; ++i)
      hotPixelAnalysis(m_hitMaps[i], geomSvc()->module(i)->id(), pixelFile);
  }
  if (m_checkSyncronisation) {
    if (m_syncMethod == 0)
      for (unsigned int i = 0; i < m_nPlanes; ++i)
        syncAnalysis(m_sync[i].avg(), geomSvc()->module(i)->id(), timingFile);
    else if (m_syncMethod == 1)
      syncOffset2(timingFile);
  }
  if (m_checkColumnOffsets) {
    pixelFile << "Offset" << std::endl;
    for (unsigned int i = 0; i < m_nPlanes; ++i)
      columnOffsetAnalysis(m_offsets[i], geomSvc()->module(i)->id(), pixelFile);
  }
  pixelFile.close();
  timingFile.close();
  return StatusCode::SUCCESS;
}

//=============================================================================
// Identify hot pixels
//=============================================================================
void TbCalibration::hotPixelAnalysis(const PROFILE2D& hitMap,
                                     const std::string& plane,
                                     std::ostream& os) {

  // Based on Hella's script for identifying hot pixels.
  // Reject pixels which are more than 40x average
  for (int col = 0; col < 256; col++) {
    for (int row = 0; row < 256; row++) {
      double count = (hitMap[col][row]).n();
      std::vector<double> nn = hitMap.neighbours(col, row);

      // Cull outliers if not at edge
      if (nn.size() == 8) {
        std::sort(nn.begin(), nn.end());
        nn.erase(nn.begin(), nn.begin() + 2);
        nn.erase(nn.end() - 2, nn.end());
      }

      double total = std::accumulate(nn.begin(), nn.end(), 0);

      if (count > 10 * total) {
        os << plane << " " << std::setw(3) << col << " " << row << std::endl;
      }
    }
  }
}

void TbCalibration::syncOffset2(std::ostream& os) {

  os << "Timing" << std::endl;

  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    std::string title =
        "Tb/TbTrackPlots/BiasedResiduals/Time/Plane" + std::to_string(i);
    if (i == m_dut)
      title = "Tb/TbDUTMonitor/ResidualsTime/Plane" + std::to_string(i);
    AIDA::IHistogram1D* hAida = NULL;
    StatusCode sc = GaudiAlgorithm::histoSvc()->retrieveObject(title, hAida);
    auto hRoot = Gaudi::Utils::Aida2ROOT::aida2root(hAida);
    os << geomSvc()->module(i)->id() << std::setw(3) << "  "
       << -hRoot->GetMean() << std::endl;
  }
}

//=============================================================================
// Calculate time offsets for each column
//=============================================================================
void TbCalibration::columnOffsetAnalysis(const PROFILE1D& avg_difference,
                                         const std::string& plane,
                                         std::ostream& os) {
  const unsigned int nDcols = 128;
  std::vector<int> offsets(nDcols, 0);
  double sum(0.), total(0.);
  for (auto& d : avg_difference) {
    sum += d.val();
    total += d.n();
  }
  double avg = sum / total;

  for (unsigned int dcol = 1; dcol < nDcols; ++dcol) {
    const double difference =
        avg_difference[dcol].avg() - avg - 25 * offsets[dcol - 1];
    if (difference > 10.)
      offsets[dcol] = -1;
    else if (difference < -10.)
      offsets[dcol] = +1;
  }
  // calculate the average offset

  // deal with the special case where the first super column is desynchronised,
  // in which case, everything will be shifted by 25 ns in this definition.
  double avg_offset(0.);
  for (const auto& d : offsets) avg_offset += d;
  avg_offset /= 128.;

  if (avg_offset > 0.5)
    for (auto& d : offsets) d--;

  else if (avg_offset < -0.5)
    for (auto& d : offsets) d++;

  for (unsigned int dcol = 0; dcol < nDcols; ++dcol) {
    if (offsets[dcol] != 0)
      os << plane << " " << std::setw(3) << dcol << " " << offsets[dcol]
         << std::endl;
  }
}

//=============================================================================
// Fill the data for calculating the column time offsets
//=============================================================================
void TbCalibration::columnOffset_execute(const LHCb::TbClusters* clusters) {
  if (!clusters || clusters->empty()) return;
  LHCb::TbClusters::const_iterator itc;
  for (itc = clusters->begin(); itc != clusters->end(); ++itc) {
    const unsigned int plane = (*itc)->plane();
    if ((*itc)->size() == 1) continue;
    auto hits = (*itc)->hits();
    for (auto& ih0 : hits) {
      for (auto& ih1 : hits) {
        const LHCb::TbHit* h0 = ih0;
        const LHCb::TbHit* h1 = ih1;
        const int col0 = h0->col();
        const int col1 = h1->col();
        if (abs(col0 - col1) != 1) continue;
        if (h0->row() == h1->row() && h0->col() / 2 != h1->col() / 2) {
          if (h0->col() > h1->col()) std::swap(h0, h1);
          m_offsets[plane][(int)(h1->col() / 2)].add(h1->htime() - h0->htime());
        }
      }
    }
  }
}

//=============================================================================
// Fill the data for checking the synchronisation.
//=============================================================================
void TbCalibration::sync_execute(
    const std::vector<LHCb::TbClusters*>& clusters) {

  LHCb::TbClusters* plane0_clusters = clusters[0];
  for (auto& c : *plane0_clusters) {
    for (unsigned int i = 1; i < m_nPlanes; ++i) {
      double nearest = nearestHit(c, clusters[i]);
      if (nearest != 9999) m_sync[i].add(nearest);
    }
  }
}

//=============================================================================
// Get the smallest time difference of a list of clusters.
//=============================================================================
double TbCalibration::nearestHit(const LHCb::TbCluster* cluster,
                                 const LHCb::TbClusters* clusters) {
  if (!clusters || !cluster || clusters->empty()) return 9999;
  double minTime = cluster->htime() - 50.;
  double maxTime = cluster->htime() + 50.;
  LHCb::TbClusters::const_iterator c = std::lower_bound(
      clusters->begin(), clusters->end(), minTime, lowerBound());
  double nn = 9999;
  for (; c != clusters->end() && (*c)->htime() < maxTime; ++c) {
    if (std::abs((*c)->htime() - cluster->htime()) < std::abs(nn))
      nn = (*c)->htime() - cluster->htime();
  }
  return nn;
}

//=============================================================================
// Fill the hit map for identifying hot pixels
//=============================================================================
void TbCalibration::hotPixel_execute(const LHCb::TbHits* hits) {
  if (!hits || hits->empty()) return;
  const unsigned int device = (*hits->begin())->device();
  for (auto& h : *hits) m_hitMaps[device][h->col()][h->row()].add(1);
}

//=============================================================================
// Save synchronisation info to file.
//=============================================================================
void TbCalibration::syncAnalysis(const double& sync, const std::string& plane,
                                 std::ostream& os) {

  os << plane << std::setw(3) << " " << sync << std::endl;
}
