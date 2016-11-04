#include "TbUTCMSIterativelyPerBeetle.h"
#include <iostream>
#include <cmath>

using namespace TbUT;
using namespace std;

CMSIterativelyPerBeetle::CMSIterativelyPerBeetle(IChannelMaskProvider& p_masksProvider):
        m_masksProvider(p_masksProvider),
        m_channelNumber(RawData<>::getnChannelNumber()),
		m_channelPerBeetle(32)
{
    cout<<"create iteratively correlation per beetle"<<endl;
    initializeCorrectionAndHitMaps();
}


void CMSIterativelyPerBeetle::initializeCorrectionAndHitMaps()
{
	for (int channel=0;channel<m_channelNumber;channel+=m_channelPerBeetle){
		m_correctionPerBeetle.insert(std::make_pair(channel,0));
		m_hitThresholdPerBeetle.insert(std::make_pair(channel,200));
	}
}

void CMSIterativelyPerBeetle::processEvent(RawData<>* p_data, RawData<double> **p_output)
{
	int numberOfIteration=2;
	RawData<double>* tmpData;
	// first iteration
	calculateCorrection<int>(p_data);
	removeCM<int, double>(p_data,p_output);
	// rest of the iterations
	for(int iteration=1;iteration<numberOfIteration; iteration++){
		tmpData=new RawData<double>(**p_output);
		calculateCorrection<double>(tmpData);
		removeCM<double, double>(tmpData,p_output);
	}
	resetHitThresholds();
}

template<typename DATA_TYPE>
void CMSIterativelyPerBeetle::calculateCorrection(RawData<DATA_TYPE>* p_inputData)
{
	for(auto& mapIt : m_correctionPerBeetle)
	{
		int usedChannels=0;
		double rmsPerBeetle=0;
		for(int channel=mapIt.first;channel<mapIt.first+m_channelPerBeetle;channel++)
		{
			double signal=static_cast<double>(p_inputData->getSignal(channel));
			if(!m_masksProvider.isMasked(channel) && abs(signal) <m_hitThresholdPerBeetle[mapIt.first]){
				mapIt.second+=signal;
				rmsPerBeetle+=signal*signal;
				usedChannels++;
			}
		}
		if(usedChannels) mapIt.second/=static_cast<double>(usedChannels);
		if(usedChannels) rmsPerBeetle/=static_cast<double>(usedChannels);
		rmsPerBeetle-=mapIt.second*mapIt.second;
		double rmsMultiplicity=4;
		m_hitThresholdPerBeetle[mapIt.first]=rmsMultiplicity*sqrt(rmsPerBeetle);
	}
}

template<typename INPUT_DATA_TYPE, typename OUTPUT_TYPE_NAME>
void CMSIterativelyPerBeetle::removeCM(RawData<INPUT_DATA_TYPE>* p_data, RawData<OUTPUT_TYPE_NAME> **p_output)
{
	for(auto& mapIt : m_correctionPerBeetle)
	{
		for(int channel=mapIt.first;channel<mapIt.first+m_channelPerBeetle;channel++)
		{
			if(m_masksProvider.isMasked(channel)){
				OUTPUT_TYPE_NAME signalMaskedChannel=0;
				(*p_output)->setSignal(signalMaskedChannel);
			}else{
				OUTPUT_TYPE_NAME l_channelSignal=p_data->getSignal(channel)-mapIt.second;
				(*p_output)->setSignal(l_channelSignal);
			}
		}

	}
	 (*p_output)->setTDC(p_data->getTDC());
	 (*p_output)->setTime(p_data->getTime());

}

void CMSIterativelyPerBeetle::resetHitThresholds()
{
	for(auto& mapIt : m_hitThresholdPerBeetle) mapIt.second=200;
}
