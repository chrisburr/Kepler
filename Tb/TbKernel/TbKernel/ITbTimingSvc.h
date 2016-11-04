#ifndef TBKERNEL_ITBTIMINGSVC_H
#define TBKERNEL_ITBTIMINGSVC_H 1

#include "GaudiKernel/IService.h"

/** @class ITbTimingSvc ITbTimingSvc.h
 *
 * Interface for the testbeam timing service.
 *
 */

class GAUDI_API ITbTimingSvc : virtual public IService {

 public:
  /// InterfaceID
  DeclareInterfaceID(ITbTimingSvc, 1, 0);

  virtual uint64_t localToGlobal(const double htime) = 0;
  virtual double globalToLocal(const uint64_t time) = 0;

  virtual void setEventDefinition(const uint64_t evtMinTime,
                                  const uint64_t evtMaxTime) = 0;
  virtual void eventDefinition(uint64_t& evtMinTime,
                               uint64_t& evtMaxTime) const = 0;

  virtual void setOverlap(const uint64_t overlap) = 0;

  virtual bool inEvent(const uint64_t time) const = 0;
  virtual bool inOverlap(const uint64_t time) const = 0;
  virtual bool beforeOverlap(const uint64_t time) const = 0;
};

#endif
