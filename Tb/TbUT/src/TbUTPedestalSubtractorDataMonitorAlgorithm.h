/*
 * TbUTPedestalDataMonitorAlgorithm.h
 *
 *  Created on: Oct 18, 2014
 *      Author: ADendek
 */
#pragma once


#include "TbUTDataMonitorAlgorithm.h"

namespace TbUT
{

class PedestalSubtractorDataMonitorAlgorithm: public DataMonitorAlgorithm
{
public:
	PedestalSubtractorDataMonitorAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
	StatusCode finalize();

private:
	  TH2D * bookHistogram2D(const std::string & p_histogramName, const std::string & p_histogramTitle, int p_sensorNumber);
	  std::string createHistogramTitle();
	  void createHistogram2D();
	  std::string createHistogramName();

};
}


