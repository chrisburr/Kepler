/*
 * TbUTClusterCreator.h
 *
 *  Created on: Jan 5, 2015
 *      Author: ADendek
 */

#pragma once

#include "TbUTIClusterCreator.h"
#include "TbUTITresholdProvider.h"

namespace TbUT
{

class ClusterCreator: public IClusterCreator
{
public:
	ClusterCreator(const std::string&  p_sensorType, ITresholdProvider& p_thresholds);
	ClusterContainer::ClusterVector createClusters(RawData<double> *p_inputData);

private:
	 enum SensorType
	   {
	      P_TYPE,
	      N_TYPE,
	      CUSTOM
	   };

	typedef ClusterContainer::ClusterVector::iterator ClusterIterator;

	void convertStringToSensorType(const std::string& p_sensorType);

	void findCulsterSeeds(RawData<double> *p_inputData);
	void removeDuplicatedSeeds(RawData<double> *p_inputData);
	void extendClusterSeeds(RawData<double> *p_inputData);

	bool isBiggerThanSeedThreshold(RawData<double>::DataType p_channelSignal, int p_channel) const;
	Cluster createCluster(RawData<double> *p_inputData, int l_channelNumber) const;

	bool arePartOfTheSameCluster(RawData<double>* p_inputData, ClusterIterator& p_firstIt, ClusterIterator& p_secondIt) const;
	bool canBePartOfTheSameCluster(ClusterIterator& p_firstIt, ClusterIterator& p_secondIt) const;

	void extendCluster(ClusterIterator& p_clusterIt, RawData<double>* p_inputData);
	void removeClusterSeedWithSmallerCharge(ClusterIterator& p_firstIt, ClusterIterator& p_secondIt);
	bool hasNotMaximumSize(ClusterIterator& p_clusterIt) const;
	bool isStripNeedToBeAddedToCluster(ClusterIterator& p_clusterIt, RawData<double> *p_inputData,int p_stripShift, bool p_isCheckingLeft  ) const;
	void updateCluster(ClusterIterator& p_clusterIt,RawData<double> *p_inputData ,int p_stripShift, bool p_isCheckingLeft);
	bool isInvalidChannelNumber(int p_stripShift) const;
	bool isBiggerThanLowThreshold(RawData<double>::DataType p_channelSignal, int p_chnnel) const;
	void normalizeClusterPosition(ClusterIterator& p_clusterIt);
	void fillOuterStrips(RawData<double> *p_inputData);



	const int m_clusterMaxSize;
	ITresholdProvider& m_thresholds;
	ClusterContainer::ClusterVector m_culsterVector;
	SensorType m_sensorType;

};


} /* namespace TbUT */

