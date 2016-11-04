/*
 * TbUTClusterCreator.cpp
 *
 *  Created on: Jan 5, 2015
 *      Author: ADendek
 */

#include "TbUTClusterCreator.h"
#include <iostream>
#include <cmath>

using namespace TbUT;
using namespace std;

ClusterCreator::ClusterCreator(const std::string&  p_sensorType,ITresholdProvider& p_thresholds):
		m_clusterMaxSize(4),
		m_thresholds(p_thresholds),
		m_culsterVector()
{
	convertStringToSensorType(p_sensorType);
}

ClusterContainer::ClusterVector ClusterCreator::createClusters(RawData<double> *p_inputData)
{
	m_culsterVector.clear();
	findCulsterSeeds(p_inputData);
	removeDuplicatedSeeds(p_inputData);
	extendClusterSeeds(p_inputData);
	fillOuterStrips(p_inputData);
	return m_culsterVector;
}

void ClusterCreator::convertStringToSensorType(const std::string& p_sensorType)
{
	if(p_sensorType == "PType")
		m_sensorType= ClusterCreator::SensorType::P_TYPE;
	else if (p_sensorType == "NType")
		m_sensorType= ClusterCreator::SensorType::N_TYPE;
	else
		m_sensorType= ClusterCreator::SensorType::CUSTOM;
}

void ClusterCreator::findCulsterSeeds(RawData<double> *p_inputData)
{
	int l_channelNumber=RawData<>::getnChannelNumber();
	for(auto channel=0;channel<l_channelNumber;channel++){
		auto l_channelSignal=p_inputData->getSignal(channel);
		if(isBiggerThanSeedThreshold(l_channelSignal,channel ))
			m_culsterVector.push_back(createCluster(p_inputData,channel));
	}

}

bool ClusterCreator::isBiggerThanSeedThreshold(RawData<double>::DataType p_channelSignal, int p_channel) const
{
	double l_channelThreshold=m_thresholds.getHighClusterThreshold(p_channel);

	switch(m_sensorType)
	{
	case ClusterCreator::SensorType::P_TYPE:
		return p_channelSignal<(-1)*l_channelThreshold;
	case ClusterCreator::SensorType::N_TYPE:
		return p_channelSignal>l_channelThreshold;
	default:
		return abs(p_channelSignal) >l_channelThreshold ;
	}
}

Cluster ClusterCreator::createCluster(RawData<double> *p_inputData, int l_channel) const
{
	int l_seedSize=1;
	Cluster l_cluster;
	l_cluster.m_charge=p_inputData->getSignal(l_channel);
	l_cluster.m_chargeSeed=p_inputData->getSignal(l_channel);
	l_cluster.m_seedPosition=l_channel;
	l_cluster.m_size=l_seedSize;
	l_cluster.m_position=l_channel*abs(l_cluster.m_charge); //position is weighted average, weight charge

	return l_cluster;
}

void ClusterCreator::removeDuplicatedSeeds(RawData<double> *p_inputData)
{
	if(0==m_culsterVector.size()) return;
	for(auto firstClusterIt = m_culsterVector.begin();firstClusterIt!=m_culsterVector.end();firstClusterIt++)
	{
		for(auto secondClusterIt=firstClusterIt+1;secondClusterIt!=m_culsterVector.end();)
		{
			if(arePartOfTheSameCluster(p_inputData, firstClusterIt,secondClusterIt))
				removeClusterSeedWithSmallerCharge(firstClusterIt,secondClusterIt);
			else
				secondClusterIt++;
			if(secondClusterIt==m_culsterVector.end()) break;
		}
		if(firstClusterIt==m_culsterVector.end()) break;
	}
}

bool ClusterCreator::arePartOfTheSameCluster(RawData<double>* p_inputData, ClusterIterator& p_firstIt, ClusterIterator& p_secondIt) const
{
	if(!canBePartOfTheSameCluster(p_firstIt, p_secondIt)) return false;

    auto channelRIterator=p_secondIt->m_seedPosition-1; // take previous strip position than pointed by secondIt
	auto firstSeedPosition=p_firstIt->m_seedPosition;
	for(;channelRIterator>firstSeedPosition;channelRIterator--){
		auto channelCharge=p_inputData->getSignal(channelRIterator);
		if(! isBiggerThanLowThreshold(channelCharge,channelRIterator) ) return false;
	}
	return true;
}


bool ClusterCreator::canBePartOfTheSameCluster(ClusterIterator& p_firstIt, ClusterIterator& p_secondIt) const
{
	int seedDistance=abs(p_firstIt->m_seedPosition-p_secondIt->m_seedPosition);
	return seedDistance<m_clusterMaxSize;
}

void ClusterCreator::removeClusterSeedWithSmallerCharge(ClusterIterator& p_firstIt, ClusterIterator& p_secondIt)
{
	if(abs(p_firstIt->m_charge)>abs(p_secondIt->m_charge))
		p_secondIt=m_culsterVector.erase(p_secondIt);
	else{
		p_firstIt =m_culsterVector.erase(p_firstIt);
		p_secondIt=p_firstIt+1;
	}
}

