/*
 * TbUTNoiseCalculatorFake.h
 *
 *  Created on: Jan 3, 2015
 *      Author: ADendek
 */

#pragma once

#include "TbUTINoiseCalculator.h"

namespace TbUT
{

class NoiseCalculatorFake: public INoiseCalculator
{
public:
	NoiseCalculatorFake();
	void updateNoise(RawData<double>* p_inputData);
	void saveNoiseToFile(const std::string& p_filaname);
};

} /* namespace TbUT */

