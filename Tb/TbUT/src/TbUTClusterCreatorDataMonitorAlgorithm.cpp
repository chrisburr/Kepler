/*
 * TbUTClusterCreatorDataMonitorAlgorithm.cpp
 *
 *  Created on: Jan 6, 2015
 *      Author: ADendek
 */

#include "TbUTClusterCreatorDataMonitorAlgorithm.h"
#include "TbUTRawData.h"
#include "GaudiUtils/Aida2ROOT.h"
#include <boost/format.hpp>

using namespace TbUT;

DECLARE_NAMESPACE_ALGORITHM_FACTORY(TbUT,ClusterCreatorDataMonitorAlgorithm)

ClusterCreatorDataMonitorAlgorithm::ClusterCreatorDataMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator ):
		GaudiHistoAlg ( name , pSvcLocator ),
		m_evtNumber(0),
		m_skipEvent(0),
		m_displayEventNumber(0),
		m_inputDataLoc(TbUT::DataLocations::Clusters_TES),
		m_sensorType()
{
    declareProperty("skippEventNumber", m_skipEvent=10);
    declareProperty("displayEventNumber", m_displayEventNumber=10);
    declareProperty("sensorType",m_sensorType="PType");

}

StatusCode ClusterCreatorDataMonitorAlgorithm::initialize() {
	if(StatusCode::SUCCESS !=initializeBase()) return StatusCode::FAILURE;
	setHistoTopDir( const_cast<char*>("TbUT/") );
	createHistograms();
  	return StatusCode::SUCCESS;
}


StatusCode ClusterCreatorDataMonitorAlgorithm::execute()
{
	if(m_evtNumber<m_skipEvent)
		return skippEvent();
	else
		return fillHistograms();
}

StatusCode ClusterCreatorDataMonitorAlgorithm::finalize()
{
	return GaudiHistoAlg::finalize();
}

StatusCode ClusterCreatorDataMonitorAlgorithm::initializeBase()
{
	  return GaudiHistoAlg::initialize();
}

StatusCode ClusterCreatorDataMonitorAlgorithm::getData()
{
	if(!exist<ClusterContainer>(m_inputDataLoc)){
		error()<< " ==> There is no input data: "<< m_inputDataLoc <<endmsg;
		return  StatusCode::FAILURE;
	}
	m_clusters=get<ClusterContainer>(m_inputDataLoc);
	if(!m_clusters){
		error()<< " ==> Input data is Null!"<<endmsg;
		return  StatusCode::FAILURE;
	}
	return  StatusCode::SUCCESS;
}

void ClusterCreatorDataMonitorAlgorithm::createHistograms()
{
	createClusterNumberPerEventHistogram();
	createClusterNumberHistogram();
	createClusterSizeHistogram();
	createClusterChargeHistograms();
}

void ClusterCreatorDataMonitorAlgorithm::createClusterNumberPerEventHistogram()
{
	double l_xmin=0;
	double l_xmax=6;
	int l_nbin=6;
	const std::string l_histogramName="ClusterNumberPerEvent";
	const std::string l_histogramTitle="Cluster Number Per Event";
	m_histogramClusterNumberPerEvent=Gaudi::Utils::Aida2ROOT::aida2root(book1D( l_histogramName, l_histogramTitle,l_xmin, l_xmax, l_nbin ));

}

void ClusterCreatorDataMonitorAlgorithm::createClusterNumberHistogram()
{
	double l_xmin=-0.5+RawData<>::getMinChannel();
	double l_xmax=RawData<>::getMaxChannel()-0.5;
	int l_nbin=RawData<>::getnChannelNumber();
	std::string l_histogramName="ClusterNumber";
	std::string l_histogramTitle="Cluster Number vs channel";
	m_histogramClusterNumber=Gaudi::Utils::Aida2ROOT::aida2root(book1D( l_histogramName, l_histogramTitle,l_xmin, l_xmax, l_nbin ));
	l_histogramName="ClusterSeedPosition";
	l_histogramTitle="Cluster Seed Position";
	m_histogramClusterSeedPosition=Gaudi::Utils::Aida2ROOT::aida2root(book1D( l_histogramName, l_histogramTitle,l_xmin, l_xmax, l_nbin ));

}

void ClusterCreatorDataMonitorAlgorithm::createClusterSizeHistogram()
{
	double l_xmin=-0.5;
	double l_xmax=5.5;
	int l_nbin=20;
	const std::string l_histogramName="ClusterSize";
	const std::string l_histogramTitle="Cluster Size";
	m_histogramClusterSize=Gaudi::Utils::Aida2ROOT::aida2root(book1D( l_histogramName, l_histogramTitle,l_xmin, l_xmax, l_nbin ));
}

