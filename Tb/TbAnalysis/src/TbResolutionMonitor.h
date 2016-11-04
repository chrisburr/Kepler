#pragma once

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"

/** @class TbResolutionMonitor TbResolutionMonitor.h
 *
 */

class TbResolutionMonitor : public TbAlgorithm {
 public:
  /// Constructor
  TbResolutionMonitor(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbResolutionMonitor() {}

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  std::vector<unsigned int> m_duts;
  std::string m_trackLocation;
  std::string m_clusterLocation;

};
