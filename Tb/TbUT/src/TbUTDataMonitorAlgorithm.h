#pragma once

#include "GaudiAlg/GaudiHistoAlg.h"
#include "TbUTRawData.h"
#include <TH1D.h>
#include <TH2D.h>
#include <boost/shared_ptr.hpp>

namespace TbUT{

class DataMonitorAlgorithm: public GaudiHistoAlg
{
public:
	DataMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

	virtual 	~DataMonitorAlgorithm();

	virtual StatusCode initialize();
	virtual StatusCode execute   ();
	virtual StatusCode finalize  ();

protected:

	enum RunPhase
	{
		SKIP,
		SAVE_SINGLE_EVENTS,
		REST
	};

  virtual StatusCode getData();
  virtual StatusCode initializeBase();

  virtual StatusCode skippEvent();
  virtual StatusCode saveSimpleEvents();
  virtual StatusCode fillOnly2DHistogram();


  virtual	RunPhase getRunPhase();

  virtual void createHistogram2D();
  virtual void storeEventIntoHistogram();
  virtual TH1D * bookHistogram(const std::string & p_histogramName, const std::string & p_histogramTitle, int p_sensorNumber);
  virtual TH2D * bookHistogram2D(const std::string & p_histogramName, const std::string & p_histogramTitle, int p_sensorNumber);

  virtual std::string createHistogramTitle();
  virtual std::string createHistogramName();
  virtual void fillHistogram(TH1D * p_histogram);
  virtual void fillHistogram2D();
  virtual void buildProjectionHistogram();



  std::string m_inputDataLoc;
  std::string m_outpuProjectionHistogramName;
  int m_displayEventNumber;
  int m_evtNumber;
  int m_skipEvent;
  TH2D* m_histogram2D;
  TbUT::RawDataContainer<> *m_dataContainer;
  TbUT::RawData<> m_data;
};

}
