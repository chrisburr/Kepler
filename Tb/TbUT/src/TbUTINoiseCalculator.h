/*
 * TbUTINoiseCalculator.h
 *
 *  Created on: Jan 3, 2015
 *      Author: ADendek
 */

#pragma once

#include "TbUTRawData.h"

namespace TbUT{

class INoiseCalculator
{
public:
	virtual ~INoiseCalculator(){};
	virtual void updateNoise(RawData<double>* p_inputData)=0;
	virtual void saveNoiseToFile(const std::string& p_filaname)=0;
};
}
