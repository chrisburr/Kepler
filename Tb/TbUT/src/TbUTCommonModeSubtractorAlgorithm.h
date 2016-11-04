/*
 * TbUTCommonModeSubtractorAlgorithm.h
 *
 *  Created on: Nov 24, 2014
 *      Author: ADendek
 */

#pragma once

#include "GaudiAlg/GaudiAlgorithm.h"

#include "TbUTRawData.h"
#include "TbUTCommonModeSubtractorFactory.h"
#include "TbUTChannelMaskProvider.h"
#include "TbUTChannelMaskFileValidator.h"
#include "TbUTNoiseCalculatorfactory.h"
#include "TbUTNoise.h"
#include <boost/shared_ptr.hpp>

namespace TbUT
{

class CommonModeSubtractorAlgorithm : public GaudiAlgorithm
{
	typedef boost::shared_ptr<ICommonModeSubtractor> CMSubtractorPtr;
public:
	CommonModeSubtractorAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

	virtual StatusCode initialize();
	virtual StatusCode execute();
	virtual StatusCode finalize();
private:

	StatusCode initializeBase();
	StatusCode buildSubtractorEngine();
	StatusCode buildNoiseCalculator();
	StatusCode retriveMasksFromFile();

	StatusCode getData();
	void processEvent();
	StatusCode skippTreaningEvent();

	StatusCode saveNoiseToFile();

	RawDataContainer<>* m_dataContainer;
	RawData<>* m_data;
	RawDataContainer<double>* m_outputContainer;


	std::string m_inputDataLocation;
	std::string m_outputDataLocation;
	std::string m_channelMaskInputLocation;
	std::string m_CMSOption;
	std::string m_noiseCalculatorType;
	std::string m_noiseOutputLocation;

	int m_event;
	int m_skipEvent;

	ChannelMaskFileValidator m_channelMaskFileValidator;
	ChannelMaskProvider m_channelMaskProvider;
	CommonModeSubtractorFactory m_SubtractorFactory;
	CMSubtractorPtr m_CMSEnginePtr;
	NoiseCalculatorFactory m_noiseCalculatorFactory;
	NoiseCalculatorFactory::NoiseCalcualtorPtr m_noiseCalculatorPtr;

};

}