void ClusterCreator::extendClusterSeeds(RawData<double> *p_inputData)
{
	for(auto clusterIt = m_culsterVector.begin();clusterIt!=m_culsterVector.end();clusterIt++)
	{
		extendCluster(clusterIt, p_inputData);
	}
}

void ClusterCreator::extendCluster(ClusterIterator& p_clusterIt,RawData<double>* p_inputData)
{
	int l_actualShift=1;
	bool l_checkLeft=true;
	bool l_checkRight=true;
	bool l_isCheckingLeft=true;


	while(hasNotMaximumSize(p_clusterIt))
	{
		l_isCheckingLeft=true;
		if(l_checkLeft && (l_checkLeft=isStripNeedToBeAddedToCluster(p_clusterIt,p_inputData, l_actualShift, l_isCheckingLeft ) ) ){
			updateCluster(p_clusterIt,p_inputData, l_actualShift, l_isCheckingLeft);
		}
		if(! hasNotMaximumSize(p_clusterIt) ) break;

		l_isCheckingLeft=false;
		if(l_checkRight && (l_checkRight=isStripNeedToBeAddedToCluster(p_clusterIt,p_inputData, l_actualShift, l_isCheckingLeft ) ) ) updateCluster(p_clusterIt,p_inputData, l_actualShift, l_isCheckingLeft);
		l_actualShift++;
		if(!l_checkLeft && !l_checkRight ) break;
	}
}

bool ClusterCreator::hasNotMaximumSize(ClusterIterator& p_firstIt) const
{
	return p_firstIt->m_size<m_clusterMaxSize;
}

bool ClusterCreator::isStripNeedToBeAddedToCluster(ClusterIterator& p_clusterIt, RawData<double> *p_inputData, int p_stripShift,bool p_isCheckingLeft ) const
{
	int l_sign =(p_isCheckingLeft)? -1:1;
	int l_channelNumber=p_clusterIt->m_seedPosition+l_sign*p_stripShift;

	if(isInvalidChannelNumber(l_channelNumber ) ) return false;

	double l_channelSignal=p_inputData->getSignal(l_channelNumber);
	return isBiggerThanLowThreshold(l_channelSignal, l_channelNumber);
}

void ClusterCreator::updateCluster(ClusterIterator& p_clusterIt,RawData<double> *p_inputData ,int p_stripShift, bool p_isCheckingLeft)
{
	int l_sign =(p_isCheckingLeft)? -1:1;
	int l_channelNumber=p_clusterIt->m_seedPosition+l_sign*p_stripShift;
	auto l_channelSignal=p_inputData->getSignal(l_channelNumber);
	p_clusterIt->m_charge+=l_channelSignal;
	p_clusterIt->m_size+=1;
	p_clusterIt->m_position+=l_channelNumber*abs(l_channelSignal);
}

bool ClusterCreator::isInvalidChannelNumber(int p_stripNumber) const
{
	if(p_stripNumber<RawData<>::getMinChannel()) return true;
	if(p_stripNumber>RawData<>::getMaxChannel()) return true;
	return false;
}

bool ClusterCreator::isBiggerThanLowThreshold(RawData<double>::DataType p_channelSignal, int p_chnnel) const
{
	double l_threshold=m_thresholds.getLowClusterThreshold(p_chnnel);
	switch(m_sensorType)
	{
	case SensorType::P_TYPE:
		return p_channelSignal<(-1)*l_threshold;
	case SensorType::N_TYPE:
		return p_channelSignal>l_threshold;
	default:
		return abs(p_channelSignal) >l_threshold ;
	}
}


void ClusterCreator::fillOuterStrips(RawData<double> *p_inputData)
{
	for(auto& cluster : m_culsterVector){
		int position=cluster.m_seedPosition;
		int oneLeft=position-1;
		if(isInvalidChannelNumber(oneLeft)) cluster.m_charge1StripLeft = 0 ;
		else cluster.m_charge1StripLeft = p_inputData->getSignal(oneLeft) ;

		int twoLeft=oneLeft-1;
		if(isInvalidChannelNumber(twoLeft)) cluster.m_charge2StripLeft = 0;
		else cluster.m_charge2StripLeft = p_inputData->getSignal(twoLeft) ;

		int oneRight=position+1;
		if(isInvalidChannelNumber(oneRight)) cluster.m_charge1StripRight = 0 ;
		else cluster.m_charge1StripRight = p_inputData->getSignal(oneRight) ;

		int twoRight=oneRight+1;
		if(isInvalidChannelNumber(twoRight)) cluster.m_charge2StripRight = 0;
		else cluster.m_charge2StripRight = p_inputData->getSignal(twoRight) ;

		/// normalize position
		cluster.m_position/=abs(cluster.m_charge);
	}
}



