/*
 * TbUTCommonModeSubtractorAlgorithm.cpp
 *
 *  Created on: Nov 24, 2014
 *      Author: ADendek
 */

#include "TbUTCommonModeSubtractorAlgorithm.h"

using namespace TbUT;

DECLARE_NAMESPACE_ALGORITHM_FACTORY(TbUT,CommonModeSubtractorAlgorithm)


CommonModeSubtractorAlgorithm::CommonModeSubtractorAlgorithm(const std::string& name, ISvcLocator* pSvcLocator):
		GaudiAlgorithm(name, pSvcLocator),
		m_dataContainer(),
		m_data(),
		m_outputContainer(),
		m_inputDataLocation(),
		m_outputDataLocation(),
		m_channelMaskInputLocation(),
		m_CMSOption(),
		m_noiseCalculatorType(),
		m_noiseOutputLocation(),
		m_event(0),
		m_skipEvent(0),
		m_channelMaskFileValidator(m_channelMaskInputLocation),
		m_channelMaskProvider(m_channelMaskFileValidator),
		m_SubtractorFactory(m_channelMaskProvider),
		m_noiseCalculatorFactory(),
		m_noiseCalculatorPtr()
{
	declareProperty("InputDataLocation", m_inputDataLocation=TbUT::DataLocations::PedestalTES);
	declareProperty("OutputDataLocation", m_outputDataLocation=TbUT::DataLocations::CMSTES);
	declareProperty("CMSType",m_CMSOption=TbUT::CMSType::Iteratively);
	declareProperty("skippEventNumber",m_skipEvent=0);
	declareProperty("ChannelMaskInputLocation", m_channelMaskInputLocation=TbUT::DataLocations::MaskLocation);
	declareProperty("NoiseCalculatorType", m_noiseCalculatorType=TbUT::NoiseCalculatorType::calculator);
	declareProperty("NoiseOutputFile", m_noiseOutputLocation=TbUT::DataLocations::NoiseTreshold);
}


StatusCode CommonModeSubtractorAlgorithm::initialize()
{
	if(StatusCode::SUCCESS !=initializeBase()) return StatusCode::FAILURE;
	if(StatusCode::SUCCESS !=buildSubtractorEngine()) return StatusCode::FAILURE;
	if(StatusCode::SUCCESS !=buildNoiseCalculator()) return StatusCode::FAILURE;
	if(StatusCode::SUCCESS !=retriveMasksFromFile()) return StatusCode::FAILURE;

	info()<<"Initialized successfully my friend!"<<endmsg;
	return StatusCode::SUCCESS;
}

StatusCode CommonModeSubtractorAlgorithm::execute()
{
	if(m_event<m_skipEvent)
	{
		return skippTreaningEvent();
	}
	if(StatusCode::SUCCESS != getData()) return StatusCode::FAILURE;
	m_outputContainer =new RawDataContainer<double>();

	if(m_dataContainer->isEmpty()){
		put(m_outputContainer,m_outputDataLocation);
		return StatusCode::SUCCESS;
	}

	for(const auto& rawDataIt: m_dataContainer->getData()){
		m_data=new RawData<>(rawDataIt);
		processEvent();
		delete m_data;
	}
	put(m_outputContainer,m_outputDataLocation);

	return StatusCode::SUCCESS;
}

StatusCode CommonModeSubtractorAlgorithm::finalize()
{
	if(StatusCode::SUCCESS !=saveNoiseToFile()) return StatusCode::FAILURE;
	return GaudiAlgorithm::finalize();
}


StatusCode CommonModeSubtractorAlgorithm::initializeBase()
{
	  return GaudiAlgorithm::initialize();
}

StatusCode CommonModeSubtractorAlgorithm::buildSubtractorEngine()
try{
	m_CMSEnginePtr.reset(m_SubtractorFactory.createCMSubtractor(m_CMSOption));
	return StatusCode::SUCCESS;
}catch(CommonModeSubtractorFactory::NoSuchState &exception)
{
	error()<<"Invalid CommonMode Engine Type  : "<<exception.what()<<endmsg;
	return StatusCode::FAILURE;
}


StatusCode CommonModeSubtractorAlgorithm::buildNoiseCalculator()
try{
	m_noiseCalculatorPtr=m_noiseCalculatorFactory.createNoiseCalculator(m_noiseCalculatorType);
	return StatusCode::SUCCESS;
}catch(NoiseCalculatorFactory::NoSuchState &exception)
{
	error()<<"Invalid NoiseCalculator Type  : "<<exception.what()<<endmsg;
	return StatusCode::FAILURE;
}


StatusCode CommonModeSubtractorAlgorithm::retriveMasksFromFile()
try{
	m_channelMaskProvider.getMaskFromFile(m_channelMaskInputLocation);
	return StatusCode::SUCCESS;
}catch(ChannelMaskProvider::InputFileError  &exception)
{
	error()<<"Channel Mask File Input Error: "<<m_channelMaskInputLocation<<endmsg;
	return StatusCode::FAILURE;
}

StatusCode CommonModeSubtractorAlgorithm::getData()
{
	m_dataContainer=getIfExists<RawDataContainer<>>(m_inputDataLocation);
	if(!m_dataContainer){
		error()<< " ==> There is no input data: "<< m_inputDataLocation <<endmsg;
		return  StatusCode::FAILURE;
	}
	return  StatusCode::SUCCESS;
}

void CommonModeSubtractorAlgorithm::processEvent()
{
	RawData<double> *l_afterCMS= new RawData<double>();
	m_CMSEnginePtr->processEvent(m_data,&l_afterCMS);
	m_noiseCalculatorPtr->updateNoise(l_afterCMS);
	m_outputContainer->addData(*l_afterCMS);
	m_event++;
}

StatusCode CommonModeSubtractorAlgorithm::skippTreaningEvent()
{
	m_event++;
	return StatusCode::SUCCESS;
}

StatusCode CommonModeSubtractorAlgorithm::saveNoiseToFile()
try{
	m_noiseCalculatorPtr->saveNoiseToFile(m_noiseOutputLocation);
	return StatusCode::SUCCESS;
}catch (Noise::NoiseCalculatorError & ex)
{
	error()<< ex.what()<<endmsg;
	return StatusCode::FAILURE;
}



