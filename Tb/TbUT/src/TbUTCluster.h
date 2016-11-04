/*
 * TbUTCluster.h
 *
 *  Created on: Jan 4, 2015
 *      Author: ADendek
 */

#pragma once
#include "GaudiKernel/DataObject.h"
#include "TbUTRawData.h"
#include <vector>

namespace TbUT
{

struct Cluster
{
	Cluster();
	double m_position;
	int m_seedPosition;
	int m_size;
	double m_charge;
	double m_chargeSeed;
	double m_charge2StripLeft;
	double m_charge1StripLeft;
	double m_charge2StripRight;
	double m_charge1StripRight;
};

class ClusterContainer : public DataObject
{
public:
	typedef std::vector<Cluster> ClusterVector;
	ClusterContainer();
	void addCluster(Cluster& p_cluster);
	void addClusters(ClusterVector& p_cluster);

	void setClusters(ClusterVector& p_clusterVec);
	int size(){return m_clusterVector.size(); }
	ClusterVector getClusters(){return m_clusterVector;};
	bool isEmpty(){return m_isEmpty;}
	unsigned int getTDC()const {return m_tdc;}
	void setTDC(unsigned int p_tdc){m_tdc=p_tdc;}

	unsigned long long getTimestamp() const {return m_timestamp;}
	void setTiemestamp(unsigned long long p_timestamp){m_timestamp =p_timestamp;}

private:
	bool m_isEmpty;
	ClusterVector m_clusterVector;
	unsigned long long m_timestamp;
	unsigned int m_tdc;
};

} /* namespace TbUT */

