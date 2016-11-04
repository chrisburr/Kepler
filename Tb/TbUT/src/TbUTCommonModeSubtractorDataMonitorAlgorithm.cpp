/*
 * TbUTCommonModeSubtractorDataMonitorAlgorithm.cpp
 *
 *  Created on: Nov 26, 2014
 *      Author: ADendek
 */

#include "TbUTCommonModeSubtractorDataMonitorAlgorithm.h"
#include "TbUTDataLocations.h"
#include "TbUTRawData.h"
#include "GaudiUtils/Aida2ROOT.h"
#include <boost/format.hpp>

using namespace TbUT;
using namespace boost;

DECLARE_NAMESPACE_ALGORITHM_FACTORY(TbUT,CommonModeSubtractorDataMonitorAlgorithm)

using namespace TbUT;

CommonModeSubtractorDataMonitorAlgorithm::CommonModeSubtractorDataMonitorAlgorithm( const std::string& name,ISvcLocator* pSvcLocator)
  : DataMonitorAlgorithm ( name , pSvcLocator ),
	m_noise()
{
	DataMonitorAlgorithm::m_inputDataLoc=TbUT::DataLocations::CMSTES;
}

StatusCode CommonModeSubtractorDataMonitorAlgorithm::execute()
{
	DataMonitorAlgorithm::RunPhase l_runPhase=DataMonitorAlgorithm::getRunPhase();
	switch(l_runPhase)
	{
	case SKIP:
		return DataMonitorAlgorithm::skippEvent();
	case SAVE_SINGLE_EVENTS:
		return saveSimpleEvents();
	default:
	 	return fillOnly2DHistogram();
	}
}

StatusCode CommonModeSubtractorDataMonitorAlgorithm::getData()
{
	m_dataContainer=getIfExists<RawDataContainer<double> >(m_inputDataLoc);
	if(!m_dataContainer){
		error()<< "=> there is no input data in "<< m_inputDataLoc<<endmsg;
		return  StatusCode::FAILURE;
	}
	return  StatusCode::SUCCESS;
}

StatusCode CommonModeSubtractorDataMonitorAlgorithm::saveSimpleEvents()
{
	if(StatusCode::SUCCESS != getData()) return StatusCode::FAILURE;
	if(m_dataContainer->isEmpty()) return StatusCode::SUCCESS;
	for(const auto& rawDataIt : m_dataContainer->getData() )
	{
		m_data=RawData<double>(rawDataIt);
		storeEventIntoHistogram();
		fillHistogram2D();
		m_evtNumber++;
	}
	return StatusCode::SUCCESS;
}

TH2D * CommonModeSubtractorDataMonitorAlgorithm::bookHistogram2D(const std::string & p_histogramName, const std::string & p_histogramTitle, int p_sensorNumber)
{
	int l_ylow=-800;
	int l_yhigh=800;
	int l_ybin=1600;
	return Gaudi::Utils::Aida2ROOT::aida2root(book2D( p_histogramName,	p_histogramTitle,
			-0.5+RawData<>::getMinChannel(),RawData<>::getMaxChannel()-0.5,p_sensorNumber,
			l_ylow,l_yhigh,l_ybin ));
}

std::string CommonModeSubtractorDataMonitorAlgorithm::createHistogramTitle()
{
	boost::format l_histogramTitle("Data after CM Subtraction - event%1%");
	l_histogramTitle% m_evtNumber;
	return str(l_histogramTitle);
}

std::string CommonModeSubtractorDataMonitorAlgorithm::createHistogramName()
{
	boost::format l_histogramName("Data_after_CMS_event_%d");
	l_histogramName % m_evtNumber;
	return str(l_histogramName);
}

void CommonModeSubtractorDataMonitorAlgorithm::createHistogram2D()
{
	std::string l_histogramName="CMSData_vs_channel";
	std::string l_histogramTtttle="Data after CMS vs channel";
	int l_sensorNum=RawData<>::getnChannelNumber();
	m_histogram2D=bookHistogram2D(l_histogramName,l_histogramTtttle,l_sensorNum );
	createNoiseHistograms();
}

