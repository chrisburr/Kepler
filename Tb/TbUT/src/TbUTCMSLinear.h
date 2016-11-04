/*
 * CMSLinear.h
 *
 *  Created on: Nov 23, 2014
 *      Author: ADendek
 */

#pragma once

#include "TbUTICommonModeSubtractor.h"
#include "TbUTIChannelMaskProvider.h"

namespace TbUT
{

class CMSLinear: public ICommonModeSubtractor
{
public:
	CMSLinear(IChannelMaskProvider& p_masksProvider );
	void processEvent(RawData<>* p_data, RawData<double> **p_output);

private:
	double calculateCorrection(RawData<>* p_inputData);
	void removeCM(RawData<>* p_data, RawData<double> **p_output, int p_channel, double p_correction);

	IChannelMaskProvider& m_masksProvider;
	int m_channelNumber;
};

}

