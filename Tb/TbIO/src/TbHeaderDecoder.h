#ifndef TBHEADERDECODER_H
#define TBHEADERDECODER_H 1

// Gaudi
#include "GaudiAlg/GaudiTool.h"

#include "TbKernel/ITbGeometrySvc.h"
#include "TbKernel/ITbPixelSvc.h"

/** @class TbHeaderDecoder TbHeaderDecoder.h
 *
 * Tool to read SPIDR header from raw data file.
 *
 */

static const InterfaceID IID_TbHeaderDecoder("TbHeaderDecoder", 1, 0);

class TbHeaderDecoder : public GaudiTool {
 public:
  // Return the interface ID
  static const InterfaceID& interfaceID() { return IID_TbHeaderDecoder; }

  /// Constructor
  TbHeaderDecoder(const std::string& type, const std::string& name,
                  const IInterface* parent);
  /// Destructor
  virtual ~TbHeaderDecoder();

  virtual StatusCode initialize();

  void print(const bool& flag) { m_print = flag; }
  bool read(const char* data, const unsigned int& fmt, 
            std::string& deviceId);
  bool readCommon(const char* data, unsigned int& size, unsigned int& fmt);

 private:
  /// Flag to activate print-out or not.
  bool m_print;

  /// Pointer to geometry service
  mutable ITbGeometrySvc* m_geomSvc;
  /// Access geometry service on-demand
  ITbGeometrySvc* geomSvc() const {
    if (!m_geomSvc) m_geomSvc = svc<ITbGeometrySvc>("TbGeometrySvc", true);
    return m_geomSvc;
  }

  /// Pointer to pixel service
  mutable ITbPixelSvc* m_pixelSvc;
  /// Access pixel service on-demand
  ITbPixelSvc* pixelSvc() const {
    if (!m_pixelSvc) m_pixelSvc = svc<ITbPixelSvc>("TbPixelSvc", true);
    return m_pixelSvc;
  }
  /// Read version 1
  bool readV1(const char* data, std::string& deviceId);
};

#endif
