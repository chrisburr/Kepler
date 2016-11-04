
#include "TbUTNTupleCreator.h"
#include "GaudiKernel/AlgFactory.h"

using namespace TbUT;

DECLARE_NAMESPACE_ALGORITHM_FACTORY(TbUT,NTupleCreator)


NTupleCreator::NTupleCreator(const std::string& name, ISvcLocator* pSvcLocator):
		GaudiTupleAlg(name, pSvcLocator),
		m_eventNumber(0)
{
	  declareProperty("StoreEventNumber", m_storeEventNumber = 100);

	  declareProperty("WriteRaw",       m_isRawWritten = true);
	  declareProperty("WriteHeader",    m_isHeaderWritten = true);
	  declareProperty("WritePedestal",  m_isPedestalWritten = true);
	  declareProperty("WriteCMS",       m_isCMSWritten = true);
	  declareProperty("WriteClusters",  m_isClusterWritten = true);

	  declareProperty("RawLocation",      m_rawLocation = TbUT::DataLocations::RawTES );
	  declareProperty("HeaderLocation",   m_headerLocation = TbUT::DataLocations::HeaderTES );
	  declareProperty("PedestalLocation", m_pedestalLocation = TbUT::DataLocations::PedestalTES );
	  declareProperty("CMSLocation",      m_cmsLocation = TbUT::DataLocations::CMSTES );
	  declareProperty("ClusterLocation",  m_clusterLocation = TbUT::DataLocations::Clusters_TES );

}

StatusCode NTupleCreator::initialize()
{
	StatusCode sc = GaudiTupleAlg::initialize();
	if (sc.isFailure()) return sc;
	if(m_isRawWritten) bookRawData();
	if(m_isHeaderWritten) bookHeaderData();
	if(m_isPedestalWritten) bookPedestal();
	if(m_isCMSWritten) bookCMS();
	if(m_isClusterWritten) bookClusters();

	return StatusCode::SUCCESS;
}

StatusCode NTupleCreator::execute()
try{
	if(m_eventNumber<m_storeEventNumber)
	{
		if(m_isRawWritten) fillRawData();
		if(m_isHeaderWritten) fillHeaderData();
		if(m_isPedestalWritten) fillPedestal();
		if(m_isCMSWritten) fillCMS();
		if(m_isClusterWritten) fillClusters();
		m_eventNumber++;
	}
	return StatusCode::SUCCESS;
}catch(DataError & err){
	error()<<err.what()<<endmsg;
	return StatusCode::FAILURE;
}


void NTupleCreator::bookRawData()
{
	NTupleFilePtr file1(ntupleSvc(), "/NTUPLES/FILE1");
	NTuplePtr ntuple(ntupleSvc(), "/NTUPLES/FILE1/TbUT/RawData");
	if (ntuple) return;
	ntuple = ntupleSvc()->book("/NTUPLES/FILE1/TbUT/RawData",CLID_ColumnWiseTuple, "TbUT nTuple");
	ntuple->addItem("rawData",RawData<>::getnChannelNumber(), m_rawSignal);
}

void NTupleCreator::bookHeaderData()
{
 
  const unsigned int maxHeaderContainerSize=16;
	NTupleFilePtr file1(ntupleSvc(), "/NTUPLES/FILE1");
	NTuplePtr ntuple(ntupleSvc(), "/NTUPLES/FILE1/TbUT/HeaderData");
	if (ntuple) return;
	ntuple = ntupleSvc()->book("/NTUPLES/FILE1/TbUT/HeaderData",CLID_ColumnWiseTuple, "TbUT nTuple");
	ntuple->addItem("headerData0",maxHeaderContainerSize, m_header0Signal);
	ntuple->addItem("headerData1",maxHeaderContainerSize, m_header1Signal);
	ntuple->addItem("headerData2",maxHeaderContainerSize, m_header2Signal);
	ntuple->addItem("headerData3",maxHeaderContainerSize, m_header3Signal);
	ntuple->addItem("headerData3P1",maxHeaderContainerSize, m_header3P1Signal);
	ntuple->addItem("headerData3P2",maxHeaderContainerSize, m_header3P2Signal);
}

