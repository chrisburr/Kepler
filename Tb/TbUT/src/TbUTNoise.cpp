/*
 * TbUTNoise.cpp
 *
 *  Created on: Jan 3, 2015
 *      Author: ADendek
 */

#include "TbUTNoise.h"
#include <boost/foreach.hpp>
#include <iostream>
#include <fstream>
#include <cmath>

using namespace TbUT;
using namespace std;

Noise::Noise():
		m_hitLimit(160)
{
	Reset();
}

void Noise::updateNoise(RawData<double>* p_inputData)
{
	int l_channelNumber =RawData<>::getnChannelNumber();
	const int channelPerBeetle=32;
	for(int channel=0;channel<l_channelNumber;channel++)
	{
		if(0 == (channel%channelPerBeetle) ) m_hitLimit= calcualteHitThreshold(p_inputData, channel);
		int signal=p_inputData->getSignal(channel);
		if(abs(signal)<m_hitLimit){
			double signalSquare=signal*signal;
			m_noiseVector[channel]+=signalSquare;
			m_channelEnties[channel]++;
			m_meanVector[channel]+=signal;
		}
	}
}

void Noise::saveNoiseToFile(const std::string& p_filename)
{
	ofstream l_file(p_filename);
	if(!l_file.good())
	{
		std::string l_errorMsg="Saving Noise to file- Cannot open output file: "+p_filename;
		throw  NoiseCalculatorError(l_errorMsg);
	}
	NormalizeNoise();
	int l_channelsNumber=RawData<>::getnChannelNumber();
	for(int channel=0;channel<l_channelsNumber;channel++)
	{
		auto l_noise=m_noiseVector[channel];
		l_file<<std::to_string(l_noise)<<" ";
	}
	l_file.close();
}

void Noise::retreiveNoiseFromFile(const std::string& p_filename)
{
		int l_channelsNumber=RawData<>::getnChannelNumber();
		ifstream l_file(p_filename);
		if(!l_file.good()){
			std::string l_errorMsg="Cannot open input noise file: "+p_filename;
			throw NoiseCalculatorError(l_errorMsg);
		}

		for(int channel=0;channel<l_channelsNumber;channel++)
		{
			double l_noiseFromFile=0;
			l_file >> l_noiseFromFile;
			m_noiseVector[channel]=l_noiseFromFile;
			cout<<"NoiseRetreiver===> channel: "<< channel <<"noise: "<<l_noiseFromFile<<endl;
		}

}

double Noise::calcualteHitThreshold(RawData<double>* p_inputData,int channelBegin) const
{
	const int channelPerBeetle=32;
	double rms=0;
	double mean=0;
	const double initialHitLimit=160;
	int usedChannel=0;

	for (int channel=channelBegin;channel<channelBegin+channelPerBeetle;channel++)
	{
		auto channelSignal=p_inputData->getSignal(channel);
		if( (0. != channelSignal ) && (abs(channelSignal)<initialHitLimit) ){
			rms+=channelSignal*channelSignal;
			mean+=channelSignal;
			usedChannel++;
		}

	}
	if(usedChannel) rms/=static_cast<double>(usedChannel);
	if(usedChannel) mean/=static_cast<double>(usedChannel);
	rms-=mean*mean;
	double rmsMultiplicity=4;
	return rmsMultiplicity*sqrt(rms);
}


void Noise::NormalizeNoise()
{
	int l_channelsNumber=RawData<>::getnChannelNumber();
	for(int channel=0;channel<l_channelsNumber;channel++)
		{
			auto normalizationFactor=m_channelEnties[channel];
			if(normalizationFactor)m_noiseVector[channel]/= static_cast<double>(normalizationFactor);
			if(normalizationFactor)m_meanVector[channel]/=static_cast<double>(normalizationFactor);
			m_noiseVector[channel]-=m_meanVector[channel]*m_meanVector[channel];
			m_noiseVector[channel]=sqrt(m_noiseVector[channel]);
		}
}

void Noise::Reset()
{
	int l_initialValue=0;
	int l_sensorNumber=RawData<>::getnChannelNumber();
	m_channelEnties=ChannelEntries(l_sensorNumber,l_initialValue);
	m_noiseVector=NoiseVector(l_sensorNumber,l_initialValue);
	m_meanVector=NoiseVector(l_sensorNumber,l_initialValue);

}


