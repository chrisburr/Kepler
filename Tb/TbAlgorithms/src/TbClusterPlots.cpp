#include <cmath>

// Gaudi
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiUtils/Aida2ROOT.h"
#include "GaudiUtils/HistoLabels.h"

// Tb/TbEvent
#include "Event/TbHit.h"

// Tb/TbKernel
#include "TbKernel/TbModule.h"

// Local
#include "TbClusterPlots.h"

using namespace Gaudi::Utils::Histos;

DECLARE_ALGORITHM_FACTORY(TbClusterPlots)

//=============================================================================
// Standard constructor.
//=============================================================================
TbClusterPlots::TbClusterPlots(const std::string& name,
                               ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator),
      m_parToT("", 0.5, 1024.5, 1024),
      m_parCharge("", 0., 60000., 200),
      m_parXY("", -25., 25., 200),
      m_parTime("", 0., 300000., 1000),
      m_parDifferenceXY("", -2., 2., 200),
      m_parDifferenceRot("", -0.1, 0.1, 200),
      m_parDifferenceT("", -1000., 1000., 1000),
      m_parSamples(0, 100000, 1) {

  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);
  declareProperty("ReferencePlane", m_referencePlane = 0);
  declareProperty("TimeWindow", m_twindow = 250. * Gaudi::Units::ns);

  declareProperty("ParametersToT", m_parToT);
  declareProperty("ParametersCharge", m_parCharge);
  declareProperty("ParametersXY", m_parXY);
  declareProperty("ParametersTime", m_parTime);
  declareProperty("ParametersDifferenceXY", m_parDifferenceXY);
  declareProperty("ParametersDifferenceRot", m_parDifferenceRot);
  declareProperty("ParametersDifferenceT", m_parDifferenceT);

  declareProperty("FillSamples", m_fillSamples = false);
  declareProperty("FillComparisonPlots", m_fillComparisonPlots = false);
  declareProperty("ParametersSamples", m_parSamples);

}

//=============================================================================
// Destructor
//=============================================================================
TbClusterPlots::~TbClusterPlots() {}

//=============================================================================
// Initialization.
//=============================================================================
StatusCode TbClusterPlots::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  // Initialise the histograms.
  setupPlots();
  // Setup the cluster finder.
  m_clusterFinder =
      tool<ITbClusterFinder>("TbClusterFinder", "ClusterFinder", this);
  if (!m_clusterFinder) return Error("Cannot retrieve cluster finder tool.");
  m_clusterFinder->setSearchAlgorithm("adap_seq");

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution.
//=============================================================================
StatusCode TbClusterPlots::execute() {

  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    // Skip masked planes.
    if (masked(i)) continue;
    // Get the clusters for this plane.
    const std::string clusterLocation = m_clusterLocation + std::to_string(i);
    LHCb::TbClusters* clusters = getIfExists<LHCb::TbClusters>(clusterLocation);
    if (!clusters) return Error("No clusters in " + clusterLocation);
    // Fill the plots depending on just these clusters.
    fillPerChipPlots(clusters);
    fillClusterVisuals(clusters);
    // Store the iterators in the cluster finder.
    m_clusterFinder->setClusters(clusters, i);
  }

  if (m_fillComparisonPlots) fillComparisonPlots();
  m_event++;
  return StatusCode::SUCCESS;
}

//=============================================================================
// Fill "event displays".
//=============================================================================
void TbClusterPlots::fillClusterVisuals(const LHCb::TbClusters* clusters) {

  for (const LHCb::TbCluster* cluster : *clusters) {
    if (cluster->htime() > m_parSamples.highEdge()) break;
    if (cluster->htime() < m_parSamples.lowEdge()) continue;
    if (cluster->associated()) continue;
    const unsigned int plane = cluster->plane();
    for (auto hit : cluster->hits()) {
      double xLocal = 0.;
      double yLocal = 0.;
      geomSvc()->pixelToPoint(hit->scol(), hit->row(), plane, xLocal, yLocal);
      Gaudi::XYZPoint pLocal(xLocal, yLocal, 0.);
      Gaudi::XYZPoint pGlobal = geomSvc()->localToGlobal(pLocal, plane);
      m_clusterVisuals[plane]->fill(pGlobal.x(), pGlobal.y());
    }
  }
}

