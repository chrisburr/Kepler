/*
 * CMSLinear.cpp
 *
 *  Created on: Nov 23, 2014
 *      Author: ADendek
 */

#include "TbUTCMSLinear.h"
#include <iostream>

using namespace TbUT;
using namespace std;

CMSLinear::CMSLinear(IChannelMaskProvider& p_masksProvider):
		m_masksProvider(p_masksProvider),
		m_channelNumber(RawData<>::getnChannelNumber())
{
}
void CMSLinear::processEvent(RawData<>* p_data, RawData<double> **p_output)
{
	double l_correction = calculateCorrection(p_data);
	for(int channel=0;channel<m_channelNumber;channel++){
		removeCM(p_data,p_output,channel,l_correction);
	}
}

double CMSLinear::calculateCorrection(RawData<>* p_inputData)
{
	double correction=0;
	int usedChannels=0;
	for(int channel=0;channel<m_channelNumber;channel++){
		if(!m_masksProvider.isMasked(channel)){
			correction+=p_inputData->getSignal(channel);
			usedChannels++;
		}
	}
	if(usedChannels) correction/=static_cast<double>(usedChannels);
	return correction;
}


void CMSLinear::removeCM(RawData<>* p_data, RawData<double> **p_output, int p_channel, double p_correction)
{
	if(m_masksProvider.isMasked(p_channel))
	{
		double l_signalMaskedChannel=0;
		(*p_output)->setSignal(l_signalMaskedChannel);
	}else{
		double l_channelSignal=p_data->getSignal(p_channel)-p_correction;
				(*p_output)->setSignal(l_channelSignal);
	}

}
