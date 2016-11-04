#pragma once

#include "GaudiAlg/GaudiAlgorithm.h"

// Tb/TbKernel
#include "TbKernel/ITbTimingSvc.h"

// Local
#include "TbUTRawData.h"
#include "TbUTAlibavaDataRetreiver.h"
#include "TbUTAlbavaFileValidator.h"
#include "TbUTRawDataFactory.h"
#include "TbUTAlibavaDataReader.h"

#include <string>


namespace TbUT
{

class RawDataReaderAlgorithm : public GaudiAlgorithm {
 public:
	RawDataReaderAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

	virtual StatusCode initialize();
	virtual StatusCode execute();

 private:
	void termiateApp();

	bool m_isStandalone;
	bool m_isAType;
	unsigned int m_eventNumber;
	unsigned int m_skipEventNumber;

	std::string  m_inputDataOption;
	std::string m_alibavaInputData;
	std::string m_outputLocation;
	int m_sensorNumber;

	double m_mean;
	double m_sigma;


	TbUT::AlibavaDataRetreiver m_alibava;
	TbUT::AlbavaFileValidator m_fileValidator;
	TbUT::RawDataFactory::DataReaderPtr m_rawDataReader;
	TbUT::RawDataFactory m_inputDataFactory;

  /// Pointer to timing service
  mutable ITbTimingSvc* m_timingSvc;
  /// Access timing service on-demand
  ITbTimingSvc* timingSvc() const {
    if (!m_timingSvc) m_timingSvc = svc<ITbTimingSvc>("TbTimingSvc", true);
    return m_timingSvc;
  }
};

}

