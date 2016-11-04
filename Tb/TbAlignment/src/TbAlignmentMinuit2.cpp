#include <algorithm>

// Gaudi
#include "GaudiKernel/PhysicalConstants.h"

// Tb/TbKernel
#include "TbKernel/TbFunctors.h"

// Local
#include "TbAlignmentMinuit2.h"

DECLARE_TOOL_FACTORY(TbAlignmentMinuit2)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TbAlignmentMinuit2::TbAlignmentMinuit2(const std::string& type,
                                       const std::string& name,
                                       const IInterface* parent)
    : TbAlignmentMinuitBase(type, name, parent) {

  declareProperty("ClusterLocation", 
                  m_clusterLocation = LHCb::TbClusterLocation::Default);
  declareProperty("DeviceToAlign", m_deviceToAlign = 999);
  declareProperty("IsDUT", m_isDUT = true);
  declareProperty("RefitTracks", m_refitTracks = false);
  declareProperty("TimeWindow", m_twindow = 200. * Gaudi::Units::ns);
  declareProperty("XWindow", m_xwindow = 1. * Gaudi::Units::mm);
  declareProperty("IgnoreEdge",m_ignoreEdge=1);
  m_dofsDefault = {true, true, true, true, true, true};
}

//=============================================================================
// Destructor
//=============================================================================
TbAlignmentMinuit2::~TbAlignmentMinuit2() {}

//=============================================================================
// Collect the tracks and clusters to be used for the alignment.
//=============================================================================
StatusCode TbAlignmentMinuit2::execute(
    std::vector<TbAlignmentTrack*>& alignmentTracks) {

  // Grab the tracks.
  LHCb::TbTracks* tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!tracks) {
    return Error("No tracks in " + m_trackLocation);
  }
  if (!m_isDUT) {
    // We want to align a plane which is included in the pattern recognition.
    for (LHCb::TbTrack* track : *tracks) {
      // Skip low-quality tracks.
      if (track->chi2() > m_maxChi2) continue;
      // Add a new alignment track (cloning track and clusters) to the store.
      alignmentTracks.emplace_back(new TbAlignmentTrack(track));
    }
    return StatusCode::SUCCESS;
  }
  // We want to align a plane which is not included in the pattern recognition.
  if (m_twindow < 0. && m_xwindow < 0.) {
    // Use the clusters which have been associated to the track.
    for (LHCb::TbTrack* track : *tracks) {
      // Skip low-quality tracks.
      if (track->chi2() > m_maxChi2) continue;
      auto clusters = track->associatedClusters();
      for (auto it = clusters.begin(), end = clusters.end(); it != end; ++it) {
        if ((*it)->plane() != m_deviceToAlign) continue;
        // Create a new alignment track (cloning the track).
        TbAlignmentTrack* alignmentTrack = new TbAlignmentTrack(track);
        // Clone the cluster and add it to the alignment track.
        LHCb::TbCluster* alignmentCluster = (*it)->clone();
        alignmentTrack->track()->addToAssociatedClusters(alignmentCluster);
        alignmentTrack->addToClusters(alignmentCluster);
        // Add the alignment track to the store.
        alignmentTracks.push_back(alignmentTrack);
        // Stop after the first valid cluster.
        break;
      }
    }
    return StatusCode::SUCCESS;
  }
  // We need to associate DUT clusters to the track.
  // Grab the clusters on the device to align.
  const std::string clusterLocation = m_clusterLocation + std::to_string(m_deviceToAlign);
  LHCb::TbClusters* clusters = getIfExists<LHCb::TbClusters>(clusterLocation);
  if (!clusters) {
    return Error("No clusters in " + clusterLocation);
  }
  unsigned int edgeRejected(0), spaceRejected(0);

  for (LHCb::TbTrack* track : *tracks) {
    // Skip low-quality tracks.
    if (track->chi2PerNdof() > m_maxChi2) continue;
    // Calculate the track intercept on the device to align.
    const auto pGlobal = geomSvc()->intercept(track, m_deviceToAlign);
    const auto pLocal = geomSvc()->globalToLocal(pGlobal, m_deviceToAlign);
    // Calculate the time window.
    const double tMin = track->htime() - m_twindow;
    const double tMax = track->htime() + m_twindow;
    // Get the first cluster within the time window.
    LHCb::TbClusters::iterator end = clusters->end();
    LHCb::TbClusters::iterator begin = std::lower_bound(
        clusters->begin(), end, tMin, lowerBound<const LHCb::TbCluster*>());
    if (begin == clusters->end()) continue;
    // Loop over the clusters.
    for (LHCb::TbClusters::iterator it = begin; it != end; ++it) {
      // Stop when outside the time window.
      if ((*it)->htime() > tMax) break;
      // Skip clusters too far away from the track intercept.
      const double dx = (*it)->xloc() - pLocal.x();
      const double dy = (*it)->yloc() - pLocal.y();
      if (fabs(dx) > m_xwindow || fabs(dy) > m_xwindow){ spaceRejected++; continue; }
      // Skip clusters close to the edge of the sensor.
      if (isEdge(*it) && m_ignoreEdge ){edgeRejected++; continue;}
      // Create a new alignment track (cloning the track).
      TbAlignmentTrack* alignmentTrack = new TbAlignmentTrack(track);
      // Clone the cluster and add it to the alignment track.
      LHCb::TbCluster* alignmentCluster = (*it)->clone();
      alignmentTrack->track()->addToAssociatedClusters(alignmentCluster);
      alignmentTrack->addToClusters(alignmentCluster);
      // Add the alignment track to the store.
      alignmentTracks.push_back(alignmentTrack);
      // Stop after the first valid cluster.
      break;
    }
  }
  //info() << edgeRejected << "   " << spaceRejected << "    " << tracks->size() << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Calculate the chi2.
