#ifndef TBTRACKFIT_H
#define TBTRACKFIT_H 1

// Gaudi
#include "GaudiAlg/GaudiTool.h"

// Tb/TbKernel
#include "TbKernel/ITbTrackFit.h"
#include "TbKernel/ITbGeometrySvc.h"

/** @class TbTrackFit TbTrackFit.h
 * Implementation of track fit for Timepix3 testbeam analysis
 *
 * @author Panagiotis Tsopelas
 * @date 2014-05-06
 *
 */

class TbTrackFit : public GaudiTool, virtual public ITbTrackFit {

 public:
  /// Constructor
  TbTrackFit(const std::string& type, const std::string& name,
             const IInterface* parent);
  /// Destructor
  virtual ~TbTrackFit();

  virtual StatusCode initialize();

  /// Fit the track and set its firstState
  virtual void fit(LHCb::TbTrack* track);

  virtual void maskPlane(const unsigned int plane);
  virtual void unmaskPlane(const unsigned int plane);

 protected:
  /// Indices of planes to be masked.
  std::vector<unsigned int> m_maskedPlanes;
  /// Flags whether a plane is masked or not.
  std::vector<bool> m_masked;

  /// Pointer to geometry service
  mutable ITbGeometrySvc* m_geomSvc = nullptr;
  /// Access geometry service on-demand
  ITbGeometrySvc* geomSvc() const {
    if (!m_geomSvc) m_geomSvc = svc<ITbGeometrySvc>("TbGeometrySvc", true);
    return m_geomSvc;
  }
};

#endif
