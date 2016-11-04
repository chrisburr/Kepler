/*
 * TbUTClusterCreatorFactory.h
 *
 *  Created on: Jan 6, 2015
 *      Author: ADendek
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include "TbUTITresholdProvider.h"
#include "TbUTIClusterCreator.h"

namespace TbUT
{

namespace ClusterCreatorType
{
static const std::string& defaultCreator = "default";
}

class ClusterCreatorFactory
{
public:
	typedef boost::shared_ptr<IClusterCreator> ClusterCreatorPtr;

	ClusterCreatorFactory(const std::string&  p_sensorType, ITresholdProvider & p_tresholds);
	ClusterCreatorPtr createClusterCreator(const std::string& p_clusterCreatorType);

	class NoSuchState: public std::runtime_error
	{
	public:
		NoSuchState(const std::string& p_errorMsg ):
			std::runtime_error(p_errorMsg)
		{
		}
	};
private:
	const std::string&  m_sensorType;
	ITresholdProvider& m_thresholdPrivder;
};

} /* namespace TbUT */

