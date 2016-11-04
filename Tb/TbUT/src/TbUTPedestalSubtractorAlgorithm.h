/*
 * TbUTPedestalSubtractorAlgorithm.h
 *
 *  Created on: Oct 14, 2014
 *      Author: ADendek
 */

#pragma once
#include "GaudiAlg/GaudiAlgorithm.h"

#include "TbUTRawData.h"
#include "TbUTPedestalFollowingFactory.h"
#include "TbUTIPedestalFollowing.h"
#include "TbUTPedestalSubtractor.h"
#include "TbUTChannelMaskProvider.h"
#include "TbUTChannelMaskFileValidator.h"
#include "TbUTPedestal.h"
#include "TbUTPedestalFileValidator.h"


#include <string>
#include <boost/shared_ptr.hpp>

namespace TbUT
{

class PedestalSubtractorAlgorithm: public GaudiAlgorithm
{
	typedef boost::shared_ptr<IPedestalFollowing> PedestalFollowingPtr;

	enum RunPhase
	{
		 SKIPP,
		 TREANING,
		SUBTRACTION	
	};

public:
	PedestalSubtractorAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

	virtual StatusCode initialize();
	virtual StatusCode execute();
	virtual StatusCode finalize();

private:

	StatusCode initializeBase();
	StatusCode buildFollowing();
	StatusCode retriveMasksFromFile();

	void skippEvent();
	void processTreaning();
	void subtractPedestals();
	
	StatusCode getData();
	void processAndSaveDataToTES();
	StatusCode savePedestalsToFile();

	RunPhase getRunPhase();

	bool m_isStandalone;
	RawDataContainer<>* m_dataContainer;
	RawData<>* m_data;
	RawDataContainer<>* m_outputDataContainer;

	std::string m_inputDataLocation;
	std::string m_outputDataLocation;
	std::string m_pedestalInputLocation;
	std::string m_pedestalOutputLocation;
	std::string m_channelMaskInputLocation;
	std::string m_followingOption;
	int m_event;
	int m_treningEventNumber;
	int m_skippEvent;

	ChannelMaskFileValidator m_channelMaskFileValidator;
	ChannelMaskProvider m_channelMaskProvider;
	Pedestal m_pedestal;
	PedestalFileValidator m_pedestalFileValidator;
	PedestalFollowingFactory m_followingFactory;
	PedestalFollowingPtr m_pedestalFollowingPtr;
	PedestalSubtractor m_pedestalSubtractor;

};

} 

