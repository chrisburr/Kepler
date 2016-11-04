/*
 * TbUTPedestalDataMonitorAlgorithm.cpp
 *
 *  Created on: Oct 18, 2014
 *      Author: ADendek
 */

#include "TbUTPedestalSubtractorDataMonitorAlgorithm.h"

#include "TbUTDataLocations.h"
#include "TbUTRawData.h"

#include "GaudiUtils/Aida2ROOT.h"
#include <boost/format.hpp>

DECLARE_NAMESPACE_ALGORITHM_FACTORY(TbUT,PedestalSubtractorDataMonitorAlgorithm)

using namespace TbUT;

PedestalSubtractorDataMonitorAlgorithm::PedestalSubtractorDataMonitorAlgorithm( const std::string& name,ISvcLocator* pSvcLocator)
  : DataMonitorAlgorithm ( name , pSvcLocator )
{
	DataMonitorAlgorithm::m_inputDataLoc=TbUT::DataLocations::PedestalTES;
}

std::string PedestalSubtractorDataMonitorAlgorithm::createHistogramName()
{
	boost::format l_histogramName("Data_after_pedestal_event_%d");
	l_histogramName % m_evtNumber;
	return str(l_histogramName);
}

TH2D * PedestalSubtractorDataMonitorAlgorithm::bookHistogram2D(const std::string & p_histogramName, const std::string & p_histogramTitle, int p_sensorNumber)
{
	int l_ylow=-800;
	int l_yhigh=800;
	int l_ybin=1600;
	return Gaudi::Utils::Aida2ROOT::aida2root(book2D( p_histogramName,	p_histogramTitle,
			-0.5+RawData<>::getMinChannel(),RawData<>::getMaxChannel()+0.5,p_sensorNumber,
			l_ylow,l_yhigh,l_ybin ));
}

std::string PedestalSubtractorDataMonitorAlgorithm::createHistogramTitle()
{
	boost::format l_histogramTitle("Data after Pedestal Subtraction - event%1%");
	l_histogramTitle% m_evtNumber;
	return str(l_histogramTitle);
}

void PedestalSubtractorDataMonitorAlgorithm::createHistogram2D()
{
	std::string l_histogramName="PedestalData_vs_channel";
	std::string l_histogramTtttle="Data after Pedestal vs channel";
	int l_sensorNum=RawData<>::getnChannelNumber();
	m_histogram2D=bookHistogram2D(l_histogramName,l_histogramTtttle,l_sensorNum );
}

StatusCode PedestalSubtractorDataMonitorAlgorithm::finalize()
{
	DataMonitorAlgorithm::m_outpuProjectionHistogramName="ProjectionPedestal";
	return DataMonitorAlgorithm::finalize();
}
