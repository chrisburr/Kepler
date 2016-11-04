#ifndef TBKERNEL_ITBPIXELSVC_H
#define TBKERNEL_ITBPIXELSVC_H 1

#include "GaudiKernel/IService.h"

// Tb/TbEvent
#include "Event/TbHit.h"

/** @class ITbPixelSvc ITbPixelSvc.h
 *
 * Interface for pixel by pixel configuration of Timepix3 devices
 *
 */

class GAUDI_API ITbPixelSvc : virtual public IService {
 public:
  /// InterfaceID
  DeclareInterfaceID(ITbPixelSvc, 1, 0);

  /// Get the pixel address for a given column and row.
  virtual unsigned int address(const unsigned int col,
                               const unsigned int row) const = 0;

  /// Return whether the pixel has been masked.
  virtual bool isMasked(const unsigned int address,
                        const unsigned int device) const = 0;

  /// Correct the timestamp of a given hit.
  virtual void applyPhaseCorrection(LHCb::TbHit* hit) = 0;

  /// Set the clock phase correction.
  virtual void setPhase(const unsigned int device,
                        const unsigned int pll_config,
                        const int amplitude = 1) = 0;

  /// Set the trim DACs.
  virtual void setTrim(const unsigned int device, const char* data) = 0;

  /// Convert time-over-threshold to charge.
  virtual double charge(const unsigned int tot, const unsigned int address,
                        const unsigned int device) const = 0;
};

#endif
