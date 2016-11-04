#pragma once

// AIDA
#include "AIDA/IHistogram1D.h"

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"

/** @class TbChargeCalib TbChargeCalib.h
 *
 */

class TbChargeCalib : public TbAlgorithm {
 public:
  /// Constructor
  TbChargeCalib(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbChargeCalib() {}

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  std::string m_clusterLocation;

  std::vector<AIDA::IHistogram1D*> m_ToTHists;

};
