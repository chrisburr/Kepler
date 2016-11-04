/*
 * PedestalFollowingFactory.cpp
 *
 *  Created on: Oct 10, 2014
 *      Author: ADendek
 */


#include "TbUTPedestalFollowingFactory.h"
#include "TbUTPedestalCalculator.h"
#include "TbUTPedestalRetreiver.h"

using namespace TbUT;

PedestalFollowingFactory::PedestalFollowingFactory(IChannelMaskProvider& p_chanelMaskProvider,Pedestal &p_pedestal , IFileValidator& p_fileValidator, const std::string& p_filename):
		m_chanelMaskProvider(p_chanelMaskProvider),
		m_pedestal(p_pedestal),
		m_fileValidator(p_fileValidator),
		m_filename(p_filename)
{
}


IPedestalFollowing* PedestalFollowingFactory::createPedestalFollowing(const std::string& p_followingType)
{
	if(!p_followingType.compare(TbUT::FollowingOptions::Calculator.c_str()))
		return new PedestalCalculator(m_chanelMaskProvider, m_pedestal);
	else if(!p_followingType.compare(TbUT::FollowingOptions::File.c_str()))
		return new PedestalRetreiver(m_pedestal,m_fileValidator,m_filename);
	else
		throw NoSuchState(p_followingType);
}
