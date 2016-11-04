/*
 * TbUTNoiseCalculatorfactory.cpp
 *
 *  Created on: Jan 3, 2015
 *      Author: ADendek
 */

#include "TbUTNoiseCalculatorfactory.h"
#include "TbUTNoiseCalculator.h"
#include "TbUTNoiseCalculatorFake.h"


using namespace TbUT;


NoiseCalculatorFactory::NoiseCalculatorFactory()
{
}

NoiseCalculatorFactory::NoiseCalcualtorPtr NoiseCalculatorFactory::createNoiseCalculator(const std::string& p_noiseCalculatorType)
{
	if(p_noiseCalculatorType==TbUT::NoiseCalculatorType::calculator)
		return NoiseCalcualtorPtr(new NoiseCalculator());
	else if (p_noiseCalculatorType ==TbUT::NoiseCalculatorType::fake)
		return NoiseCalcualtorPtr(new NoiseCalculatorFake());

	else
		throw NoSuchState(p_noiseCalculatorType);
}
