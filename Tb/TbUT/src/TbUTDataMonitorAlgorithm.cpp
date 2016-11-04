#include "TbUTDataMonitorAlgorithm.h"
#include "GaudiUtils/Aida2ROOT.h"
#include "GaudiKernel/AlgFactory.h" 
#include <boost/format.hpp>

using namespace TbUT;
using namespace boost;
DECLARE_NAMESPACE_ALGORITHM_FACTORY(TbUT,DataMonitorAlgorithm)


DataMonitorAlgorithm::DataMonitorAlgorithm( const std::string& name,ISvcLocator* pSvcLocator)
  : GaudiHistoAlg ( name , pSvcLocator ),
	m_inputDataLoc(),
	m_outpuProjectionHistogramName(),
    m_displayEventNumber(0),
    m_evtNumber(0),
	m_skipEvent(0),
	m_histogram2D()
{
    declareProperty("displayEventNumber", m_displayEventNumber=10);
    declareProperty("skippEventNumber", m_skipEvent=0);

}

DataMonitorAlgorithm::~DataMonitorAlgorithm(){}

StatusCode DataMonitorAlgorithm::initialize() {
	if(StatusCode::SUCCESS !=initializeBase()) return StatusCode::FAILURE;
	setHistoTopDir( const_cast<char*>("TbUT/") );
	createHistogram2D();
  	return StatusCode::SUCCESS;
}

StatusCode DataMonitorAlgorithm::execute()
{
	RunPhase l_runPhase=getRunPhase();
	switch(l_runPhase)
	{
	case SKIP:
		return skippEvent();
	case SAVE_SINGLE_EVENTS:
		return saveSimpleEvents();
	default:
	 	return fillOnly2DHistogram();
	}
}

StatusCode DataMonitorAlgorithm::finalize()
{
	buildProjectionHistogram();
	return GaudiHistoAlg::finalize();
}

StatusCode DataMonitorAlgorithm::initializeBase()
{
	  return GaudiHistoAlg::initialize();
}


void DataMonitorAlgorithm::createHistogram2D()
{
	std::string l_histogramName="RawData_vs_channel";
	std::string l_histogramTtttle="RawData_vs_channel";
	int l_sensorNum=RawData<>::getnChannelNumber();
	m_histogram2D=bookHistogram2D(l_histogramName,l_histogramTtttle,l_sensorNum );
}

StatusCode DataMonitorAlgorithm::getData()
{
	m_dataContainer=getIfExists<RawDataContainer<> >(m_inputDataLoc);
	if(!m_dataContainer){
		error()<< " ==> there is no input data in "<< m_inputDataLoc<<endmsg;
		return  StatusCode::FAILURE;
	}
	return  StatusCode::SUCCESS;
}

StatusCode DataMonitorAlgorithm::skippEvent()
{
	m_evtNumber++;
	return StatusCode::SUCCESS;
}

StatusCode DataMonitorAlgorithm::saveSimpleEvents()
{
	if(StatusCode::SUCCESS != getData()) return StatusCode::FAILURE;
	if(m_dataContainer->isEmpty()) return StatusCode::SUCCESS;
	for(const auto& rawDataIt : m_dataContainer->getData() )
	{
		m_data=RawData<>(rawDataIt);
		storeEventIntoHistogram();
		fillHistogram2D();
		m_evtNumber++;
	}
	return StatusCode::SUCCESS;
}

StatusCode DataMonitorAlgorithm::fillOnly2DHistogram()
{
	if(StatusCode::SUCCESS != getData()) return StatusCode::FAILURE;
	if(m_dataContainer->isEmpty()) return StatusCode::SUCCESS;
	for(const auto& rawDataIt : m_dataContainer->getData() )
	{
		m_data=RawData<>(rawDataIt);
		fillHistogram2D();
		m_evtNumber++;
	}
	return StatusCode::SUCCESS;
}

void DataMonitorAlgorithm::storeEventIntoHistogram()
{
	std::string l_histName= createHistogramName();
	std::string l_histTitle= createHistogramTitle();
	int l_sensorNum=RawData<>::getnChannelNumber();
	TH1D* l_eventHist=bookHistogram(l_histName,l_histTitle, l_sensorNum );
	fillHistogram(l_eventHist);
}

std::string DataMonitorAlgorithm::createHistogramName()
{
	boost::format l_histogramName("event %d");
	l_histogramName % m_evtNumber;
	return str(l_histogramName);
}

std::string DataMonitorAlgorithm::createHistogramTitle()
{
	boost::format l_histogramTitle("Raw Data - event%1%");
	l_histogramTitle% m_evtNumber;
	return str(l_histogramTitle);
}

TH1D * DataMonitorAlgorithm::bookHistogram(const std::string & p_histogramName, const std::string & p_histogramTitle, int p_sensorNumber)
{
	return Gaudi::Utils::Aida2ROOT::aida2root(book1D( p_histogramName,	p_histogramTitle,
			-0.5+RawData<>::getMinChannel(),RawData<>::getMaxChannel()-0.5,p_sensorNumber ));
}

void DataMonitorAlgorithm::fillHistogram(TH1D * p_histogram)
{
	int channelNumber=RawData<>::getnChannelNumber();
	for(int chan =0 ; chan <channelNumber; chan++ ){
		p_histogram->SetBinContent(chan,m_data.getSignal(chan));
	  }
}

TH2D * DataMonitorAlgorithm::bookHistogram2D(const std::string & p_histogramName, const std::string & p_histogramTitle, int p_sensorNumber)
{
	int l_ylow=-800;
	int l_yhigh=800;
	int l_ybin=1600;
	return Gaudi::Utils::Aida2ROOT::aida2root(book2D( p_histogramName,	p_histogramTitle,
			-0.5+RawData<>::getMinChannel(),RawData<>::getMaxChannel()-0.5,p_sensorNumber,
			l_ylow,l_yhigh,l_ybin ));
}

void DataMonitorAlgorithm::fillHistogram2D()
{
	int channelNumber=RawData<>::getnChannelNumber();
	for(int chan = 0; chan <channelNumber; chan++ ){
		auto channelSignal=m_data.getSignal(chan);
		if(0!=channelSignal) // no need to push masked values
			m_histogram2D->Fill(chan+RawData<>::getMinChannel(),channelSignal);
	  }
}

DataMonitorAlgorithm::RunPhase DataMonitorAlgorithm::getRunPhase()
{
	if(m_evtNumber< m_skipEvent) return SKIP;
	if( m_evtNumber<m_displayEventNumber+m_skipEvent) return SAVE_SINGLE_EVENTS;
	else return REST;
}

void DataMonitorAlgorithm::buildProjectionHistogram()
{
		m_histogram2D->ProjectionY(m_outpuProjectionHistogramName.c_str());
}
