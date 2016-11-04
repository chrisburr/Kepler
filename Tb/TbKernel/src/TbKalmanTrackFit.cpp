// Tb/TbEvent
#include "Event/TbCluster.h"
#include "Event/TbTrack.h"
#include "Event/TbKalmanTrack.h"
#include "Event/TbKalmanNode.h"

// Local
#include "TbKalmanTrackFit.h"

DECLARE_TOOL_FACTORY(TbKalmanTrackFit)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TbKalmanTrackFit::TbKalmanTrackFit(const std::string& type,
                                   const std::string& name,
                                   const IInterface* parent)
    : GaudiTool(type, name, parent) {

  declareInterface<ITbTrackFit>(this);

  declareProperty("MaskedPlanes", m_maskedPlanes = {});
  declareProperty("Noise2", m_scat2 = {});
  // Noise square (1.2e-8 for PS).
  declareProperty("Noise2Default", m_scat2Default = 4.e-11);

}

//=============================================================================
// Destructor
//=============================================================================
TbKalmanTrackFit::~TbKalmanTrackFit() {}

//=============================================================================
// Initialisation
//=============================================================================
StatusCode TbKalmanTrackFit::initialize() {

  // Initialise the base class.
  StatusCode sc = GaudiTool::initialize();
  if (sc.isFailure()) return sc;
  // Get the straight-line fit tool (used for seeding the Kalman filter).
  m_fitter = tool<ITbTrackFit>("TbTrackFit", "StraightLineFitter", this);
  if (!m_fitter) return Error("Cannot retrieve TbTrackFit.");

  // Get the number of telescope planes.
  const auto nPlanes = geomSvc()->modules().size();
  // Set the flags whether a plane is masked or not.
  m_masked.resize(nPlanes, false);
  for (const unsigned int plane : m_maskedPlanes) {
    m_masked[plane] = true;
    m_fitter->maskPlane(plane);
  }
  // Set the noise terms.
  if (m_scat2.size() != nPlanes) {
    info() << "Using default noise term (" << m_scat2Default 
          << ") for all planes." << endmsg;
    m_scat2.assign(nPlanes, m_scat2Default);
  } else {
    info() << "Noise terms:" << endmsg;
    for (unsigned int i = 0; i < nPlanes; ++i) {
      info() << "    Plane " << i << ": " << m_scat2[i] << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Exclude a plane from the fit
//=============================================================================
void TbKalmanTrackFit::maskPlane(const unsigned int plane) {

  if (plane < m_masked.size()) {
    m_masked[plane] = true;
    m_fitter->maskPlane(plane);
  }
}

//=============================================================================
// (Re-)include a plane in the fit
//=============================================================================
void TbKalmanTrackFit::unmaskPlane(const unsigned int plane) {

  if (plane < m_masked.size()) {
    m_masked[plane] = false;
    m_fitter->unmaskPlane(plane);
  }
}

//=========================================================================
// Perform the fit
//=========================================================================
void TbKalmanTrackFit::fit(LHCb::TbTrack* track) {

  // Make a straight-line fit.
  m_fitter->fit(track);

  // Create a Kalman track.
  LHCb::TbKalmanTrack ktrack(*track, m_scat2);
  // Run the Kalman filter.
  ktrack.fit();
  // Copy over the nodes.
  track->clearNodes();
  auto nodes = ktrack.nodes();
  for (auto it = nodes.cbegin(), end = nodes.cend(); it != end; ++it) {
    track->addToNodes(*it);
  }
  // Set the chi2 and update the fit status.
  track->setChi2PerNdof(ktrack.chi2PerNdof());
  track->setNdof(ktrack.ndof());
  track->setFitStatus(LHCb::TbTrack::Kalman);
}
