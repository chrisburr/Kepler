/*
 * TbUTICommonModeSubtractor.h
 *
 *  Created on: Nov 23, 2014
 *      Author: ADendek
 */
#pragma once

#include "TbUTIProcessingEngine.h"


namespace TbUT
{

class ICommonModeSubtractor : public IProcessingEngine<int, double>
{
public:
	virtual ~ICommonModeSubtractor(){};
};

}


