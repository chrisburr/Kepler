/*
 * TbUTNoise.h
 *
 *  Created on: Jan 3, 2015
 *      Author: ADendek
 */

#pragma once
#include "TbUTRawData.h"
#include <vector>


namespace TbUT
{

class Noise
{
	typedef std::vector<double> NoiseVector;
	typedef std::vector<int> ChannelEntries;
public:
	Noise();
	void updateNoise(RawData<double>* p_inputData);
	void saveNoiseToFile(const std::string& p_filename);
	void retreiveNoiseFromFile(const std::string& p_filename);

	double calcualteHitThreshold(RawData<double>* p_inputData, int channelBegin) const;

	double getNoise(int p_channel) const{return m_noiseVector[p_channel];}
	double operator[](int p_channel) const {return getNoise(p_channel);}

	void NormalizeNoise();
	void Reset();

	class NoiseCalculatorError: public std::runtime_error
	{
	public:
		NoiseCalculatorError(std::string &msg) :
					std::runtime_error(msg)
			{
			}
		};

private:

	int m_hitLimit;
	ChannelEntries m_channelEnties;
	NoiseVector m_noiseVector;
	NoiseVector m_meanVector;
};

}