void ClusterCreatorDataMonitorAlgorithm::createClusterChargeHistograms()
{
	double l_xmin;
	double l_xmax;

	if(m_sensorType=="PType"){
		l_xmin=-1500.;
		l_xmax=-1.;
	}else if(m_sensorType=="NType"){
		l_xmin=1.;
		l_xmax=1500.;
	}else{
		l_xmin=-1500.;
		l_xmax=1500.;
	}

	int l_nbin=100;
	std::string l_histogramName="ClusterCharge";
	std::string l_histogramTitle="Cluster Charge";
	m_histogramClusterCharge=Gaudi::Utils::Aida2ROOT::aida2root(book1D( l_histogramName, l_histogramTitle,l_xmin, l_xmax, l_nbin ));
	l_histogramName="ClusterChargeSingleStrip";
	l_histogramTitle="Cluster Charge Single Strip";
	m_histogramClusterChargeSingleStrip=Gaudi::Utils::Aida2ROOT::aida2root(book1D( l_histogramName, l_histogramTitle,l_xmin, l_xmax, l_nbin ));
	l_histogramName="ClusterChargeDoubleStrip";
	l_histogramTitle="Cluster Charge Double Strip";
	m_histogramClusterChargeDoubleStrip=Gaudi::Utils::Aida2ROOT::aida2root(book1D( l_histogramName, l_histogramTitle,l_xmin, l_xmax, l_nbin ));
	l_histogramName="ClusterChargeTripleStrip";
	l_histogramTitle="Cluster Charge Triple Strip";
	m_histogramClusterChargeTripleStrip=Gaudi::Utils::Aida2ROOT::aida2root(book1D( l_histogramName, l_histogramTitle,l_xmin, l_xmax, l_nbin ));
}

StatusCode ClusterCreatorDataMonitorAlgorithm::skippEvent()
{
	m_evtNumber++;
	return StatusCode::SUCCESS;
}

StatusCode ClusterCreatorDataMonitorAlgorithm::fillHistograms()
{
	if(StatusCode::SUCCESS != getData()) return StatusCode::FAILURE;
	if(m_clusters->isEmpty())
	{
		m_histogramClusterNumberPerEvent->Fill(0.);
		m_evtNumber++;
		return StatusCode::SUCCESS;
	}
	auto l_clusters=m_clusters->getClusters();
	fillClusterNumberHistogram(l_clusters);
	fillClusterSizeHistogram(l_clusters);
	fillClusterChargeHistograms(l_clusters);
	if(m_evtNumber<m_displayEventNumber+m_skipEvent)fillEventsHistograms(l_clusters);
	m_evtNumber++;
	return StatusCode::SUCCESS;
}

void ClusterCreatorDataMonitorAlgorithm::fillClusterNumberHistogram(ClusterContainer::ClusterVector& p_clusters)
{
	m_histogramClusterNumberPerEvent->Fill(p_clusters.size());
	for(const auto& l_cluster: p_clusters )
	{
		auto l_position=l_cluster.m_position;
		l_position+=RawData<>::getMinChannel();
		m_histogramClusterNumber->Fill(l_position);
		auto l_seedPosition=l_cluster.m_seedPosition;
		m_histogramClusterSeedPosition->Fill(l_seedPosition);
	}
}

void ClusterCreatorDataMonitorAlgorithm::fillClusterSizeHistogram(ClusterContainer::ClusterVector& p_clusters)
{
	for(const auto& l_cluster: p_clusters ){
		auto l_size=l_cluster.m_size;
		m_histogramClusterSize->Fill(l_size);
	}
}

void ClusterCreatorDataMonitorAlgorithm::fillClusterChargeHistograms(ClusterContainer::ClusterVector& p_clusters)
{
	using namespace std;
	for(const auto& l_cluster: p_clusters )
	{
		auto l_charge=l_cluster.m_charge;
		m_histogramClusterCharge->Fill(l_charge);
		switch(l_cluster.m_size)
		{
		case 1:
			m_histogramClusterChargeSingleStrip->Fill(l_charge);
			break;
		case 2:
			m_histogramClusterChargeDoubleStrip->Fill(l_charge);
			break;
		case 3:
			m_histogramClusterChargeTripleStrip->Fill(l_charge);
			break;
		}
	}
}


void ClusterCreatorDataMonitorAlgorithm::fillEventsHistograms(ClusterContainer::ClusterVector& p_clusters)
{
	for(const auto& l_cluster: p_clusters )
	{
			auto l_position=l_cluster.m_position;
			auto l_charge=l_cluster.m_charge;
			auto l_size=l_cluster.m_size;

			auto l_name=createSingleEventHistogramName("charge");
			auto l_histogram=bookHistogram(l_name);
			fillSingleEventHistogram(l_histogram,l_position,l_charge);

			l_name=createSingleEventHistogramName("size");
			l_histogram=bookHistogram(l_name);
			fillSingleEventHistogram(l_histogram,l_position,l_size);
		}
}

TH1D* ClusterCreatorDataMonitorAlgorithm::bookHistogram(const std::string& p_histogramName)
{
	return Gaudi::Utils::Aida2ROOT::aida2root(book1D( p_histogramName,	p_histogramName,
			-0.5+RawData<>::getMinChannel(),RawData<>::getMaxChannel()-0.5,RawData<>::getnChannelNumber() ));
}

void ClusterCreatorDataMonitorAlgorithm::fillSingleEventHistogram(TH1D* p_histogram, int p_channel, double p_value)
{
	p_histogram->SetBinContent(p_channel,p_value);
}

std::string ClusterCreatorDataMonitorAlgorithm::createSingleEventHistogramName(std::string p_HistogramType)
{
	using namespace boost;
	boost::format l_histogramName("cluster_%s_event_%d");
		l_histogramName % p_HistogramType % m_evtNumber;
		return str(l_histogramName);
}
