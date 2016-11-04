// Gaudi
#include "GaudiUtils/HistoLabels.h"

// Tb/TbEvent
#include "Event/TbHit.h"

// Tb/TbKernel
#include "TbKernel/TbConstants.h"
#include "TbKernel/TbModule.h"

// Local
#include "TbHitMonitor.h"

using namespace Gaudi::Utils::Histos;

DECLARE_ALGORITHM_FACTORY(TbHitMonitor)

//=============================================================================
// Standard constructor
//=============================================================================
TbHitMonitor::TbHitMonitor(const std::string& name, ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator),
      m_parToT("", 0.5, 1024.5, 1024),
      m_parCharge("", 0., 20000., 200),
      m_parHitsInEvent("", 0., 10000., 100),
      m_parDeltaT("", 0., 100., 200) {

  declareProperty("HitLocation", m_hitLocation = LHCb::TbHitLocation::Default);

  declareProperty("ParametersToT", m_parToT);
  declareProperty("ParametersCharge", m_parCharge);
  declareProperty("ParametersHitsInEvent", m_parHitsInEvent);
  declareProperty("ParametersDeltaT", m_parDeltaT);
}

//=============================================================================
// Destructor
//=============================================================================
TbHitMonitor::~TbHitMonitor() {}

//=============================================================================
// Initialisation
//=============================================================================
StatusCode TbHitMonitor::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;

  // Setup the histograms.
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    const std::string plane = std::to_string(i);
    const std::string title = geomSvc()->modules().at(i)->id();
    const unsigned int nRows = Tb::NRows;
    const unsigned int nCols = geomSvc()->modules().at(i)->cols();
    std::string name = "HitMap/Plane" + plane;

    m_hHitMap.push_back(book2D(name, title, -0.5, nCols - 0.5, nCols, -0.5,
                               nRows - 0.5, nRows));
    setAxisLabels(m_hHitMap[i], "column", "row");

    int bins = m_parToT.bins();
    double low = m_parToT.lowEdge();
    double high = m_parToT.highEdge();
    name = "ToT/Plane" + plane;
    m_hToT.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_hToT[i], "ToT", "entries");

    bins = m_parCharge.bins();
    low = m_parCharge.lowEdge();
    high = m_parCharge.highEdge();
    name = "Charge/Plane" + plane;
    m_hCharge.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_hCharge[i], "charge [electrons]", "entries");

    name = "ToTvsCol/Plane" + plane;
    m_hToTvsCol.push_back(bookProfile1D(name, title, 
                                        -0.5, nCols - 0.5, nCols));
    setAxisLabels(m_hToTvsCol[i], "column", "ToT");

    name = "ChargevsCol/Plane" + plane;
    m_hChargevsCol.push_back(bookProfile1D(name, title, 
                             -0.5, nCols - 0.5, nCols));
    setAxisLabels(m_hChargevsCol[i], "column", "charge [electrons]");

    bins = m_parHitsInEvent.bins();
    low = m_parHitsInEvent.lowEdge();
    high = m_parHitsInEvent.highEdge();
    name = "HitsInEvent/Plane" + plane;
    m_hHitsInEvent.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_hHitsInEvent[i], "number of hits", "events");
    name = "HitsInEventTrend/Plane" + plane;
    m_hHitsInEventTrend.push_back(book1D(name, title, -0.5, 999.5, 1000));
    setAxisLabels(m_hHitsInEventTrend[i], "event", "number of hits");

    bins = m_parDeltaT.bins();
    low = m_parDeltaT.lowEdge();
    high = m_parDeltaT.highEdge();
    name = "TimeBetweenHits/Plane" + plane;
    m_hTimeBetweenHits.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_hTimeBetweenHits[i], "#Deltat [ns]", "entries");
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbHitMonitor::execute() {

  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    // Grab the hits.
    const std::string hitLocation = m_hitLocation + std::to_string(i);
    const LHCb::TbHits* hits = getIfExists<LHCb::TbHits>(hitLocation);
    if (!hits) continue;
    m_hHitsInEvent[i]->fill(hits->size());
    m_hHitsInEventTrend[i]->fill(m_nEvents, hits->size());
    if (hits->empty()) continue;
    double tprev = 0.;
    bool first = true;
    for (const LHCb::TbHit* hit : *hits) {
      m_hToT[i]->fill(hit->ToT());
      m_hCharge[i]->fill(hit->charge());
      m_hToTvsCol[i]->fill(hit->scol(), hit->ToT());
      m_hChargevsCol[i]->fill(hit->scol(), hit->charge());
      m_hHitMap[i]->fill(hit->scol(), hit->row());
      const double t = hit->htime();
      if (!first) m_hTimeBetweenHits[i]->fill(t - tprev);
      first = false;
      tprev = t;
    }
  }
  ++m_nEvents;
  return StatusCode::SUCCESS;
}
