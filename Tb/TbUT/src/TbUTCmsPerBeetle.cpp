//
// Created by ja on 8/4/15.
//

#include "TbUTCmsPerBeetle.h"
#include <iostream>
#include <cmath>

using namespace TbUT;
using namespace std;

CmsPerBeetle::CmsPerBeetle(IChannelMaskProvider& p_masksProvider,double p_hitThreshold):
        m_masksProvider(p_masksProvider),
        m_channelNumber(RawData<>::getnChannelNumber()),
		m_hitThreshold(p_hitThreshold)
{
	cout<<"create correlation per beetle"<<endl;
	initializeCorrectionMap();
}

void CmsPerBeetle::initializeCorrectionMap()
{
	int channelPerBeetle=32;
	for (int channel=0;channel<m_channelNumber;channel+=channelPerBeetle)
		m_correctionPerBeetle.insert(std::make_pair(channel,0));
}

void CmsPerBeetle::processEvent(RawData<>* p_data, RawData<double> **p_output)
{
	calculateCorrection(p_data);
	removeCM(p_data,p_output);
}

void CmsPerBeetle::calculateCorrection(RawData<>* p_inputData)
{
	int channelPerBeetle=32;
	for(auto& mapIt : m_correctionPerBeetle)
	{
		int usedChannels=0;
		for(int channel=mapIt.first;channel<mapIt.first+channelPerBeetle;channel++)
		{
			auto signal=p_inputData->getSignal(channel);
			if(!m_masksProvider.isMasked(channel) && abs(signal) <m_hitThreshold){
				mapIt.second+=signal;
				usedChannels++;
			}
		}
		if(usedChannels) mapIt.second/=static_cast<double>(usedChannels);
	}
}

void CmsPerBeetle::removeCM(RawData<>* p_data, RawData<double> **p_output)
{
	int channelPerBeetle=32;
	for(auto& mapIt : m_correctionPerBeetle)
	{
		for(int channel=mapIt.first;channel<mapIt.first+channelPerBeetle;channel++)
		{
			if(m_masksProvider.isMasked(channel)){
				double signalMaskedChannel=0;
				(*p_output)->setSignal(signalMaskedChannel);
			}else{
				double l_channelSignal=p_data->getSignal(channel)-mapIt.second;
				(*p_output)->setSignal(l_channelSignal);
			}
		}

	}

}