//=============================================================================
// Fill plots.
//=============================================================================
void TbClusterPlots::fillPerChipPlots(const LHCb::TbClusters* clusters) {

  double tprev = 0.;
  bool first = true;
  for (const LHCb::TbCluster* cluster : *clusters) {
    const unsigned int plane = cluster->plane();
    const unsigned int tot = cluster->ToT();
    const double charge = cluster->charge();
    m_hToT[plane]->fill(tot);
    m_hCharge[plane]->fill(charge);
    const unsigned int size = cluster->size();
    if (size == 1) {
      m_hToTOnePixel[plane]->fill(tot);
      m_hChargeOnePixel[plane]->fill(charge);
    } else if (size == 2) {
      m_hToTTwoPixel[plane]->fill(tot);
      m_hChargeTwoPixel[plane]->fill(charge);
    } else if (size == 3) {
      m_hToTThreePixel[plane]->fill(tot);
      m_hChargeThreePixel[plane]->fill(charge);
    } else if (size == 4) {
      m_hToTFourPixel[plane]->fill(tot);
      m_hChargeFourPixel[plane]->fill(charge);
    }
    m_hSize[plane]->fill(size);
    
    // Hitmap.
    m_hHitMap[plane]->fill(cluster->x(), cluster->y());

    counter("effFractionAssociated" + std::to_string(plane)) +=
        cluster->associated();
    if (cluster->charge() > 50) {
      counter("effFractionAssociatedAbove50TOT" + std::to_string(plane)) +=
          cluster->associated();
    }
    const double t = cluster->htime();
    m_hTime[plane]->fill(t);
    if (!first) m_hTimeBetweenClusters[plane]->fill(t - tprev);
    first = false;
    tprev = t;

    if (cluster->associated()) {
      m_hHitMapAssociated[plane]->fill(cluster->x(), cluster->y());
      m_hSizeAssociated[plane]->fill(size);
      m_hToTAssociated[plane]->fill(tot);
      m_hChargeAssociated[plane]->fill(charge);
      m_hTimeAssociated[plane]->fill(t);
    } else {
      m_hHitMapNonAssociated[plane]->fill(cluster->x(), cluster->y());
      m_hSizeNonAssociated[plane]->fill(size);
      m_hToTNonAssociated[plane]->fill(tot);
      m_hChargeNonAssociated[plane]->fill(charge);
      m_hTimeNonAssociated[plane]->fill(t);
    }
    m_hWidthCol[plane]->fill(cluster->cols());
    m_hWidthRow[plane]->fill(cluster->rows());
    m_hGlobalXvsZ->fill(cluster->z(), cluster->x());
    m_hGlobalYvsZ->fill(cluster->z(), cluster->y());

    // Loop over the hits in the cluster.
    auto hits = cluster->hits();
    bool firstHit = true;
    double tSeed = 0.;
    for (const LHCb::TbHit* hit : hits) {
      if (firstHit) {
        tSeed = hit->htime();
        firstHit = false;
      } else {
        m_hTimeSeedMinusHit[plane]->fill(hit->htime() - tSeed);
      }
    }
  }
  if (m_fillSamples) fillSamples(clusters);
}

//=============================================================================
// Comparison windows (via scrolling window).
//=============================================================================
void TbClusterPlots::fillComparisonPlots() {

  // Make sure there are clusters on the reference plane.
  if (m_clusterFinder->empty(m_referencePlane)) return;
  // Scroll over clusters on the reference plane, then draw comparisons
  // between this cluster and those inside a time window on the other planes.
  const auto refBegin = m_clusterFinder->first(m_referencePlane);
  const auto refEnd = m_clusterFinder->end(m_referencePlane);
  for (auto itRef = refBegin; itRef != refEnd; ++itRef) {
    // Calculate the time window for this cluster.
    const auto tRef = (*itRef)->htime();
    const auto tMin = tRef - m_twindow;
    const auto tMax = tRef + m_twindow;

    const double xRef = (*itRef)->x();
    const double yRef = (*itRef)->y();

    // Loop over other the other planes.
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      // Skip empty planes.
      if (m_clusterFinder->empty(i)) continue;
      // Get the first cluster within the time range.
      const auto begin = m_clusterFinder->getIterator(tMin, i);
      const auto end = m_clusterFinder->end(i);
      // Loop over the clusters within the range.
      for (auto ic = begin; ic != end; ++ic) {
        // Stop when too far ahead.
        if ((*ic)->htime() >= tMax) break;
        // (Re)-check if inside the window.
        if ((*ic)->htime() >= tMin) {
          // Fill correlation plots.
          m_gx_correls[i]->fill((*ic)->x(), xRef);
          m_gy_correls[i]->fill((*ic)->y(), yRef);
          m_gt_correls[i]->fill((*ic)->htime(), tRef);
          // Fill difference plots.
          m_gx_diffs[i]->fill((*ic)->x() - xRef);
          m_gy_diffs[i]->fill((*ic)->y() - yRef);
          m_gt_diffs[i]->fill((*ic)->htime() - tRef);
        }
      }
    }
  }
}