void NTupleCreator::bookPedestal()
{
	NTupleFilePtr file1(ntupleSvc(), "/NTUPLES/FILE1");
	NTuplePtr ntuple(ntupleSvc(), "/NTUPLES/FILE1/TbUT/Pedestal");
	if (ntuple) return;
	ntuple = ntupleSvc()->book("/NTUPLES/FILE1/TbUT/Pedestal",CLID_ColumnWiseTuple, "TbUT nTuple");
	ntuple->addItem("pedestalData",RawData<>::getnChannelNumber(), m_pedestalSignal);
}

void NTupleCreator::bookCMS()
{
	NTupleFilePtr file1(ntupleSvc(), "/NTUPLES/FILE1");
	NTuplePtr ntuple(ntupleSvc(), "/NTUPLES/FILE1/TbUT/CMS");
	if (ntuple) return;
	ntuple = ntupleSvc()->book("/NTUPLES/FILE1/TbUT/CMS",CLID_ColumnWiseTuple, "TbUT nTuple");
	ntuple->addItem("cmsData",RawData<>::getnChannelNumber() ,m_cmsSignal);
}

void NTupleCreator::bookClusters()
{
	NTupleFilePtr file1(ntupleSvc(), "/NTUPLES/FILE1");
	NTuplePtr ntuple(ntupleSvc(), "/NTUPLES/FILE1/TbUT/Clusters");
	if (ntuple) return;

	const unsigned int maxClusterContainerSize=10;
	ntuple = ntupleSvc()->book("/NTUPLES/FILE1/TbUT/Clusters",CLID_ColumnWiseTuple, "TbUT nTuple");
	ntuple->addItem("clusterNumberPerEvent",m_clusterNumberPerEvent);
	ntuple->addItem("clustersTDC", m_tdc);
	ntuple->addItem("timestamps", m_timestamp);

	ntuple->addItem("clustersPosition",maxClusterContainerSize ,m_clusterPosition);
	ntuple->addItem("clustersSeedPosition",maxClusterContainerSize ,m_clusterSeedPosition);

	ntuple->addItem("clustersCharge",maxClusterContainerSize, m_clusterCharge);
	ntuple->addItem("clustersSize",maxClusterContainerSize ,m_clusterSize);

	ntuple->addItem("clustersSeedCharge",maxClusterContainerSize ,m_clusterSeedCharge);
	ntuple->addItem("clustersCharge2StripLeft",maxClusterContainerSize ,m_clusterCharge2StripLeft);
	ntuple->addItem("clustersCharge1StripLeft",maxClusterContainerSize ,m_clusterCharge1StripLeft);
	ntuple->addItem("clustersCharge1StripRight",maxClusterContainerSize ,m_clusterCharge1StripRight);
	ntuple->addItem("clustersCharge2StripRight",maxClusterContainerSize ,m_clusterCharge2StripRight);

}

void NTupleCreator::fillRawData()
{
	using namespace std;
	RawDataContainer<>* dataContainer=getDataFromTES<>(m_rawLocation);
	for(const auto& rawData : dataContainer->getData())// iterate over rawDatas
	{
		for(int channel=0;channel<RawData<>::getnChannelNumber();channel++){
			m_rawSignal[channel]=rawData.getSignal(channel);
		}
	    ntupleSvc()->writeRecord("/NTUPLES/FILE1/TbUT/RawData");
	}
}
void NTupleCreator::fillHeaderData()
{
  const unsigned int maxHeaderContainerSize=16;
	RawDataContainer<>* dataContainer=getDataFromTES(m_rawLocation);
	for(const auto& rawData : dataContainer->getData())// iterate over rawDatas
	{
		for(unsigned int channel=0;channel<maxHeaderContainerSize;channel++){
			m_header0Signal[channel]=rawData.getHeader0(channel);
			m_header1Signal[channel]=rawData.getHeader1(channel);
			m_header2Signal[channel]=rawData.getHeader2(channel);
			m_header3Signal[channel]=rawData.getHeader3(channel);
			m_header3P1Signal[channel]=rawData.getHeader3P1(channel);
			m_header3P2Signal[channel]=rawData.getHeader3P2(channel);
		}
	    ntupleSvc()->writeRecord("/NTUPLES/FILE1/TbUT/HeaderData");
	}
}

