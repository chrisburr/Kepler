/*
 * TbUTIPedestalFollowing.h
 *
 *  Created on: Oct 10, 2014
 *      Author: ADendek
 */

#pragma once
#include "TbUTRawData.h"

#include <vector>
#include <string>

namespace TbUT
{
class IPedestalFollowing
{
public:
	virtual ~IPedestalFollowing(){};

	virtual StatusCode processEvent(RawData<>* p_data)=0;
	virtual void savePedestalToFile(const std::string& p_filename)=0;

	class PedestalCalculatorError: public std::runtime_error
		{
		public:
		PedestalCalculatorError(std::string &msg) :
					std::runtime_error(msg)
			{
			}
		};
};
}