//=============================================================================
void TbAlignmentMinuit2::chi2(double& f, double* par, double* /*g*/) {

  const unsigned int offset = m_deviceToAlign * 6;
  // Check the z-position.
  const double z = m_modules[m_deviceToAlign]->z() + par[offset + 2];
  if (m_deviceToAlign > 0) {
    if (z <= m_modules[m_deviceToAlign - 1]->z()) {
      info() << "Z is below limit!" << endmsg;
      f = std::numeric_limits<double>::max();
      return;
    }
  }
  if (m_deviceToAlign < m_nPlanes - 1) {
    if (z >= m_modules[m_deviceToAlign + 1]->z()) {
      info() << "Z is above limit!" << endmsg;
      f = std::numeric_limits<double>::max();
      return;
    }
  }

  m_modules[m_deviceToAlign]->setAlignment(par[offset + 0], par[offset + 1],
                                           par[offset + 2], par[offset + 3],
                                           par[offset + 4], par[offset + 5]);
  // Loop over the alignment tracks.
  f = 0.;
  for (auto it = m_tracks.begin(), end = m_tracks.end(); it != end; ++it) {
    // Form residuals with all clusters selected for alignment
    auto clusters = (*it)->clusters();
    for (auto ic = clusters.cbegin(), end = clusters.cend(); ic != end; ++ic) {
      if ((*ic)->plane() != m_deviceToAlign) continue;
      Gaudi::XYZPoint point = geomSvc()->localToGlobal(
          Gaudi::XYZPoint((*ic)->xloc(), (*ic)->yloc(), 0), m_deviceToAlign);
      Gaudi::XYZPoint intercept =
          geomSvc()->intercept((*it)->track(), m_deviceToAlign);
      // TODO: why not the local residual?
      const double xresidual = point.x() - intercept.x();
      // const double xresidual = (*ic)->xloc() - intercept.x();
      const double yresidual = point.y() - intercept.y();
      // const double yresidual = (*ic)->yloc() - intercept.y();
      f += xresidual * xresidual + yresidual * yresidual;
    }
  }
}

//=============================================================================
// Main alignment function.
//=============================================================================
void TbAlignmentMinuit2::align(
    std::vector<TbAlignmentTrack*>& alignmentTracks) {

  TbAlignmentMinuitBase::align(alignmentTracks);
  info() << "Minuit technique 2. Aligning plane " << m_deviceToAlign << endmsg;
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    if (masked(i) || i == m_deviceToAlign)
      m_trackFit->maskPlane(i);
    else
      m_trackFit->unmaskPlane(i);
  }

  for (auto alignmentTrack : m_tracks) m_trackFit->fit(alignmentTrack->track());

  for (unsigned int iteration = 0; iteration < m_nIterations; ++iteration) {
    info() << "Iteration " << iteration + 1 << "/" << m_nIterations << endmsg;
    // Loop over detector modules
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      const unsigned int offset = 6 * i;
      if (i == m_deviceToAlign) {
        info() << "*** Wobbling detector " << i << endmsg;
        for (unsigned int j = 0; j < 6; ++j) {
          if (m_dofs[j]) {
            m_fitter->ReleaseParameter(offset + j);
          } else {
            m_fitter->FixParameter(offset + j);
          }
        }
      } else {
        m_fitter->FixParameter(offset + 0);  // x
        m_fitter->FixParameter(offset + 1);  // y
        m_fitter->FixParameter(offset + 2);  // z
        m_fitter->FixParameter(offset + 3);  // Rx
        m_fitter->FixParameter(offset + 4);  // Ry
        m_fitter->FixParameter(offset + 5);  // Rz
      }
    }
    // Execute minimization and calculate proper error matrix
    double arglist[2];
    arglist[0] = 10000;
    arglist[1] = 1.e-2;
    m_fitter->ExecuteCommand("MIGRAD", arglist, 2);
    m_fitter->ExecuteCommand("HESSE", arglist, 1);
    if (msgLevel(MSG::INFO)) m_fitter->ExecuteCommand("SHOW PARAMETERS", 0, 0);
 
    updateGeometry();
  }

  if (m_refitTracks) {
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      if (masked(i))
        m_trackFit->maskPlane(i);
      else
        m_trackFit->unmaskPlane(i);
    }
    for (auto it : m_tracks) m_trackFit->fit(it->track());
  }
}
