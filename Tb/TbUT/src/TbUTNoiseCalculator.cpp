/*
 * TbUTNoiseCalculator.cpp
 *
 *  Created on: Jan 3, 2015
 *      Author: ADendek
 */

#include "TbUTNoiseCalculator.h"

using namespace TbUT;


NoiseCalculator::NoiseCalculator()
{
}

void NoiseCalculator::updateNoise(RawData<double>* p_inputData)
{
	m_noise.updateNoise(p_inputData);
}

void NoiseCalculator::saveNoiseToFile(const std::string& p_filaname)
{
	m_noise.saveNoiseToFile(p_filaname);
}

