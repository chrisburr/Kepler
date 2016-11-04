/*
 * TbUTNoiseCalculator.h
 *
 *  Created on: Jan 3, 2015
 *      Author: ADendek
 */

#pragma once

#include "TbUTINoiseCalculator.h"
#include "TbUTNoise.h"


namespace TbUT
{

class NoiseCalculator: public INoiseCalculator
{
public:
	NoiseCalculator();
	void updateNoise(RawData<double>* p_inputData);
	void saveNoiseToFile(const std::string& p_filaname);

private:
	Noise m_noise;

};

}
