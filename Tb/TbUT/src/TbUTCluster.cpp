/*
 * TbUTCluster.cpp
 *
 *  Created on: Jan 4, 2015
 *      Author: ADendek
 */

#include "TbUTCluster.h"

using namespace TbUT;


Cluster::Cluster():
		m_position(0),
		m_size(0),
		m_charge(0),
		m_chargeSeed(0),
		m_charge2StripLeft(0),
		m_charge1StripLeft(0),
		m_charge2StripRight(0),
		m_charge1StripRight(0)
{
}

ClusterContainer::ClusterContainer():
		m_isEmpty(true),
		m_clusterVector(),
		m_timestamp(0),
		m_tdc(0)
{
}

void ClusterContainer::setClusters(ClusterVector& p_clusterVec)
{
	m_clusterVector=p_clusterVec;
	m_isEmpty=false;
}

void ClusterContainer::addClusters(ClusterVector& p_cluster)
{
	m_isEmpty=false;
	m_clusterVector.insert(m_clusterVector.begin(),p_cluster.begin(),p_cluster.end());
}

