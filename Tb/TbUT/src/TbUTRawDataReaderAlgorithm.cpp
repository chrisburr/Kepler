#include "TbUTRawDataReaderAlgorithm.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/IEventProcessor.h"


using namespace TbUT;

DECLARE_NAMESPACE_ALGORITHM_FACTORY(TbUT,RawDataReaderAlgorithm)

RawDataReaderAlgorithm::RawDataReaderAlgorithm(const std::string& name, ISvcLocator* pSvcLocator):
	GaudiAlgorithm(name, pSvcLocator),
	m_isStandalone(true),
	m_isAType(true),
	m_eventNumber(0),
	m_skipEventNumber(0),
	m_alibavaInputData(),
	m_outputLocation(),
	m_sensorNumber(1),
	m_mean(0),
	m_sigma(1),
	m_alibava(),
	m_fileValidator(m_alibavaInputData),
	m_rawDataReader(),
	m_inputDataFactory(m_alibavaInputData, m_alibava,m_fileValidator, m_isAType,m_mean,m_sigma )
{
	declareProperty("InputDataType", m_inputDataOption=TbUT::InputDataOption::Mamba);
	declareProperty("SkipEventNumber", m_skipEventNumber=0);
	declareProperty("inputData", m_alibavaInputData="run_000007_2607_1639.ali");
	declareProperty("outputLoc", m_outputLocation=TbUT::DataLocations::RawTES );
	declareProperty("sensorNumber", m_sensorNumber=1 );
	declareProperty("standalone", m_isStandalone=true );
	declareProperty("isAType", m_isAType=true );


	declareProperty("sigma", m_sigma=0. );
	declareProperty("mean", m_mean=1. );
}


StatusCode RawDataReaderAlgorithm::initialize()
{
	StatusCode sc = GaudiAlgorithm::initialize();
	if (!sc.isSuccess()) return StatusCode::FAILURE;
	try {
		m_rawDataReader=m_inputDataFactory.createDataEngine(m_inputDataOption);
		info()<<"Create data engine done"<<endmsg;
	} catch(RawDataFactory::NoSuchState &ex) {
		error()<<"Error input in factory: "<< ex.what()<<endmsg;
		return StatusCode::FAILURE;
	}
	try {
		m_rawDataReader->checkInput();
		return StatusCode::SUCCESS;
	} catch(IDataReader::InputFileError &ex) {
		error()<<"input file error: "<< ex.what() <<endmsg;
		return StatusCode::FAILURE;
	}
	try {
		Sensor l_sensor(m_sensorNumber);
		RawData<>::setSensor(l_sensor);
	} catch(Sensor::SensorNumberError &ex) {
		error()<<"Error create sensor: "<< ex.what()<<endmsg;
		return StatusCode::FAILURE;
	}
	info()<<"initialization success!"<<endmsg;
	return StatusCode::SUCCESS;

}

StatusCode RawDataReaderAlgorithm::execute()
{
	uint64_t mambaOffset=0; // need to be tuned
	uint64_t keplerEventBegin = 0;
	uint64_t keplerEventEnd = 0;
	if(!m_isStandalone)
		timingSvc()->eventDefinition(keplerEventBegin, keplerEventEnd);
	else
		keplerEventEnd=1;// more than 0

	uint64_t mambaTimestamp=0;
	RawDataContainer<> * outputDataContainer=new RawDataContainer<>();
	while(mambaTimestamp+mambaOffset<keplerEventEnd){
		try{
			if(0==(m_eventNumber%1000) ) info()<<"Read event"<<m_eventNumber<<endmsg;
			RawData<> * outputData=m_rawDataReader->getEventData();
			outputDataContainer->addData(*outputData);
			m_eventNumber++;
			if(m_isStandalone) break; // do this loop only once
		}catch(IDataReader::ReadEventError& ex ){
			error()<<"event read error: "<< ex.what()<<endmsg;
			return StatusCode::RECOVERABLE;
		}catch(IDataReader::NoMoreEvents& ex){
			SmartIF<IEventProcessor> app(serviceLocator()->service("ApplicationMgr"));
			if (app){
				info()<<"No more event. Terminate!"<<endmsg;
				return app->stopRun();
			}
		}

	}
	put(outputDataContainer, m_outputLocation );
	return StatusCode::SUCCESS;
}


