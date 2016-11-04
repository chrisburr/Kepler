/*
 * TbUTSensor.h
 *
 *  Created on: Feb 6, 2015
 *      Author: ADendek
 */

#pragma once
#include <string>
#include <stdexcept>

namespace TbUT
{

class Sensor
{
public:
	class SensorNumberError: public std::runtime_error
	{
	public:
		SensorNumberError(std::string &msg) :
					std::runtime_error(msg)
			{
			}
		};
	Sensor();
	Sensor(int p_sensorNumber);

	static const int channelNumber=512;
	int minChannel() const {return m_channelMin;}
	int maxChannel() const {return m_channelMax;}
	int sensorNumber() const {return m_sensorNumber;}

private:
	void setChannelsNumbers(int p_sensorNumber);

	int m_sensorNumber;
	int m_channelMax;
	int m_channelMin;

};

}