void NTupleCreator::fillPedestal()
{
	RawDataContainer<>* dataContainer=getDataFromTES<>(m_pedestalLocation);
	for(const auto& data: dataContainer->getData()) // iterate over rawDatas
	{
		for(int channel=0;channel<RawData<>::getnChannelNumber();channel++){
			m_pedestalSignal[channel]=data.getSignal(channel);
		}
	    ntupleSvc()->writeRecord("/NTUPLES/FILE1/TbUT/Pedestal");
	}
}

void NTupleCreator::fillCMS()
{
	RawDataContainer<double>* dataContainer=getDataFromTES<double>(m_cmsLocation);
	for(const auto& cmsData:  dataContainer->getData())// iterate over rawDatas
	{
		for(int channel=0;channel<RawData<>::getnChannelNumber();channel++){
			m_cmsSignal[channel]=cmsData.getSignal(channel);
		}
	    ntupleSvc()->writeRecord("/NTUPLES/FILE1/TbUT/CMS");
	}
}
template <typename DATA_TYPE>
RawDataContainer<DATA_TYPE>* NTupleCreator::getDataFromTES(const std::string& p_location)
{
	RawDataContainer<DATA_TYPE>* dataContainer=getIfExists<RawDataContainer<DATA_TYPE> >(p_location);
	if(!dataContainer){
		std::string errorMsg="There is no data in: " +p_location;
		throw DataError(errorMsg);
	}
	return  dataContainer;
}

ClusterContainer* NTupleCreator::getClustersFromTES()
{
	ClusterContainer* clusterContainer=getIfExists<ClusterContainer>(m_clusterLocation);
	if(!clusterContainer){
		std::string errorMsg="There is no clusters in: " +m_clusterLocation;
		throw DataError(errorMsg);
	}
	return  clusterContainer;
}

void NTupleCreator::fillClusters()
{
	ClusterContainer* clusterContainer=getClustersFromTES();
	m_tdc=clusterContainer->getTDC();
	m_timestamp=clusterContainer->getTimestamp();
	if(clusterContainer->isEmpty()){
		m_clusterNumberPerEvent=0;
		ntupleSvc()->writeRecord("/NTUPLES/FILE1/TbUT/Clusters");
		return;
	}
	int clusterPerEvent=clusterContainer->size();
	m_clusterNumberPerEvent=clusterPerEvent;
	const unsigned int maxClusterContainerSize=10;
	unsigned int clusterCounter=0;
	for(const auto& cluster: clusterContainer->getClusters())
	{
		m_clusterSize[clusterCounter]=cluster.m_size;
		m_clusterCharge[clusterCounter]=cluster.m_charge;
		m_clusterPosition[clusterCounter]=cluster.m_position;
		m_clusterSeedPosition[clusterCounter]=cluster.m_seedPosition;
		m_clusterSeedCharge[clusterCounter]=cluster.m_chargeSeed;
		m_clusterCharge2StripLeft[clusterCounter]=cluster.m_charge2StripLeft;
		m_clusterCharge1StripLeft[clusterCounter]=cluster.m_charge1StripLeft;
		m_clusterCharge2StripRight[clusterCounter]=cluster.m_charge2StripRight;
		m_clusterCharge1StripRight[clusterCounter]=cluster.m_charge1StripRight;
		clusterCounter++;
		if(clusterCounter>=maxClusterContainerSize) break;
	}
	ntupleSvc()->writeRecord("/NTUPLES/FILE1/TbUT/Clusters");
}
