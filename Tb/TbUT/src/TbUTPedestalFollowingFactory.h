/*
 * TbUTPedestalFollowingfactory.h
 *
 *  Created on: Oct 10, 2014
 *      Author: ADendek
 */

#pragma once
#include "TbUTIPedestalFollowing.h"
#include "TbUTPedestal.h"
#include "TbUTIChannelMaskProvider.h"
#include "TbUTIFileValidator.h"

#include <string>

namespace TbUT
{
namespace FollowingOptions
{
static const std::string& Calculator = "calculator";
static const std::string& File = "file";
}

class PedestalFollowingFactory
{
public:
	PedestalFollowingFactory(IChannelMaskProvider& p_chanelMaskProvider,Pedestal &p_pedestal, IFileValidator& p_fileValidator,const std::string& p_filename  );
	IPedestalFollowing* createPedestalFollowing(const std::string& p_followingType);
	class NoSuchState: public std::runtime_error
		{
		public:
		NoSuchState(const std::string& p_errorMsg ):
					std::runtime_error(p_errorMsg)
			{
			}
		};
private:
	IChannelMaskProvider& m_chanelMaskProvider;
	Pedestal &m_pedestal;
	IFileValidator& m_fileValidator;
	const std::string& m_filename;

};
}
