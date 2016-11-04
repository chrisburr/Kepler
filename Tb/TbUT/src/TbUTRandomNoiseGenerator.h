/*
 * TbUTRandomNoiseGenerator.h
 *
 *  Created on: Mar 16, 2015
 *      Author: ADendek
 */

#pragma once

#include "TbUTIDataReader.h"
#include <TRandom.h>

namespace TbUT
{

class RandomNoiseGenerator : public IDataReader
{
public:
	RandomNoiseGenerator(double& p_sigma, double& p_mean);
	void checkInput();
	RawData<>* getEventData();

private:
	double& m_sigma;
	double& m_mean;
	TRandom m_randomGenerator;

};

} /* namespace TbUT */

