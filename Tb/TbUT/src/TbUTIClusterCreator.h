/*
 * TbUTIClusterCreator.h
 *
 *  Created on: Jan 5, 2015
 *      Author: ADendek
 */

#pragma once

#include "TbUTCluster.h"
#include "TbUTRawData.h"

namespace TbUT
{

class IClusterCreator
{
public:
	virtual ~IClusterCreator(){};
	virtual ClusterContainer::ClusterVector createClusters(RawData<double> *p_inputData)=0;

	class ClusterCreatorError: public std::runtime_error
	{
	public:
		ClusterCreatorError(std::string &msg) :
			std::runtime_error(msg)
		{
		}
	};

};

} /* namespace TbUT */

