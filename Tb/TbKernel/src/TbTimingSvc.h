#ifndef TBTIMINGSVC_H
#define TBTIMINGSVC_H 1

// Gaudi
#include "GaudiKernel/Service.h"
#include "GaudiKernel/Bootstrap.h"

// Local
#include "TbKernel/ITbTimingSvc.h"
#include "TbKernel/ITbGeometrySvc.h"

template <class TYPE>
class SvcFactory;

/** @class TbTimingSvc TbTimingSvc.h
 *
 * Implementation of the testbeam timing service.
 *
 */

class TbTimingSvc : public extends1<Service, ITbTimingSvc> {

 public:
  /// Constructor
  TbTimingSvc(const std::string& name, ISvcLocator* svc);
  /// Destructor
  virtual ~TbTimingSvc();

  virtual StatusCode initialize();
  virtual StatusCode finalize();

  virtual uint64_t localToGlobal(const double htime);
  virtual double globalToLocal(const uint64_t time);

  virtual void setEventDefinition(const uint64_t evtMinTime,
                                  const uint64_t evtMaxTime) {
    m_evtMinTime = evtMinTime;
    m_evtMaxTime = evtMaxTime;
  }
  virtual void eventDefinition(uint64_t& evtMinTime,
                               uint64_t& evtMaxTime) const {
    evtMinTime = m_evtMinTime;
    evtMaxTime = m_evtMaxTime;
  }

  virtual void setOverlap(const uint64_t overlap) { m_overlap = overlap; }
  virtual bool inEvent(const uint64_t time) const {
    return time > m_evtMinTime && time < m_evtMaxTime;
  }
  virtual bool inOverlap(const uint64_t time) const {
    return time > m_evtMaxTime - m_overlap && time < m_evtMaxTime;
  }
  virtual bool beforeOverlap(const uint64_t time) const {
    return time < m_evtMaxTime - m_overlap;
  }

 private:
  /// Allow SvcFactory to instantiate the service.
  friend class SvcFactory<TbTimingSvc>;

  /// Lower limit of the current event (in global time units)
  uint64_t m_evtMinTime;
  /// Upper limit of the current event (in global time units)
  uint64_t m_evtMaxTime;
  /// Overlap with the next event (in global time units)
  uint64_t m_overlap;

};

#endif
