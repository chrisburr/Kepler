/*
 * TbUTClusterCreatorDataMonitorAlgorithm.h
 *
 *  Created on: Jan 6, 2015
 *      Author: ADendek
 */

#pragma once

#include "GaudiAlg/GaudiHistoAlg.h"
#include "TbUTCluster.h"
#include <TH1D.h>

namespace TbUT
{

class ClusterCreatorDataMonitorAlgorithm: public GaudiHistoAlg
{
public:
	ClusterCreatorDataMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

	virtual StatusCode initialize();
	virtual StatusCode execute   ();
	virtual StatusCode finalize  ();
private:

	virtual StatusCode getData();
	virtual StatusCode initializeBase();
	virtual void createHistograms();

	virtual StatusCode skippEvent();
	virtual StatusCode fillHistograms();
	virtual void createClusterNumberPerEventHistogram();
	virtual void createClusterNumberHistogram();
	virtual void createClusterSizeHistogram();
	virtual void createClusterChargeHistograms();

	virtual void fillClusterNumberHistogram(ClusterContainer::ClusterVector& p_clusters);
	virtual void fillClusterSizeHistogram(ClusterContainer::ClusterVector& p_clusters);
	virtual void fillClusterChargeHistograms(ClusterContainer::ClusterVector& p_clusters);

	virtual void  fillEventsHistograms(ClusterContainer::ClusterVector& p_clusters);
	virtual TH1D* bookHistogram(const std::string& p_histogramName);
	virtual void fillSingleEventHistogram(TH1D* p_histogram, int p_channel, double p_value);
	virtual std::string createSingleEventHistogramName(std::string p_HistogramType);

	int m_evtNumber;
    int m_skipEvent;
    int m_displayEventNumber;
	std::string m_inputDataLoc;
	std::string m_sensorType;

	ClusterContainer* m_clusters;
	TH1D* m_histogramClusterNumber;
	TH1D* m_histogramClusterSeedPosition;
	TH1D* m_histogramClusterNumberPerEvent;
	TH1D* m_histogramClusterSize;
	TH1D* m_histogramClusterCharge;
	TH1D* m_histogramClusterChargeSingleStrip;
	TH1D* m_histogramClusterChargeDoubleStrip;
	TH1D* m_histogramClusterChargeTripleStrip;


};

} /* namespace TbUT */

