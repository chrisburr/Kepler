#ifndef TBKERNEL_ITBTRACKFIT_H
#define TBKERNEL_ITBTRACKFIT_H 1

#include "GaudiKernel/IAlgTool.h"

#include "Event/TbTrack.h"

static const InterfaceID IID_ITbTrackFit("ITbTrackFit", 1, 0);

/** @class ITbTrackFit ITbTrackFit.h
 *
 *  Interface for track fit tools
 *
 */

class ITbTrackFit : virtual public IAlgTool {

 public:
  /// Return the interface ID
  static const InterfaceID& interfaceID() { return IID_ITbTrackFit; }

  virtual void fit(LHCb::TbTrack* track) = 0;

  /// Exclude a plane from the fit.
  virtual void maskPlane(const unsigned int plane) = 0;
  /// Include a plane in the fit.
  virtual void unmaskPlane(const unsigned int plane) = 0;
};
#endif
