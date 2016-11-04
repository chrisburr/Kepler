/*
 * TbUTRawDataMonitorAlgorithm.h
 *
 *  Created on: Oct 7, 2014
 *      Author: ADendek
 */

#pragma once

#include "TbUTDataMonitorAlgorithm.h"

namespace TbUT
{

class RawDataMonitorAlgorithm : public DataMonitorAlgorithm
{
public:
	RawDataMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
	StatusCode finalize  ();
};
}
