// Gaudi
#include "GaudiKernel/PhysicalConstants.h"

// Tb/TbKernel
#include "TbKernel/TbFunctors.h"
#include "TbKernel/TbConstants.h"
#include "TbKernel/TbModule.h"
#include "TbKernel/TbCondFile.h"

// Local
#include "TbClustering.h"

DECLARE_ALGORITHM_FACTORY(TbClustering)

//=============================================================================
// Standard constructor
//=============================================================================
TbClustering::TbClustering(const std::string& name, ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator) {

  declareProperty("HitLocation", m_hitLocation = LHCb::TbHitLocation::Default);
  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);
  declareProperty("TimeWindow", m_twindow = 100. * Gaudi::Units::ns);
  declareProperty("SearchDist", m_searchDist = 1);
  declareProperty("ClusterErrorMethod", m_clusterErrorMethod = 0);
}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbClustering::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;

  m_eta.resize(m_nPlanes);
  for (const auto& filename : dataSvc()->getEtaConfig()) {
    info() << "Importing eta corrections from " << filename << endmsg; 
    readEta(filename);
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbClustering::execute() {

  // Loop over the planes.
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    // Get the pixel hits to be clustered.
    const std::string ext = std::to_string(i);
    const std::string hitLocation = m_hitLocation + ext;
    const LHCb::TbHits* hits = getIfExists<LHCb::TbHits>(hitLocation);
    if (!hits) return Error("No hits in " + hitLocation);
    // Create a cluster container and transfer its ownership to the TES.
    LHCb::TbClusters* clusters = new LHCb::TbClusters();
    put(clusters, m_clusterLocation + ext);
    // Skip masked planes.
    if (masked(i)) continue;
    // Keep track of which hits have been clustered.
    std::vector<bool> used(hits->size(), false);
    std::vector<const LHCb::TbHit*> pixels;
    pixels.reserve(100);
    // Cycle over the (time ordered) hits.
    for (LHCb::TbHits::const_iterator it = hits->begin(); it != hits->end();
         ++it) {
      // Skip hits which are already part of a cluster.
      if (used[(*it)->key()]) continue;
      // Start a cluster from this seed pixel and tag the seed as used.
      used[(*it)->key()] = true;
      pixels.clear();
      pixels.push_back(*it);
      // Get the search range.
      const double tMax = (*it)->htime() + m_twindow;
      LHCb::TbHits::const_iterator end = std::upper_bound(
          it, hits->end(), tMax, lowerBound<const LHCb::TbHit*>());
      // Add neighbouring hits in the cluster time window.
      addNeighbouringHits(pixels, it, end, used);
      // Sort the hits by time.
      std::sort(pixels.begin(), pixels.end(),
                TbFunctors::LessByTime<const LHCb::TbHit*>());
      // Finally, set the remaining cluster attributes according to its hits.
      completeCluster(i, pixels, clusters);
    }
    // Sort the clusters by time.
    std::sort(clusters->begin(), clusters->end(),
              TbFunctors::LessByTime<LHCb::TbCluster*>());
    // Fill the counter.
    counter("NumberOfClusters" + ext) += clusters->size();
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
// Finding touching hits
//=============================================================================
void TbClustering::addNeighbouringHits(std::vector<const LHCb::TbHit*>& pixels,
                                       LHCb::TbHits::const_iterator begin,
                                       LHCb::TbHits::const_iterator end,
                                       std::vector<bool>& used) {

  // Keep track of the cluster's bounding box.
  int scolmin = pixels.front()->scol();
  int scolmax = scolmin;
  int rowmin = pixels.front()->row();
  int rowmax = rowmin;
  // Try adding hits to the cluster.
  bool hitAdded = true;
  while (hitAdded) {
    hitAdded = false;
    unsigned int nUnused = 0;
    for (LHCb::TbHits::const_iterator it = begin; it != end; ++it) {
      // Skip hits which are already part of a cluster.
      if (used[(*it)->key()]) continue;
      ++nUnused;
      const int scol = (*it)->scol();
      const int row = (*it)->row();
      if (scol < scolmin - m_searchDist) continue;
      if (scol > scolmax + m_searchDist) continue;
      if (row < rowmin - m_searchDist) continue;
      if (row > rowmax + m_searchDist) continue;
      // Ask if the hit is touching the cluster (with its current set of hits).
      if (hitTouchesCluster((*it)->scol(), (*it)->row(), pixels)) {
        // Add the hit to the cluster.
        pixels.push_back(*it);
        used[(*it)->key()] = true;
        --nUnused;
        hitAdded = true;
        // Update the bounding box.
        if (scol < scolmin)
          scolmin = scol;
        else if (scol > scolmax)
          scolmax = scol;
        if (row < rowmin)
          rowmin = row;
        else if (row > rowmax)
          rowmax = row;
      }
    }
    if (nUnused == 0) break;
  }
}

//=============================================================================
// Complete remaining cluster attributes
//=============================================================================
void TbClustering::completeCluster(
    const unsigned int plane, const std::vector<const LHCb::TbHit*>& pixels,
    LHCb::TbClusters* clusters) {

  // Create a new cluster object.
  LHCb::TbCluster* cluster = new LHCb::TbCluster();
  cluster->setPlane(plane);
  // Set cluster width along the column and row direction.
  auto cols = std::minmax_element(pixels.cbegin(), pixels.cend(),
                        [](const LHCb::TbHit* h1, const LHCb::TbHit* h2) {
    return h1->scol() < h2->scol();
  });
  auto rows = std::minmax_element(pixels.cbegin(), pixels.cend(),
                        [](const LHCb::TbHit* h1, const LHCb::TbHit* h2) {
    return h1->row() < h2->row(); 
  });
  const unsigned int nCols =
    1 + (*cols.second)->scol() - (*cols.first)->scol();
  const unsigned int nRows = 1 + (*rows.second)->row() - (*rows.first)->row();
  cluster->setCols(nCols);
  cluster->setRows(nRows);
  // Add the pixel hits to the cluster, sum up the charge and
  // calculate the centre of gravity.
  unsigned int tot = 0;
  double charge = 0.;
  double xLocal = 0.;
  double yLocal = 0.;
  for (auto it = pixels.cbegin(), end = pixels.cend(); it != end; ++it) {
    cluster->addToHits(*it);
    tot += (*it)->ToT();
    const double q = (*it)->charge();
    charge += q;
    double x = 0.;
    double y = 0.;
    geomSvc()->pixelToPoint((*it)->scol(), (*it)->row(), plane, x, y);
    xLocal += x * q;
    yLocal += y * q;
  }
  cluster->setToT(tot);
  cluster->setCharge(charge);
  // Assume that the cluster time is the time of the earliest hit.
  cluster->setTime(pixels.front()->time());
  cluster->setHtime(pixels.front()->htime());
  // Calculate the local and global coordinates.
  xLocal /= charge;
  yLocal /= charge;
  // Apply eta-corrections.
  etaCorrection(xLocal, yLocal, nCols, nRows, plane);
  
  cluster->setXloc(xLocal);
  cluster->setYloc(yLocal);
  const Gaudi::XYZPoint pLocal(xLocal, yLocal, 0.);
  const auto pGlobal = geomSvc()->localToGlobal(pLocal, plane);
  cluster->setX(pGlobal.x());
  cluster->setY(pGlobal.y());
  cluster->setZ(pGlobal.z());
  // Assign error estimates.
  setClusterError(cluster);
  // Add the cluster to the container.
  clusters->insert(cluster);
}

//=============================================================================
// Calculate the cluster error estimate.
//=============================================================================
void TbClustering::setClusterError(LHCb::TbCluster* cluster) const {

  // Initialise the errors to some default value (for large cluster widths).
  double dx = 0.1;
  double dy = 0.1;
  const bool dut = (m_nPlanes == 9) && (cluster->plane() == 4);
  const unsigned int clusterErrorMethod = dut ? 2 : m_clusterErrorMethod;
 
  switch (clusterErrorMethod) {
    case 0: {
      if (cluster->plane() < 4) {
        constexpr std::array<double, 3> sigmasX = {0.0028, 0.0040, 0.016};
        constexpr std::array<double, 3> sigmasY = {0.0022, 0.0034, 0.012};
        if (cluster->cols() < 4) dx = sigmasX[cluster->cols() - 1];
        if (cluster->rows() < 4) dy = sigmasY[cluster->rows() - 1];
        if (cluster->plane() == 0) {
          if (cluster->cols() == 3) dx = 0.010;
          if (cluster->rows() == 3) dy = 0.009;
        }
      } else {
        constexpr std::array<double, 3> sigmasX = {0.0035, 0.0047, 0.016};
        constexpr std::array<double, 3> sigmasY = {0.0029, 0.0042, 0.013};
        if (cluster->cols() < 4) dx = sigmasX[cluster->cols() - 1];
        if (cluster->rows() < 4) dy = sigmasY[cluster->rows() - 1];
      }
      break;
    } 
    case 2:
      dx = dy = 0.004;
      break;
    default:
      dx = dy = 0.004;
      break;
  }
  cluster->setXErr(dx);
  cluster->setYErr(dy);
  cluster->setWx(1. / (dx * dx));
  cluster->setWy(1. / (dy * dy));
}


//=============================================================================
// Read the eta correction parameters from file.
//=============================================================================
void TbClustering::readEta(const std::string& filename) {

  TbCondFile f(filename);
  if (!f.is_open()) {
    warning() << "Cannot open " << filename << endmsg;
    return;
  }
  unsigned int indexPlane = 999;
  unsigned int indexXy = 0;
  unsigned int indexCol = 0;
  unsigned int nLines = 0;
  while (!f.eof()) {
    std::string line = "";
    ++nLines;
    if (!f.getLine(line)) continue;
    if (line.find("Plane") != std::string::npos) {
      indexPlane = 999;
      std::string key = "";
      std::string id = "";
      std::string xy = "";
      int cols = 0;
      if (!f.split(line, ' ', key, id, xy, cols)) {
        warning() << "Error reading line " << nLines << endmsg;
        continue;
      }
      // Find the index corresponding to the given plane name. 
      for (unsigned int i = 0; i < m_nPlanes; ++i) {
        if (id == geomSvc()->modules()[i]->id()) {
          indexPlane = i;
          break;
        }
      } 
      if (indexPlane == 999) {
        warning() << "Module " << id 
                  << " is not in the alignment file" << endmsg;
        continue;
      }
      // Check whether this set of parameters is for the x or the y direction.
      if (xy == "x" || xy == "X") {
        indexXy = 0;
      } else if (xy == "y" || xy == "Y") {
        indexXy = 1;
      } else {
        warning() << "Unexpected direction specifier " << xy 
                  << " (expected x or y). Skip this parameter set." << endmsg;
        indexPlane = 999;
        continue;
      }
      // Check the cluster width.
      if (cols < 2) {
        warning() << "Unexpected cluster width (" << cols
                  << "). Skip this parameter set." << endmsg;
        indexPlane = 999;
        continue; 
      }
      indexCol = cols - 2;
      if (m_eta[indexPlane][indexXy].size() < indexCol + 1) {
        m_eta[indexPlane][indexXy].resize(indexCol + 1);
      }
      const std::string rowcol = indexXy == 0 ? " columns." : " rows.";
      info() << "Reading eta-correction parameters for plane " << indexPlane
             << " for clusters covering " << cols << rowcol << endmsg;
      m_eta[indexPlane][indexXy][indexCol].clear();
    } else {
      if (indexPlane == 999) continue;
      // Read the intra-pixel interval and the coefficients of the polynomial.
      double xmin = 0., xmax = 0.;
      double a = 0., b = 0., c = 0.;
      if (!f.split(line, ' ', xmin, xmax, a, b, c)) {
        warning() << "Error reading line " << nLines << endmsg;
        continue;
      }
      EtaCorrection corr;
      corr.xmin = xmin;
      corr.xmax = xmax;
      corr.coefficients = {a, b, c};
      m_eta[indexPlane][indexXy][indexCol].push_back(corr);
    }
  }

}

//=============================================================================
// Calculate the eta correction terms.
//=============================================================================
void TbClustering::etaCorrection(double& xLocal, double& yLocal,
                                 const unsigned int nCols, 
                                 const unsigned int nRows, 
                                 const unsigned int plane) const { 

  // No point to continue in case of single-pixel clusters.
  if (nCols * nRows == 1) return;
  // Calculate the intra-pixel coordinates of the cluster.
  unsigned int scol = 0, row = 0;
  geomSvc()->pointToPixel(xLocal, yLocal, plane, scol, row);
  double x0 = 0., y0 = 0.;
  geomSvc()->pixelToPoint(scol, row, plane, x0, y0);
  // TODO: this only works for single-chip assemblies.
  const double x = xLocal - x0 + 0.5 * Tb::PixelPitch;
  const double y = yLocal - y0 + 0.5 * Tb::PixelPitch;
  // Calculate the correction terms.
  if (nCols > 1 && m_eta[plane][0].size() > nCols - 2) {
    const auto& corrections = m_eta[plane][0][nCols - 2];
    for (const auto& corr : corrections) {
      if (x >= corr.xmin && x < corr.xmax) {
        xLocal += corr.coefficients[0] + corr.coefficients[1] * x + 
                  corr.coefficients[2] * x * x;
        break;
      }
    }
  } 
  if (nRows > 1 && m_eta[plane][1].size() > nRows - 2) {
    const auto& corrections = m_eta[plane][1][nRows - 2];
    for (const auto& corr : corrections) {
      if (y >= corr.xmin && y < corr.xmax) {
        yLocal += corr.coefficients[0] + corr.coefficients[1] * y + 
                  corr.coefficients[2] * y * y;
        break;
      }
    }
  } 
}
