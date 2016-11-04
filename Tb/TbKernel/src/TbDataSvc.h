#ifndef TBPIXELSVC_H
#define TBPIXELSVC_H

// Gaudi
#include "GaudiKernel/Service.h"
#include "GaudiKernel/Bootstrap.h"

// Local
#include "TbKernel/ITbDataSvc.h"

template <class TYPE>
class SvcFactory;

/** @class TbDataSvc TbDataSvc.h
 *
 * Implementation of the Testbeam data / configuration lookup services
 * for use with EOS
 */

class TbDataSvc : public extends1<Service, ITbDataSvc> {

 public:
  /// Constructor
  TbDataSvc(const std::string& name, ISvcLocator* svc);
  /// Destructor
  virtual ~TbDataSvc();

  virtual StatusCode initialize();
  virtual StatusCode finalize();

  virtual const std::vector<std::string>& getInputFiles() {
    return m_inputFiles;
  }
  virtual const std::vector<std::string>& getPixelConfig() {
    return m_pixelFiles;
  }
  virtual const std::string& getTimingConfig() { return m_timingFile; }
  virtual const std::string& getAlignmentFile() { return m_alignmentFile; }
  virtual const std::vector<std::string>& getEtaConfig() {
    return m_etaFiles;
  }

 private:
  /// Allow SvcFactory to instantiate the service.
  friend class SvcFactory<TbDataSvc>;

  std::vector<std::string> m_inputs;
  std::vector<std::string> m_inputFiles;
  std::vector<std::string> m_pixelFiles;
  std::string m_alignmentFile;
  std::string m_timingFile;
  std::vector<std::string> m_etaFiles;

  /// Add the prefix for reading network files.
  std::string expandPath(const std::string& fname);
};

#endif
