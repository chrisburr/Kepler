// Local
#include "TbTimingSvc.h"

DECLARE_SERVICE_FACTORY(TbTimingSvc)

//============================================================================
// Constructor
//============================================================================
TbTimingSvc::TbTimingSvc(const std::string& name, ISvcLocator* svc)
    : base_class(name, svc),
      m_evtMinTime(0UL),
      m_evtMaxTime(0UL) {}

//============================================================================
// Destructor
//============================================================================
TbTimingSvc::~TbTimingSvc() {}

//============================================================================
// Initialisation
//============================================================================
StatusCode TbTimingSvc::initialize() {

  // Initialise the base class.
  StatusCode sc = Service::initialize();
  if (!sc.isSuccess()) return sc;
  return StatusCode::SUCCESS;
}

//============================================================================
// Finalisation
//============================================================================
StatusCode TbTimingSvc::finalize() {

  // Finalise the base class.
  return Service::finalize();
}

//============================================================================
// Convert local time (in ns) to global timestamp.
//============================================================================
uint64_t TbTimingSvc::localToGlobal(const double htime) {

  // Conversion factor (25 ns correspond to 4096 global time units).
  constexpr double f = 4096. / 25.;
  const double dt = floor(htime * f);
  return dt < 0. ? m_evtMinTime - static_cast<uint64_t>(fabs(dt))
                 : m_evtMinTime + static_cast<uint64_t>(dt);
}

//============================================================================
// Convert global timestamp to local time (in ns).
//============================================================================
double TbTimingSvc::globalToLocal(const uint64_t time) {
  // Conversion factor (4096 global time units correspond to 25 ns).
  constexpr double f = 25. / 4096.;
  return time < m_evtMinTime ? -f * (m_evtMinTime - time)
                             : f * (time - m_evtMinTime);
}
