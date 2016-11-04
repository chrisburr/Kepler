/*
 * TbUTPedestal.cpp
 *
 *  Created on: Dec 31, 2014
 *      Author: ADendek
 */

#include "TbUTPedestal.h"
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>


using namespace TbUT;
using namespace std;

Pedestal::Pedestal():
		m_normalization(1024),
		m_isNormalized(false)
{
	int l_initialValue=0*m_normalization;// temporary change. until initial value of the pedestal stabilize
	int l_sensorNumber=RawData<>::getnChannelNumber();
	m_pedestals=DataVector(l_sensorNumber,l_initialValue);
}

int Pedestal::getPedestal(int p_channel)
{
	//normalizePedestals();
	return m_pedestals[p_channel];
}

void Pedestal::setPedestal(int p_channel, int p_value)
{
	m_pedestals[p_channel]=p_value;
}

void Pedestal::normalizePedestals()
{
	if(!m_isNormalized){
		int i=0;
		BOOST_FOREACH(auto& l_pedestal, m_pedestals)
		{
			l_pedestal/=m_normalization;
			cout<<"channel: "<<i<<" pedestal: " <<l_pedestal<<endl;
			i++;
			m_isNormalized=true;
		}
	}
}
