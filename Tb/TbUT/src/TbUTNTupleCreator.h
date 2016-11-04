
#pragma once

#include "GaudiAlg/GaudiTupleAlg.h"
#include "TbUTRawData.h"
#include "TbUTCluster.h"

#include <string>
#include <stdexcept>


namespace TbUT
{
class NTupleCreator : public GaudiTupleAlg {

	class DataError: public std::runtime_error
	{
	public:
		DataError(const std::string& err) :
			std::runtime_error(err)
		{
		}
	};

 public:
	NTupleCreator(const std::string& name, ISvcLocator* pSvcLocator);

	virtual StatusCode initialize();
	virtual StatusCode execute();

 private:
	void bookRawData();
        void bookHeaderData();
	void bookPedestal();
	void bookCMS();
	void bookClusters();


	void fillRawData();
        void fillHeaderData();
	void fillPedestal();
	void fillCMS();
	void fillClusters();
	template <typename DATA_TYPE = int>
	RawDataContainer<DATA_TYPE>* getDataFromTES(const std::string& p_location);
	ClusterContainer* getClustersFromTES();

	int m_eventNumber;
	int m_storeEventNumber;
	bool m_isRawWritten;
        bool m_isHeaderWritten;
	bool m_isPedestalWritten;
	bool m_isCMSWritten;
	bool m_isClusterWritten;

	std::string m_rawLocation;
	std::string m_pedestalLocation;
	std::string m_cmsLocation;
	std::string m_clusterLocation;
	std::string m_headerLocation;
 
	NTuple::Array<double> m_rawSignal;
	NTuple::Array<double> m_header0Signal;
	NTuple::Array<double> m_header1Signal;
	NTuple::Array<double> m_header2Signal;
	NTuple::Array<double> m_header3Signal;
	NTuple::Array<double> m_header3P1Signal;
	NTuple::Array<double> m_header3P2Signal;
	NTuple::Array<double> m_pedestalSignal;
	NTuple::Array<double> m_cmsSignal;

	NTuple::Item<int> m_clusterNumberPerEvent;
	NTuple::Item<unsigned int> m_tdc;
	NTuple::Item<unsigned long long> m_timestamp;

	NTuple::Array<double> m_clusterPosition;
	NTuple::Array<int> m_clusterSize;
	NTuple::Array<int> m_clusterSeedPosition;
	NTuple::Array<double> m_clusterCharge;
	NTuple::Array<double> m_clusterSeedCharge;
	NTuple::Array<double> m_clusterCharge2StripLeft;
	NTuple::Array<double> m_clusterCharge1StripLeft;
	NTuple::Array<double> m_clusterCharge2StripRight;
	NTuple::Array<double> m_clusterCharge1StripRight;

};

}
