/*
 * TbUTCommonModeSubtractorFactory.cpp
 *
 *  Created on: Nov 24, 2014
 *      Author: ADendek
 */

#include "TbUTCommonModeSubtractorFactory.h"

using namespace TbUT;


CommonModeSubtractorFactory::CommonModeSubtractorFactory(IChannelMaskProvider& p_masksProvider):
		m_masksProvider(p_masksProvider)
{
}

ICommonModeSubtractor* CommonModeSubtractorFactory::createCMSubtractor(const std::string& p_CMSType)
{
	if(!p_CMSType.compare(TbUT::CMSType::Linear.c_str()))
		return new CMSLinear(m_masksProvider);
	else if (p_CMSType == TbUT::CMSType::Beetle){
		double hitThreshold=160.;
		return new CmsPerBeetle(m_masksProvider,hitThreshold);
	}
	else if(p_CMSType == TbUT::CMSType::Iteratively)
		return new CMSIterativelyPerBeetle(m_masksProvider);
	else
		throw NoSuchState(p_CMSType);
}
