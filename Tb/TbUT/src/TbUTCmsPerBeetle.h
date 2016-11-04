//
// Created by ja on 8/4/15.
//
#pragma once

#include "TbUTICommonModeSubtractor.h"
#include "TbUTIChannelMaskProvider.h"

#include <map>

namespace TbUT
{

class CmsPerBeetle: public ICommonModeSubtractor
{
public:
	CmsPerBeetle(IChannelMaskProvider& p_masksProvider,double p_hitThreshold);
	void processEvent(RawData<>* p_data, RawData<double> **p_output);
private:
	void calculateCorrection(RawData<>* p_inputData);
	void removeCM(RawData<>* p_data, RawData<double> **p_output);

	void initializeCorrectionMap();
	IChannelMaskProvider& m_masksProvider;
	int m_channelNumber;
	double m_hitThreshold;
	std::map<int, double> m_correctionPerBeetle;
};

}
