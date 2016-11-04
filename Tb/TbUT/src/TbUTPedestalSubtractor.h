/*
 * TbUTPedestalSubtractor.h
 *
 *  Created on: Oct 14, 2014
 *      Author: ADendek
 */

#pragma once

#include "TbUTIProcessingEngine.h"
#include "TbUTPedestal.h"
#include "TbUTIChannelMaskProvider.h"
#include "TbUTRawData.h"
#include <boost/shared_ptr.hpp>

namespace TbUT
{

class PedestalSubtractor : public IProcessingEngine<>
{
public:
	PedestalSubtractor(Pedestal  & p_pedestalSum,IChannelMaskProvider& p_masksProvider );
	void processEvent(RawData<>* p_data, RawData<> **p_output);

private:

	Pedestal& m_pedestal;
	IChannelMaskProvider& m_masksProvider;
};
}