//=============================================================================
// Plot a set of clusters.
//=============================================================================
void TbClusterPlots::fillSamples(const LHCb::TbClusters* clusters) {

  // Takes a set number of clusters and plots their hits on a TH2, with
  // the bins weighted by ToT values. Clusters are spaced equally. A dot at
  // their reconstructed positions would be cool.

  const unsigned int nSamplesRoot = 6;
  const unsigned int nSamples = nSamplesRoot * nSamplesRoot;
  const unsigned int sampleSpacing = 6;
  const unsigned int n = nSamplesRoot * sampleSpacing;
  LHCb::TbClusters::const_iterator ic = clusters->begin();
  for (unsigned int i = 0; i < nSamples && i < clusters->size(); ++i) {
    // Modify later to not always visualize first few clusters.
    // Position of cluster seed on TH2.
    // Sequentially left to right, then down to up.
    const int c_col = (i % nSamplesRoot) * sampleSpacing;
    const int c_row = (i / nSamplesRoot) * sampleSpacing;
    const auto& hits = (*ic)->hits();
    const int seed_col = hits.front()->col();
    const int seed_row = hits.front()->row();
    for (auto it = hits.cbegin(), end = hits.cend(); it != end; ++it) {
      // Center the cluster on the seed hit, then shift to posn on TH2.
      const int col = ((*it)->col() - seed_col) + c_col;
      const int row = ((*it)->row() - seed_row) + c_row;
      plot2D(col, row, "ClusterSamples", "Cluster samples", 0, n, 0, n, n, n,
             (*it)->ToT());
    }
    ++ic;
  }

  // Switch off filling the samples plot after the first call.
  m_fillSamples = false;
}

