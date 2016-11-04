#pragma once

#include "GaudiKernel/IService.h"

/** @class ITbDataSvc ITbDataSvc.h
 *
 * Interface for accessing raw and configuration data
 *
 */

class GAUDI_API ITbDataSvc : virtual public IService {
 public:
  DeclareInterfaceID(ITbDataSvc, 1, 0);
  virtual const std::vector<std::string>& getInputFiles() = 0;
  virtual const std::vector<std::string>& getPixelConfig() = 0;
  virtual const std::string& getTimingConfig() = 0;
  virtual const std::string& getAlignmentFile() = 0;
  virtual const std::vector<std::string>& getEtaConfig() = 0;
};
