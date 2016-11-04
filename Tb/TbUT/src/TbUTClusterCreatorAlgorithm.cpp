/*
 * TbUTClusterCreatorAlgorithm.cpp
 *
 *  Created on: Jan 6, 2015
 *      Author: ADendek
 */

#include "TbUTClusterCreatorAlgorithm.h"

using namespace TbUT;

DECLARE_NAMESPACE_ALGORITHM_FACTORY(TbUT,ClusterCreatorAlgorithm)

ClusterCreatorAlgorithm::ClusterCreatorAlgorithm(const std::string& name, ISvcLocator* pSvcLocator):
GaudiAlgorithm(name, pSvcLocator),
m_dataContainer(),
m_data(),
m_inputDataLocation(),
m_outputDataLocation(),
m_clusterCreatorOption(),
m_noiseFile(),
m_sensorType(),
m_event(),
m_skipEvent(),
m_lowThreshold(),
m_highThreshold(),
m_zsThresholdProvider(m_noiseFile,m_lowThreshold,m_highThreshold),
m_clusterCreatorFactory(m_sensorType,m_zsThresholdProvider),
m_clusterCreator()
{
	declareProperty("InputDataLocation", m_inputDataLocation=TbUT::DataLocations::CMSTES);
	declareProperty("OutputDataLocation", m_outputDataLocation=TbUT::DataLocations::Clusters_TES);
	declareProperty("ClusterCreatorOption",m_clusterCreatorOption=TbUT::ClusterCreatorType::defaultCreator);
	declareProperty("NoiseInputFile",m_noiseFile=TbUT::DataLocations::NoiseTreshold);
	declareProperty("skippEventNumber",m_skipEvent=0);
	declareProperty("sensorType",m_sensorType="PType");
	declareProperty("LowThreshold",m_lowThreshold=3);
	declareProperty("HighThreshold",m_highThreshold=4);
}


StatusCode ClusterCreatorAlgorithm::initialize()
{
	if(StatusCode::SUCCESS !=initializeBase()) return StatusCode::FAILURE;
	if(StatusCode::SUCCESS !=buildClusterCreator()) return StatusCode::FAILURE;
	if(StatusCode::SUCCESS !=retreiveNoise()) return StatusCode::FAILURE;

	info()<<"Cluster thresholds (unit of noise multiplicity):"<<endmsg;
	info()<<"Low:" <<m_lowThreshold <<endmsg;
	info()<<"high:" <<m_highThreshold <<endmsg;

	info()<<"Initialized successfully!"<<endmsg;
	return StatusCode::SUCCESS;
}

StatusCode ClusterCreatorAlgorithm::execute()
{
	if(m_event<m_skipEvent)
	{
		m_event++;
		return StatusCode::SUCCESS;
	}
	if(StatusCode::SUCCESS != getData()) return StatusCode::FAILURE;
	if(m_dataContainer->isEmpty()){
		ClusterContainer *l_clontainer=new ClusterContainer();
		put(l_clontainer,m_outputDataLocation);
		return StatusCode::SUCCESS;
	}
	processAndSaveDataToTES();
	m_event++;
	return StatusCode::SUCCESS;
}

StatusCode ClusterCreatorAlgorithm::finalize()
{
	return GaudiAlgorithm::finalize();
}

StatusCode ClusterCreatorAlgorithm::initializeBase()
{
	return GaudiAlgorithm::initialize();
}

StatusCode ClusterCreatorAlgorithm::buildClusterCreator()
try{
	m_clusterCreator=m_clusterCreatorFactory.createClusterCreator(m_clusterCreatorOption);
	return StatusCode::SUCCESS;
}catch(ClusterCreatorFactory::NoSuchState &exception)
{
	error()<<"Invalid ZS Engine Type  : "<<exception.what()<<endmsg;
	return StatusCode::FAILURE;
}

StatusCode ClusterCreatorAlgorithm::retreiveNoise()
try{
	m_zsThresholdProvider.retreiveTresholds();
	return StatusCode::SUCCESS;
}catch(ITresholdProvider::ThresholdProviderError &err){
	error()<<"Cannot open Noise file: "<<err.what()<<endmsg;
	return StatusCode::FAILURE;
}


StatusCode ClusterCreatorAlgorithm::getData()
{
	m_dataContainer=getIfExists<RawDataContainer <double> >(m_inputDataLocation);
	if(!m_dataContainer){
		error()<< " ==> There is no input data: "<< m_inputDataLocation <<endmsg;
		return  StatusCode::FAILURE;
	}
	return  StatusCode::SUCCESS;
}

void ClusterCreatorAlgorithm::processAndSaveDataToTES()
{
	ClusterContainer *l_container=new ClusterContainer();
	for(const auto& rawDataIt: m_dataContainer->getData()){
		m_data=new RawData<double>(rawDataIt);
		ClusterContainer::ClusterVector clusterVector=m_clusterCreator->createClusters(m_data);
		l_container->addClusters(clusterVector);
		l_container->setTDC(m_data->getTDC());
		l_container->setTiemestamp(m_data->getTime());
		delete m_data;
	}
	put(l_container,m_outputDataLocation);
}
