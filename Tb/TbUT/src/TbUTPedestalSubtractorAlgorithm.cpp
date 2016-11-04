/*
 * TbUTPedestalSubtractorAlgorithm.cpp
 *
 *  Created on: Oct 14, 2014
 *      Author: ADendek
 */

#include "TbUTPedestalSubtractorAlgorithm.h"
#include "TbUTDataLocations.h"


using namespace TbUT;

DECLARE_NAMESPACE_ALGORITHM_FACTORY(TbUT,PedestalSubtractorAlgorithm)


PedestalSubtractorAlgorithm::PedestalSubtractorAlgorithm(const std::string& name, ISvcLocator* pSvcLocator):
		GaudiAlgorithm(name, pSvcLocator),
		m_isStandalone(true),
		m_data(),
		m_inputDataLocation(),
		m_outputDataLocation(),
		m_pedestalInputLocation(),
		m_pedestalOutputLocation(),
		m_channelMaskInputLocation(),
		m_followingOption(),
		m_event(0),
		m_treningEventNumber(0),
		m_skippEvent(0),
		m_channelMaskFileValidator(m_channelMaskInputLocation),
		m_channelMaskProvider(m_channelMaskFileValidator),
		m_pedestal(),
		m_pedestalFileValidator(m_pedestalInputLocation),
		m_followingFactory(m_channelMaskProvider, m_pedestal,m_pedestalFileValidator ,m_pedestalInputLocation),
		m_pedestalFollowingPtr(),
		m_pedestalSubtractor(m_pedestal, m_channelMaskProvider)
{
	declareProperty("SkippEventNumber", m_skippEvent=0);
	declareProperty("InputDataLocation", m_inputDataLocation=TbUT::DataLocations::RawTES);
	declareProperty("OutputDataLocation", m_outputDataLocation=TbUT::DataLocations::PedestalTES);
	declareProperty("FollowingOption",m_followingOption=TbUT::FollowingOptions::Calculator);
	declareProperty("treningEntry",m_treningEventNumber=1024);
	declareProperty("ChannelMaskInputLocation", m_channelMaskInputLocation=TbUT::DataLocations::MaskLocation);
	declareProperty("PedestalInputFile", m_pedestalInputLocation=TbUT::DataLocations::PedestalLocation);
	declareProperty("PedestalOutputFile", m_pedestalOutputLocation=TbUT::DataLocations::PedestalLocation);
	declareProperty("standalone",m_isStandalone=true );
}

StatusCode PedestalSubtractorAlgorithm::initialize()
{
	if(StatusCode::SUCCESS !=initializeBase()) return StatusCode::FAILURE;
	if(StatusCode::SUCCESS !=buildFollowing()) return StatusCode::FAILURE;
	if(StatusCode::SUCCESS !=retriveMasksFromFile()) return StatusCode::FAILURE;

	info()<<"Initialized successfully!"<<endmsg;
	return StatusCode::SUCCESS;
}

StatusCode PedestalSubtractorAlgorithm::execute()
{
	if(StatusCode::SUCCESS != getData()) return StatusCode::FAILURE;
	m_outputDataContainer=new RawDataContainer<>();

	if(m_dataContainer->isEmpty()){
		info()<<"ped suba put empty"<<endmsg;
		put(m_outputDataContainer,m_outputDataLocation);
		return StatusCode::SUCCESS;
	}

	for(const auto& rawDataIt: m_dataContainer->getData()){
		m_data=new RawData<>(rawDataIt);
		RunPhase l_runPhase=getRunPhase();
		switch (l_runPhase)
		{
		case SKIPP:
			skippEvent();
		case TREANING:
			processTreaning();
		default:
			subtractPedestals();
		}
		delete m_data;
	}
	put(m_outputDataContainer,m_outputDataLocation);
	return StatusCode::SUCCESS;
}

StatusCode PedestalSubtractorAlgorithm::finalize()
{
	savePedestalsToFile();
	return GaudiAlgorithm::finalize();
}

StatusCode PedestalSubtractorAlgorithm::initializeBase()
{
	  return GaudiAlgorithm::initialize();
}

StatusCode PedestalSubtractorAlgorithm::buildFollowing()
try{
	m_pedestalFollowingPtr.reset(m_followingFactory.createPedestalFollowing(m_followingOption));
	return StatusCode::SUCCESS;
}catch(PedestalFollowingFactory::NoSuchState &exception)
{
	error()<<"Invalid Following Option: "<<exception.what()<<endmsg;
	return StatusCode::FAILURE;
}


StatusCode PedestalSubtractorAlgorithm::retriveMasksFromFile()
try{
	m_channelMaskProvider.getMaskFromFile(m_channelMaskInputLocation);
	return StatusCode::SUCCESS;
}catch(ChannelMaskProvider::InputFileError  &exception)
{
	error()<<"Channel Mask File Input Error: "<<m_channelMaskInputLocation<<endmsg;
	return StatusCode::FAILURE;
}


StatusCode PedestalSubtractorAlgorithm::getData()
{
	m_dataContainer=getIfExists<RawDataContainer<> >(m_inputDataLocation);
	if(!m_dataContainer){
		error()<< " ==> There is no input data: "<< m_inputDataLocation <<endmsg;
		return  StatusCode::FAILURE;
	}
	return  StatusCode::SUCCESS;
}

PedestalSubtractorAlgorithm::RunPhase PedestalSubtractorAlgorithm::getRunPhase()
{
	if(m_event<m_skippEvent)
		return SKIPP;
	else if (m_event<m_skippEvent+m_treningEventNumber)
		return TREANING;
	else
		return SUBTRACTION;
}


void PedestalSubtractorAlgorithm::skippEvent()
{
	m_event++;
}

void PedestalSubtractorAlgorithm::processTreaning()
{
	m_pedestalFollowingPtr->processEvent(m_data);
	m_event++;
	//if( m_followingOption != TbUT::FollowingOptions::Calculator) subtractPedestals();
}

void PedestalSubtractorAlgorithm::subtractPedestals()
{
	processAndSaveDataToTES();
	m_event++;
}

void PedestalSubtractorAlgorithm::processAndSaveDataToTES()
{
	RawData<> *afterPedestal=new RawData<>();
	m_pedestalSubtractor.processEvent(m_data,&afterPedestal);
	m_outputDataContainer->addData(*afterPedestal);
}


StatusCode PedestalSubtractorAlgorithm::savePedestalsToFile()
try{
	info()<<"Save Pedestal to File"<<endmsg;
	m_pedestalFollowingPtr->savePedestalToFile(m_pedestalOutputLocation);
	return  StatusCode::SUCCESS;
}
catch(IPedestalFollowing::PedestalCalculatorError &er)
{
	error()<<er.what() <<endmsg;
	return  StatusCode::FAILURE;
}
