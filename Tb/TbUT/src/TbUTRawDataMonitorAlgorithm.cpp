/*
 * TbUTRawDataMonitorAlgorithm.cpp
 *
 *  Created on: Oct 7, 2014
 *      Author: ADendek
 */
#include "TbUTRawDataMonitorAlgorithm.h"
#include "TbUTDataLocations.h"

DECLARE_NAMESPACE_ALGORITHM_FACTORY(TbUT,RawDataMonitorAlgorithm)

using namespace TbUT;

RawDataMonitorAlgorithm::RawDataMonitorAlgorithm( const std::string& name,ISvcLocator* pSvcLocator)
  : DataMonitorAlgorithm ( name , pSvcLocator )
{
	RawDataMonitorAlgorithm::m_inputDataLoc=TbUT::DataLocations::RawTES;
}



StatusCode RawDataMonitorAlgorithm::finalize()
{
	DataMonitorAlgorithm::m_outpuProjectionHistogramName="ProjectionRawData";
	return DataMonitorAlgorithm::finalize();
}
