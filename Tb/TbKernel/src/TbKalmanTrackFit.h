#ifndef TBTRACKFIT_H
#define TBTRACKFIT_H 1

// Gaudi
#include "GaudiAlg/GaudiTool.h"

// Tb/TbKernel
#include "TbKernel/ITbTrackFit.h"
#include "TbKernel/ITbGeometrySvc.h"

/** @class TbKalmanTrackFit TbKalmanTrackFit.h
 * Track fit tool implementation using a Kalman-filter
 *
 */

class TbKalmanTrackFit : public GaudiTool, virtual public ITbTrackFit {

 public:
  /// Constructor
  TbKalmanTrackFit(const std::string& type, const std::string& name,
                   const IInterface* parent);
  /// Destructor
  virtual ~TbKalmanTrackFit();

  virtual StatusCode initialize();

  /// Fit the track
  virtual void fit(LHCb::TbTrack* track);

  virtual void maskPlane(const unsigned int plane);
  virtual void unmaskPlane(const unsigned int plane);

 protected:
  /// Indices of planes to be masked.
  std::vector<unsigned int> m_maskedPlanes;
  /// Flags whether a plane is masked or not.
  std::vector<bool> m_masked;

  /// Noise term squared for each plane
  std::vector<double> m_scat2;
  /// Default noise term squared
  double m_scat2Default;

  /// Straight-line fit
  ITbTrackFit* m_fitter = nullptr;

  /// Pointer to geometry service
  mutable ITbGeometrySvc* m_geomSvc = nullptr;
  /// Access geometry service on-demand
  ITbGeometrySvc* geomSvc() const {
    if (!m_geomSvc) m_geomSvc = svc<ITbGeometrySvc>("TbGeometrySvc", true);
    return m_geomSvc;
  }
};

#endif
