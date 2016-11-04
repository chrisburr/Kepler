/*
 * TbUTCommonModeSubtractorFactory.h
 *
 *  Created on: Nov 24, 2014
 *      Author: ADendek
 */

#pragma once

#include "TbUTCMSLinear.h"
#include "TbUTCmsPerBeetle.h"
#include "TbUTCMSIterativelyPerBeetle.h"

namespace TbUT
{

namespace CMSType
{
static const std::string& Iteratively="Iteratively";
static const std::string& Beetle = "Beetle";
static const std::string& Linear = "Linear";
}

class CommonModeSubtractorFactory
{
public:
	CommonModeSubtractorFactory(IChannelMaskProvider& p_masksProvider);

	ICommonModeSubtractor* createCMSubtractor(const std::string& p_CMSType);

	class NoSuchState: public std::runtime_error
	{
	public:
		NoSuchState(const std::string& p_errorMsg ):
			std::runtime_error(p_errorMsg)
		{
		}
	};
private:
	IChannelMaskProvider& m_masksProvider;
};

}

