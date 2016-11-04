/*
 * TbUTPedestal.h
 *
 *  Created on: Dec 31, 2014
 *      Author: ADendek
 */

#pragma once

#include "TbUTRawData.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <iostream>


namespace TbUT
{

class Pedestal
{
	typedef std::vector<int> DataVector;

public:
	Pedestal();
	 void setPedestal(int p_channel, int p_value);
	 int  getPedestal(int p_channel);
	 int & operator[](size_t el) {return m_pedestals[el];}
	 DataVector& getPedestalVector()
	 {
		 normalizePedestals();
		 return m_pedestals;
	 }
	 void normalizePedestals();
	 void setNormalizationFlag(bool p_flag){m_isNormalized=p_flag;}
private:

	int m_normalization;
	bool m_isNormalized;
	DataVector m_pedestals;

};

} /* namespace TbUT */
