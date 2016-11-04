// Gaudi
#include "GaudiUtils/HistoLabels.h"

// Tb/TbEvent
#include "Event/TbTrigger.h"

// Local
#include "TbTriggerMonitor.h"

using namespace Gaudi::Utils::Histos;

DECLARE_ALGORITHM_FACTORY(TbTriggerMonitor)

//=============================================================================
// Standard constructor
//=============================================================================
TbTriggerMonitor::TbTriggerMonitor(const std::string& name,
                                   ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator),
      m_parTriggersInEvent("", 0., 1000., 200),
      m_nEvents(0) {

  declareProperty("TriggerLocation",
                  m_triggerLocation = LHCb::TbTriggerLocation::Default);
  declareProperty("ParametersTriggersInEvent", m_parTriggersInEvent);
}

//=============================================================================
// Destructor
//=============================================================================
TbTriggerMonitor::~TbTriggerMonitor() {}

//=============================================================================
// Initialisation
//=============================================================================
StatusCode TbTriggerMonitor::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  m_missedTriggers.resize(m_nPlanes);
  m_counter.resize(m_nPlanes);
  m_counter.resize(m_nPlanes, 0);
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    const std::string plane = std::to_string(i);
    std::string name = "TimeBetweenTriggers";
    std::string title = "Plane " + plane;
    m_hTimeBetweenTriggers.push_back(book1D(name, title, 0., 500., 200));
    setAxisLabels(m_hTimeBetweenTriggers[i], "#Deltat [ns]", "entries");
    unsigned int bins = m_parTriggersInEvent.bins();
    double low = m_parTriggersInEvent.lowEdge();
    double high = m_parTriggersInEvent.highEdge();
    name = "TriggersInEvent/Plane" + plane;
    m_hTriggersInEvent.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_hTriggersInEvent[i], "number of triggers", "events");
    name = "TriggersInEventTrend/Plane" + plane;
    m_hTriggersInEventTrend.push_back(book1D(name, title, -0.5, 999.5, 1000));
    setAxisLabels(m_hTriggersInEventTrend[i], "event", "number of hits");
  }
  return StatusCode::SUCCESS;
}

StatusCode TbTriggerMonitor::finalize() {
  /*
    for( unsigned int i = 0 ; i < m_nPlanes; ++i){

     std::vector<std::pair<uint64_t, uint64_t>> summary = m_missedTriggers[i];
      if( summary.size() != 0 ){
        info() << "Trigger jumps on plane " << i << endmsg;;
        for( std::vector<std::pair<uint64_t, uint64_t >>::iterator jump =
    summary.begin(); jump != summary.end(); ++jump ){

          info() << "Trigger jumps from " << jump->second << " to " <<
    jump->first << endmsg;
        }
      }
    }
  */
  return TbAlgorithm::finalize();
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbTriggerMonitor::execute() {

  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    // Grab the triggers.
    const std::string ext = std::to_string(i);
    const std::string location = m_triggerLocation + ext;
    const LHCb::TbTriggers* triggers = getIfExists<LHCb::TbTriggers>(location);
    if (!triggers) return StatusCode::SUCCESS;
    m_hTriggersInEvent[i]->fill(triggers->size());
    m_hTriggersInEventTrend[i]->fill(m_nEvents, triggers->size());
    double tprev = 0.;
    LHCb::TbTriggers::const_iterator begin = triggers->begin();
    LHCb::TbTriggers::const_iterator end = triggers->end();
    for (LHCb::TbTriggers::const_iterator it = begin; it != end; ++it) {
      // Check for missed triggers.
      if ((*it)->counter() - m_counter[i] != 1 && m_counter[i] != 4095) {
        info() << "Counter on plane " << i << " jumps from " << m_counter[i]
               << " to " << (*it)->counter() << endmsg;
      }
      m_counter[i] = (*it)->counter();
      const double t = (*it)->htime();

      if (it != begin) {
        m_hTimeBetweenTriggers[i]->fill(t - tprev);
      }
      tprev = t;
      counter("effFractionAssociated" + ext) += (*it)->associated();
    }
  }
  ++m_nEvents;
  return StatusCode::SUCCESS;
}
