/*
 * TbUTTresholdProvider.h
 *
 *  Created on: Jan 4, 2015
 *      Author: ADendek
 */

#pragma once

#include "TbUTITresholdProvider.h"
#include "TbUTNoise.h"
#include  <string>

namespace TbUT
{

class TresholdProvider: public ITresholdProvider
{
public:
	TresholdProvider(const std::string& p_noiseFile,
			const double& p_lowThresholdMultiplicity,
			const double& p_highThresholdMultiplicity );

	void retreiveTresholds();
	double getLowClusterThreshold(int p_channel);
	double getHighClusterThreshold(int p_channel);
private:
	const std::string& m_noiseFile;
	const double& m_lowThresholdMultiplicity;
	const double& m_highThresholdMultiplicity;
	Noise m_noise;
};

} /* namespace TbUT */