StatusCode CommonModeSubtractorDataMonitorAlgorithm::fillOnly2DHistogram()
{
	if(StatusCode::SUCCESS != getData()) return StatusCode::FAILURE;
	if(m_dataContainer->isEmpty()) return StatusCode::SUCCESS;
	const int storeNoiseFrequency=10000;
	for(const auto& rawDataIt :m_dataContainer->getData() )
	{
		m_data=RawData<double>(rawDataIt);
		m_noise.updateNoise(&(m_data));
		if( 0 ==(DataMonitorAlgorithm::m_evtNumber%storeNoiseFrequency) ) fillNoiseHistograms();
		fillHistogram2D();
		DataMonitorAlgorithm::m_evtNumber++;
	}
	return StatusCode::SUCCESS;
}

void CommonModeSubtractorDataMonitorAlgorithm::fillHistogram2D()
{
	int channelNumber=RawData<>::getnChannelNumber();
	for(int chan = 0; chan <channelNumber; chan++ ){
		auto channelSignal=m_data.getSignal(chan);
		if(0!=channelSignal) // no need to push masked values
			m_histogram2D->Fill(chan+RawData<>::getMinChannel(),channelSignal);
	  }
}

void CommonModeSubtractorDataMonitorAlgorithm::fillHistogram(TH1D * p_histogram)
{
	int channelNumber=RawData<>::getnChannelNumber();
	for(int chan =0 ; chan <channelNumber; chan++ ){
		p_histogram->SetBinContent(chan,m_data.getSignal(chan));
	  }
}

StatusCode CommonModeSubtractorDataMonitorAlgorithm::finalize()
{
	DataMonitorAlgorithm::m_outpuProjectionHistogramName="ProjectionCommonMode";
	return DataMonitorAlgorithm::finalize();
}


void CommonModeSubtractorDataMonitorAlgorithm::createNoiseHistograms()
{
	// This function is very ugly!
	// should be refactored!
 	const int l_channelNumber=RawData<>::getnChannelNumber();
	const int l_ylow=0;
	const int l_yhigh=100;
	const int l_ybin=10;
	const int channelPerBeetle=32;
	int sensorNumber=0;

	for(int channel=0;channel<l_channelNumber ; channel++){
		boost::format l_histogramName("Noise_channel_%d");
		l_histogramName % channel;
		m_noisePerChannelHistograms.insert(
				std::make_pair(
						channel,Gaudi::Utils::Aida2ROOT::aida2root(
									book1D( l_histogramName.str(),	l_histogramName.str(),l_ybin, l_ylow, l_yhigh)
								)
				)
		);
		if( 0 == (channel%channelPerBeetle )){
			l_histogramName= boost::format("Noise_beetle_%d");
			l_histogramName % sensorNumber;
			m_noisePerSensorHistograms.insert(
					std::make_pair(
							sensorNumber,Gaudi::Utils::Aida2ROOT::aida2root(
										book1D( l_histogramName.str(),	l_histogramName.str(),l_ybin, l_ylow, l_yhigh)
									)
					)
			);
			sensorNumber++;
		}

	}
}

void CommonModeSubtractorDataMonitorAlgorithm::fillNoiseHistograms()
{
	m_noise.NormalizeNoise();
	for(auto noiseHistogramIt:m_noisePerChannelHistograms){
		noiseHistogramIt.second->Fill(m_noise.getNoise(noiseHistogramIt.first));
	}

	const int l_channelNumber=RawData<>::getnChannelNumber();
	const int channelPerBeetle=32;
	double meanNoisePerBeetle=0;
	int sensorNumber=0;

	for(int channel=0;channel<l_channelNumber ; channel++){
		meanNoisePerBeetle+=m_noise.getNoise(channel);
		if( 0 == (channel%channelPerBeetle )){
			meanNoisePerBeetle/=static_cast<double>(channelPerBeetle);
			m_noisePerSensorHistograms[sensorNumber]->Fill(meanNoisePerBeetle);
			sensorNumber++;
			meanNoisePerBeetle=0;// reset mean value
		}
	}
	m_noise.Reset();
}


