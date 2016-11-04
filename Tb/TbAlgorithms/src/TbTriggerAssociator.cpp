// Gaudi
#include "GaudiKernel/PhysicalConstants.h"

// Tb/TbEvent
#include "Event/TbTrack.h"

// Local
#include "TbTriggerAssociator.h"

DECLARE_ALGORITHM_FACTORY(TbTriggerAssociator)

//=============================================================================
// Standard constructor
//=============================================================================
TbTriggerAssociator::TbTriggerAssociator(const std::string& name,
                                         ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator) {

  declareProperty("TrackLocation",
                  m_trackLocation = LHCb::TbTrackLocation::Default);
  declareProperty("TriggerLocation",
                  m_triggerLocation = LHCb::TbTriggerLocation::Default);

  declareProperty("TimeWindow", m_twindow = 1000. * Gaudi::Units::ns);
  declareProperty("TimeOffset", m_toffset = 0.);
  declareProperty("Plane", m_plane = 999);
}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbTriggerAssociator::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbTriggerAssociator::execute() {

  // Grab the tracks.
  LHCb::TbTracks* tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!tracks) {
    return Error("No tracks in " + m_trackLocation);
  }
  // Grab the triggers.
  std::vector<LHCb::TbTriggers*> triggers(m_nPlanes, nullptr);
  std::vector<LHCb::TbTriggers::iterator> begin(m_nPlanes);
  std::vector<LHCb::TbTriggers::iterator> end(m_nPlanes);
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    if (m_plane != 999 && i != m_plane) continue;
    const std::string location = m_triggerLocation + std::to_string(i);
    triggers[i] = getIfExists<LHCb::TbTriggers>(location);
    if (!triggers[i]) {
      return Error("No triggers in " + location);
    }
    begin[i] = triggers[i]->begin();
    end[i] = triggers[i]->end();
  }

  // Loop over the tracks.
  for (LHCb::TbTrack* track : *tracks) {
    const double t = track->htime() + m_toffset;
    // Calculate the time window.
    const double tMin = t - m_twindow;
    const double tMax = t + m_twindow;
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      if (m_plane != 999 && i != m_plane) continue;
      if (triggers[i]->empty()) continue;
      // Get the first trigger within the time window (if any).
      LHCb::TbTriggers::iterator it =
          std::lower_bound(begin[i], end[i], tMin, lowerBound());
      if (it == end[i]) continue;
      // Associate all triggers within the window to the track.
      for (; it != end[i]; ++it) {
        // Stop when outside the time window.
        if ((*it)->htime() > tMax) break;
        track->addToTriggers(*it);
        (*it)->setAssociated(true);
      }
    }
  }
  return StatusCode::SUCCESS;
}
