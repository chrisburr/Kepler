/*
 * TbUTPedestalCalculator.h
 *
 *  Created on: Oct 10, 2014
 *      Author: ADendek
 */

#pragma once
#include "TbUTIPedestalFollowing.h"
#include "TbUTIChannelMaskProvider.h"
#include "TbUTPedestal.h"
#include <stdexcept>

namespace TbUT
{
class PedestalCalculator: public IPedestalFollowing
{
	enum RunPhase
	{
		CALCULATE_INITIAL_VALUE,
		NORMALIZE_INITIAL_VALUE,
		CALUCLATE_PEDESTAL
	};

public:
	PedestalCalculator(IChannelMaskProvider& p_maskProvider, Pedestal & p_pedestal);
	StatusCode processEvent(RawData<>* p_data);
	void savePedestalToFile(const std::string& p_filename);

private:

	StatusCode calculateInitialValue(RawData<>* p_data);
	StatusCode normalizeInitialValue();
	StatusCode calculaPedestal(RawData<>* p_data);

	RunPhase getRunPhase() const;

	double calculateUpdate(int p_channel, RawData<>::SignalVector&  p_data);

	IChannelMaskProvider& m_maskProvider;
	Pedestal & m_pedestal;
	int m_normalization;
	int m_event;
	const int m_calculateInitialValueEvents;
};
}
