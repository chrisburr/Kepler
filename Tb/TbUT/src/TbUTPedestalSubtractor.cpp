/*
 * TbUTPedestalSubtractor.cpp
 *
 *  Created on: Oct 14, 2014
 *      Author: ADendek
 */

#include"TbUTPedestalSubtractor.h"
#include <iostream>

using namespace TbUT;

PedestalSubtractor::PedestalSubtractor(Pedestal & p_pedestal,IChannelMaskProvider& p_masksProvider ):
		m_pedestal(p_pedestal),
		m_masksProvider(p_masksProvider)
{
}

 void PedestalSubtractor::processEvent(RawData<>* p_data, RawData<> **p_output)
{
	 int l_channelNumber=RawData<>::getnChannelNumber();
	 RawData<>::DataType l_maskedChannelValue=0;
	 for(int channel=0;channel<l_channelNumber;channel++)
	 {
		 if(m_masksProvider.isMasked(channel)) (*p_output)->setSignal(l_maskedChannelValue);
		 else{
			 RawData<>::DataType l_signalAfterPedestal= p_data->getSignal(channel)-m_pedestal.getPedestal(channel);
			 (*p_output)->setSignal(l_signalAfterPedestal);
		 }
	 }
	 (*p_output)->setTDC(p_data->getTDC());
	 (*p_output)->setTime(p_data->getTime());

}


