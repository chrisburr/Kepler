/*
 * TbUTIIChannelMaskProvider.h
 *
 *  Created on: Oct 10, 2014
 *      Author: ADendek
 */

#pragma once
#include <vector>
#include <string>
#include "TbUTDataLocations.h"


namespace TbUT
{
class IChannelMaskProvider{
public:
	virtual ~IChannelMaskProvider(){};
	virtual void getMaskFromFile(const std::string& p_filename=TbUT::DataLocations::MaskLocation)=0;
	virtual bool isMasked(int p_channel)=0;
};
}
