/*
 * TbUTRandomNoiseGenerator.cpp
 *
 *  Created on: Mar 16, 2015
 *      Author: ADendek
 */

#include "TbUTRandomNoiseGenerator.h"
#include <iostream>

using namespace TbUT;

RandomNoiseGenerator::RandomNoiseGenerator(double& p_sigma, double& p_mean):
		m_sigma(p_sigma),
		m_mean(p_mean),
		m_randomGenerator()
{
}

void RandomNoiseGenerator::checkInput()
{
}

RawData<>* RandomNoiseGenerator::getEventData()
{
	RawData<>* l_outputNoise=new RawData<>();
	for(int channel=0;channel<RawData<>::getnChannelNumber();channel++)
	{
		double l_randomNoise=m_randomGenerator.Gaus(m_sigma,m_mean);
		l_outputNoise->setSignal(l_randomNoise);
	}
	return l_outputNoise;
}
