/*
 * TbUTNoiseCalculatorfactory.h
 *
 *  Created on: Jan 3, 2015
 *      Author: ADendek
 */
#pragma once

#include "TbUTINoiseCalculator.h"
#include <string>
#include <boost/shared_ptr.hpp>

namespace TbUT
{

namespace NoiseCalculatorType
{
static const std::string& calculator="calculator";
static const std::string& fake="fake";
}

class NoiseCalculatorFactory
{
public:
	typedef boost::shared_ptr<INoiseCalculator> NoiseCalcualtorPtr;
	NoiseCalculatorFactory();
	NoiseCalcualtorPtr createNoiseCalculator(const std::string& p_noiseCalculatorType);

	class NoSuchState: public std::runtime_error
		{
		public:
			NoSuchState(const std::string& p_errorMsg ):
				std::runtime_error(p_errorMsg)
			{
			}
		};

};

} /* namespace TbUT */

