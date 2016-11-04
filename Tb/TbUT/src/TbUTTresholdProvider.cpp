/*
 * TbUTTresholdProvider.cpp
 *
 *  Created on: Jan 4, 2015
 *      Author: ADendek
 */

#include "TbUTTresholdProvider.h"

using namespace TbUT;

TresholdProvider::TresholdProvider(const std::string& p_noiseFile,
		const double& p_lowThresholdMultiplicity,
		const double& p_highThresholdMultiplicity):
		m_noiseFile(p_noiseFile),
		m_lowThresholdMultiplicity(p_lowThresholdMultiplicity),
		m_highThresholdMultiplicity(p_highThresholdMultiplicity),
		m_noise()
{
}

void TresholdProvider::retreiveTresholds()
try{
	m_noise.retreiveNoiseFromFile(m_noiseFile);
}catch(Noise::NoiseCalculatorError& err){
	throw ThresholdProviderError(err.what());
}

double TresholdProvider::getLowClusterThreshold(int p_channel)
{
	return (m_noise[p_channel]*m_lowThresholdMultiplicity);
}

double TresholdProvider::getHighClusterThreshold(int p_channel)
{
	return (m_noise[p_channel]*m_highThresholdMultiplicity);
}
