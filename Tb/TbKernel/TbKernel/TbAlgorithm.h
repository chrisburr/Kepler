#ifndef TB_ALGORITHM_H
#define TB_ALGORITHM_H 1

// Gaudi
#include "GaudiAlg/GaudiTupleAlg.h"

// Tb/TbKernel
#include "TbKernel/ITbGeometrySvc.h"
#include "TbKernel/ITbTimingSvc.h"
#include "TbKernel/ITbPixelSvc.h"
#include "TbKernel/ITbDataSvc.h"

/** @class TbAlgorithm TbAlgorithm.h
 *
 *  Base class for testbeam algorithms.
 *
 */

class TbAlgorithm : public GaudiTupleAlg {
 public:
  /// Standard constructor
  TbAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbAlgorithm();

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution
  virtual StatusCode finalize();    ///< Algorithm finalization

 protected:
  /// Number of telescope planes
  unsigned int m_nPlanes = 0;
  /// Number of chips, counting chips in planes with multiple chips
  unsigned int m_nDevices = 0;
  /// Flag to print out algorithm properties during initialization.
  bool m_printConfiguration = false;
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
  /// Pointer to pixel service
  mutable ITbPixelSvc* m_pixelSvc = nullptr;
  /// Access pixel service on-demand.
  ITbPixelSvc* pixelSvc() const {
    if (!m_pixelSvc) m_pixelSvc = svc<ITbPixelSvc>("TbPixelSvc", true);
    return m_pixelSvc;
  }
  /// Pointer to timing service
  mutable ITbTimingSvc* m_timingSvc = nullptr;
  /// Access timing service on-demand.
  ITbTimingSvc* timingSvc() const {
    if (!m_timingSvc) m_timingSvc = svc<ITbTimingSvc>("TbTimingSvc", true);
    return m_timingSvc;
  }
  /// Pointer to data service
  mutable ITbDataSvc* m_dataSvc = nullptr;
  /// Access data service on-demand.
  ITbDataSvc* dataSvc() const {
    if (!m_dataSvc) m_dataSvc = svc<ITbDataSvc>("TbDataSvc", true);
    return m_dataSvc;
  }

  /// Retrieve the masked flag for a given plane.
  bool masked(const unsigned int plane) const {
    return plane < m_masked.size() ? m_masked[plane] : false;
  }
};

#endif