//=============================================================================
// Initialize histograms
//=============================================================================
void TbClusterPlots::setupPlots() {

  const double zMin = geomSvc()->modules().front()->z() - 50.;
  const double zMax = geomSvc()->modules().back()->z() + 50.;
  unsigned int bins = m_parXY.bins();
  double low = m_parXY.lowEdge();
  double high = m_parXY.highEdge();
  m_hGlobalXvsZ =
      book2D("GlobalXvsZ", "GlobalXvsZ", zMin, zMax, 5000, low, high, bins);
  m_hGlobalYvsZ =
      book2D("GlobalYvsZ", "GlobalYvsZ", zMin, zMax, 5000, low, high, bins);
  setAxisLabels(m_hGlobalXvsZ, "global #it{z} [mm]", "global #it{x} [mm]");
  setAxisLabels(m_hGlobalYvsZ, "global #it{z} [mm]", "global #it{y} [mm]");
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    const std::string plane = std::to_string(i);
    const std::string title = geomSvc()->modules().at(i)->id();

    // ToT distributions
    bins = m_parToT.bins();
    low = m_parToT.lowEdge();
    high = m_parToT.highEdge();
    std::string name = "ToT/All/Plane" + plane;
    m_hToT.push_back(book1D(name, title, low, high, bins));
    name = "ToT/OnePixel/Plane" + plane;
    m_hToTOnePixel.push_back(book1D(name, title, low, high, bins));
    name = "ToT/TwoPixel/Plane" + plane;
    m_hToTTwoPixel.push_back(book1D(name, title, low, high, bins));
    name = "ToT/ThreePixel/Plane" + plane;
    m_hToTThreePixel.push_back(book1D(name, title, low, high, bins));
    name = "ToT/FourPixel/Plane" + plane;
    m_hToTFourPixel.push_back(book1D(name, title, low, high, bins));
    name = "ToT/Associated/Plane" + plane;
    m_hToTAssociated.push_back(book1D(name, title, low, high, bins));
    name = "ToT/NonAssociated/Plane" + plane;
    m_hToTNonAssociated.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_hToT[i], "ToT", "entries");
    setAxisLabels(m_hToTOnePixel[i], "ToT", "entries");
    setAxisLabels(m_hToTTwoPixel[i], "ToT", "entries");
    setAxisLabels(m_hToTThreePixel[i], "ToT", "entries");
    setAxisLabels(m_hToTFourPixel[i], "ToT", "entries");
    setAxisLabels(m_hToTAssociated[i], "ToT", "entries");
    setAxisLabels(m_hToTNonAssociated[i], "ToT", "entries");

    // Charge distributions
    bins = m_parCharge.bins();
    low = m_parCharge.lowEdge();
    high = m_parCharge.highEdge();
    name = "Charge/All/Plane" + plane;
    m_hCharge.push_back(book1D(name, title, low, high, bins));
    name = "Charge/OnePixel/Plane" + plane;
    m_hChargeOnePixel.push_back(book1D(name, title, low, high, bins));
    name = "Charge/TwoPixel/Plane" + plane;
    m_hChargeTwoPixel.push_back(book1D(name, title, low, high, bins));
    name = "Charge/ThreePixel/Plane" + plane;
    m_hChargeThreePixel.push_back(book1D(name, title, low, high, bins));
    name = "Charge/FourPixel/Plane" + plane;
    m_hChargeFourPixel.push_back(book1D(name, title, low, high, bins));
    name = "Charge/Associated/Plane" + plane;
    m_hChargeAssociated.push_back(book1D(name, title, low, high, bins));
    name = "Charge/NonAssociated/Plane" + plane;
    m_hChargeNonAssociated.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_hCharge[i], "charge [electrons]", "entries");
    setAxisLabels(m_hChargeOnePixel[i], "charge [electrons]", "entries");
    setAxisLabels(m_hChargeTwoPixel[i], "charge [electrons]", "entries");
    setAxisLabels(m_hChargeThreePixel[i], "charge [electrons]", "entries");
    setAxisLabels(m_hChargeFourPixel[i], "charge [electrons]", "entries");
    setAxisLabels(m_hChargeAssociated[i], "charge [electrons]", "entries");
    setAxisLabels(m_hChargeNonAssociated[i], "charge [electrons]", "entries");

    // Cluster size distributions
    name = "Size/Plane" + plane;
    m_hSize.push_back(book1D(name, title, 0.5, 10.5, 10));
    name = "SizeAssociated/Plane" + plane;
    m_hSizeAssociated.push_back(book1D(name, title, 0.5, 10.5, 10));
    name = "SizeNonAssociated/Plane" + plane;
    m_hSizeNonAssociated.push_back(book1D(name, title, 0.5, 10.5, 10));
    setAxisLabels(m_hSize[i], "cluster size", "entries");
    setAxisLabels(m_hSizeAssociated[i], "cluster size", "entries");
    setAxisLabels(m_hSizeNonAssociated[i], "cluster size", "entries");

    // Cluster width along column and row directions
    name = "Width/col/Plane" + plane;
    m_hWidthCol.push_back(book1D(name, title, 0.5, 10.5, 10));
    name = "Width/row/Plane" + plane;
    m_hWidthRow.push_back(book1D(name, title, 0.5, 10.5, 10));
    setAxisLabels(m_hWidthCol[i], "columns", "entries");
    setAxisLabels(m_hWidthRow[i], "rows", "entries");

    // Cluster position hit maps
    bins = m_parXY.bins();
    low = m_parXY.lowEdge();
    high = m_parXY.highEdge();
    name = "Positions/Plane" + plane;
    m_hHitMap.push_back(book2D(name, title, low, high, bins, low, high, bins));
    name = "PositionsAssociated/Plane" + plane;
    m_hHitMapAssociated.push_back(
        book2D(name, title, low, high, bins, low, high, bins));
    name = "PositionsNonAssociated/Plane" + plane;
    m_hHitMapNonAssociated.push_back(
        book2D(name, title, low, high, bins, low, high, bins));
    setAxisLabels(m_hHitMap[i], "global #it{x} [mm]", "global #it{y} [mm]");
    setAxisLabels(m_hHitMapAssociated[i], "global #it{x} [mm]",
                  "global #it{y} [mm]");
    setAxisLabels(m_hHitMapNonAssociated[i], "global #it{x} [mm]",
                  "global #it{y} [mm]");

    // Global x/y correlations
    name = "Correlations/x/Plane" + plane;
    m_gx_correls.push_back(
        book2D(name, title, low, high, bins, low, high, bins));
    name = "Correlations/y/Plane" + plane;
    m_gy_correls.push_back(
        book2D(name, title, low, high, bins, low, high, bins));
    setAxisLabels(m_gx_correls[i], "#it{x} [mm]", "#it{x}_{ref} [mm]");
    setAxisLabels(m_gy_correls[i], "#it{y} [mm]", "#it{y}_{ref} [mm]");

    // Time distributions
    bins = m_parTime.bins();
    low = m_parTime.lowEdge();
    high = m_parTime.highEdge();
    name = "Time/Plane" + plane;
    m_hTime.push_back(book1D(name, title, low, high, bins));
    name = "TimeAssociated/Plane" + plane;
    m_hTimeAssociated.push_back(book1D(name, title, low, high, bins));
    name = "TimeNonAssociated/Plane" + plane;
    m_hTimeNonAssociated.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_hTime[i], "time [ns]", "entries");
    setAxisLabels(m_hTimeAssociated[i], "time [ns]", "entries");
    setAxisLabels(m_hTimeNonAssociated[i], "time [ns]", "entries");
    // Time spread of hits within a cluster.
    name = "TimeHitMinusSeed/Plane" + plane;
    m_hTimeSeedMinusHit.push_back(book1D(name, title, 0., 500., 200));
    setAxisLabels(m_hTimeSeedMinusHit[i], "#it{t}_{hit} - #it{t}_{seed} [ns]",
                  "entries");
    // Time correlations
    name = "Correlations/t/Plane" + plane;
    m_gt_correls.push_back(
        book2D(name, title, low, high, bins, low, high, bins));
    setAxisLabels(m_gt_correls[i], "#it{t}", "#it{t}_{ref}");

    // Global x/y differences
    bins = m_parDifferenceXY.bins();
    low = m_parDifferenceXY.lowEdge();
    high = m_parDifferenceXY.highEdge();
    name = "Differences/x/Plane" + plane;
    m_gx_diffs.push_back(book1D(name, title, low, high, bins));
    name = "Differences/y/Plane" + plane;
    m_gy_diffs.push_back(book1D(name, title, low, high, bins));
    bins = m_parDifferenceRot.bins();
    low = m_parDifferenceRot.lowEdge();
    high = m_parDifferenceRot.highEdge();
    setAxisLabels(m_gx_diffs[i], "#it{x} - #it{x}_{ref} [mm]", "entries");
    setAxisLabels(m_gy_diffs[i], "#it{y} - #it{y}_{ref} [mm]", "entries");
    // Time differences
    bins = m_parDifferenceT.bins();
    low = m_parDifferenceT.lowEdge();
    high = m_parDifferenceT.highEdge();
    name = "Differences/t/Plane" + plane;
    m_gt_diffs.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_gt_diffs[i], "#it{t} - #it{t}_{ref}", "entries");

    // Time between clusters
    name = "TimeBetweenClusters/Plane" + plane,
    m_hTimeBetweenClusters.push_back(book1D(name, title, 0., 1000., 50));
    setAxisLabels(m_hTimeBetweenClusters[i], "#Deltat [ns]", "entries");
  }

  for (unsigned int i = 0; i < m_nPlanes; i++) {
    std::string name = "ClusterVisuals/Sample" + std::to_string(i);
    m_clusterVisuals.push_back(
        book2D(name, name, 0, 14.08, 256, 0, 14.08, 256));
  }
}
