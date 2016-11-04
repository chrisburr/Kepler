/*
 * TbUTPedestalCalculator.cpp
 *
 *  Created on: Oct 10, 2014
 *      Author: ADendek
 */

#include "TbUTPedestalCalculator.h"
#include <boost/serialization/serialization.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <fstream>


using namespace TbUT;
using namespace std;

PedestalCalculator::PedestalCalculator(IChannelMaskProvider& p_maskProvider, Pedestal & p_pedestal):
		m_maskProvider(p_maskProvider),
		m_pedestal(p_pedestal),
		m_normalization(1024),
		m_event(0),
		m_calculateInitialValueEvents(100)
{
}

void PedestalCalculator::savePedestalToFile(const std::string& p_filename)
{
	ofstream l_file(p_filename);
	if(!l_file.good())
	{
		std::string l_errorMsg="Saving Pedestal to file- Cannot open output file: "+p_filename;
		throw  PedestalCalculatorError(l_errorMsg);
	}
	BOOST_FOREACH(auto l_pedestal, m_pedestal.getPedestalVector())
	{
		l_file<<std::to_string(l_pedestal)<<" ";
	}
	l_file.close();
}

StatusCode PedestalCalculator::processEvent(RawData<>* p_data)
{
	RunPhase l_runPhase=getRunPhase();
	switch(l_runPhase)
	{
		case CALCULATE_INITIAL_VALUE:
			return	calculateInitialValue(p_data);
		case NORMALIZE_INITIAL_VALUE:
			return normalizeInitialValue();
		case CALUCLATE_PEDESTAL:
			return calculaPedestal(p_data);
		default:
			string l_errorMsg="No such Phase!";
			throw  PedestalCalculatorError(l_errorMsg);
	}
}

PedestalCalculator::RunPhase PedestalCalculator::getRunPhase() const
{
	if(m_event<m_calculateInitialValueEvents)
		return	CALCULATE_INITIAL_VALUE;
	else if (m_event==m_calculateInitialValueEvents)
		return NORMALIZE_INITIAL_VALUE;
	else
		return CALUCLATE_PEDESTAL;
}

StatusCode PedestalCalculator::calculateInitialValue(RawData<>* p_data)
{
	RawData<>::SignalVector l_inputData=p_data->getSignal();
	int l_channelNumber=RawData<>::getnChannelNumber();
	for(int channel=0;channel<l_channelNumber;channel++)
		m_pedestal[channel]+=l_inputData[channel]*m_normalization;
	m_event++;
	return StatusCode::SUCCESS;
}

StatusCode PedestalCalculator::normalizeInitialValue()
{
	int l_channelNumber=RawData<>::getnChannelNumber();
	for(int channel=0;channel<l_channelNumber;channel++){
		m_pedestal[channel]/=m_calculateInitialValueEvents;
		std::cout<<"Initial value of the Pedestal: channel " << channel << " value: "<<m_pedestal[channel]<< std::endl;
	}
	m_event++;
	return StatusCode::SUCCESS;
}

StatusCode PedestalCalculator::calculaPedestal(RawData<>* p_data)
{
	RawData<>::SignalVector l_inputData=p_data->getSignal();
	int l_channelNumber=RawData<>::getnChannelNumber();
	for(int channel=0;channel<l_channelNumber;channel++)
		m_pedestal[channel]+=calculateUpdate(channel, l_inputData);
	m_event++;

	return StatusCode::SUCCESS;
}

double PedestalCalculator::calculateUpdate(int p_channel, RawData<>::SignalVector&  p_data )
{
	double l_update=0;
	double l_saturation=15;
	if(m_maskProvider.isMasked(p_channel))
		l_update=0;
	else
	{
		l_update=p_data[p_channel]-(m_pedestal[p_channel]/m_normalization);
		if(l_update>l_saturation)l_update=l_saturation;
		else if (l_update <(-1)*l_saturation) l_update=(-1)*l_saturation;
	}
	return l_update;
}


