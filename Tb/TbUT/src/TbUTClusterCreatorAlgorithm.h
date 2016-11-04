/*
 * TbUTClusterCreatorAlgorithm.h
 *
 *  Created on: Jan 6, 2015
 *      Author: ADendek
 */

#pragma once

#include "GaudiAlg/GaudiAlgorithm.h"
#include "TbUTRawData.h"
#include "TbUTClusterCreatorFactory.h"
#include "TbUTTresholdProvider.h"
#include "TbUTCluster.h"


namespace TbUT
{

class ClusterCreatorAlgorithm :  public GaudiAlgorithm
{
public:
	ClusterCreatorAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

	virtual StatusCode initialize();
	virtual StatusCode execute();
	virtual StatusCode finalize();

private:

	StatusCode initializeBase();
	StatusCode buildClusterCreator();
	StatusCode retreiveNoise();

	StatusCode getData();
	void processAndSaveDataToTES();

	RawDataContainer<double>* m_dataContainer;
	RawData<double>* m_data;

	std::string m_inputDataLocation;
	std::string m_outputDataLocation;
	std::string m_clusterCreatorOption;
	std::string m_noiseFile;
	std::string m_sensorType;
	int m_event;
	int m_skipEvent;
	double m_lowThreshold;
	double m_highThreshold;

	TresholdProvider m_zsThresholdProvider;
	ClusterCreatorFactory m_clusterCreatorFactory;
	ClusterCreatorFactory::ClusterCreatorPtr m_clusterCreator;
};

} /* namespace TbUT */

