/*
 * TbUTIZSTresholdProvider.h
 *
 *  Created on: Nov 27, 2014
 *      Author: ADendek
 */

#pragma once

#include <string>
#include <stdexcept>

namespace TbUT
{

class ITresholdProvider
{
public:
	virtual ~ITresholdProvider(){}
	virtual void retreiveTresholds()=0;
	virtual double getLowClusterThreshold(int p_channel)=0;
	virtual double getHighClusterThreshold(int p_channel)=0;

	class ThresholdProviderError: public std::runtime_error
	{
	public:
		ThresholdProviderError(const std::string& p_errorMsg ):
					std::runtime_error(p_errorMsg)
		{
		}
	};

};

}
