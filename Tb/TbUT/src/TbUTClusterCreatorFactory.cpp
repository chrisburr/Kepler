/*
 * TbUTClusterCreatorFactory.cpp
 *
 *  Created on: Jan 6, 2015
 *      Author: ADendek
 */

#include "TbUTClusterCreatorFactory.h"
#include "TbUTClusterCreator.h"

using namespace TbUT;


ClusterCreatorFactory::ClusterCreatorFactory(const std::string&  p_sensorType, ITresholdProvider & p_tresholds):
	m_sensorType(p_sensorType),
	m_thresholdPrivder(p_tresholds)
{
}

ClusterCreatorFactory::ClusterCreatorPtr ClusterCreatorFactory::createClusterCreator(const std::string& p_clusterCreatorType)
{
	if(p_clusterCreatorType == TbUT::ClusterCreatorType::defaultCreator)
		return ClusterCreatorPtr(new ClusterCreator(m_sensorType,m_thresholdPrivder));
	else
		throw NoSuchState(p_clusterCreatorType);
}
