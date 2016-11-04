/*
 * TbUTSensor.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: ADendek
 */

#include "TbUTSensor.h"

using namespace TbUT;


Sensor::Sensor():
		m_channelMax(0),
		m_channelMin(0)
{
	setChannelsNumbers(1);
}

Sensor::Sensor(int p_sensorNumber):
		m_sensorNumber(p_sensorNumber),
		m_channelMax(0),
		m_channelMin(0)
{
	setChannelsNumbers(p_sensorNumber);
}


void Sensor::setChannelsNumbers(int p_number)
{
	m_channelMin = 0;
	m_channelMax=512;

	int l_maxSensorNumber=4;
	if(p_number>l_maxSensorNumber){
		std::string l_errMsg="Invalid Sensor Number: "+std::to_string(p_number);
		throw SensorNumberError(l_errMsg);
	}
}
