/*
 * TbUTCommonModeSubtractorDataMonitorAlgorithm.h
 *
 *  Created on: Nov 26, 2014
 *      Author: ADendek
 */

#pragma once

#include "TbUTDataMonitorAlgorithm.h"
#include "TbUTNoise.h"
#include <map>

namespace TbUT
{

class CommonModeSubtractorDataMonitorAlgorithm : public DataMonitorAlgorithm
{
public:
	CommonModeSubtractorDataMonitorAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
	StatusCode execute   ();
	StatusCode finalize();

private:
	 typedef std::map<int, TH1D*> HistogramMap;

	 StatusCode getData();

	 StatusCode saveSimpleEvents();
	 virtual StatusCode fillOnly2DHistogram();
	 
	 void createHistogram2D();
	 TH2D * bookHistogram2D(const std::string & p_histogramName, const std::string & p_histogramTitle, int p_sensorNumber);
	
	 std::string createHistogramTitle();
	 std::string createHistogramName();

	 void fillHistogram2D();
	 void fillHistogram(TH1D * p_histogram);

	 void createNoiseHistograms();
	 void fillNoiseHistograms();
	
	 TbUT::RawDataContainer<double> *m_dataContainer;
  	 TbUT::RawData<double> m_data;

	 Noise m_noise;

	 HistogramMap m_noisePerChannelHistograms;
	 HistogramMap m_noisePerSensorHistograms;


};

}

