/*
 * TbUTRawData<>Reader.cpp
 *
 *  Created on: Oct 5, 2014
 *      Author: ADendek
 */
#include "TbUTAlibavaDataReader.h"
#include <iostream>
using namespace TbUT;


AlibavaDataReader::AlibavaDataReader(std::string& p_filename  , IDataRetreiver & p_alibava, IFileValidator& p_fileValidator):
				m_filaname(p_filename ),
				m_alibava(p_alibava),
				m_fileValidator(p_fileValidator)
{
}

void AlibavaDataReader::checkInput()
{
	if( ! m_fileValidator.validateFile() ){
		std::string errorMsg= "file validation error";
		throw IDataReader::InputFileError(errorMsg);
	}

	m_alibava.open(m_filaname);
	if(! m_alibava.valid()){
		std::string errorMsg= "alibava validation error";
		throw IDataReader::InputFileError(errorMsg);
	}
}

RawData<>* AlibavaDataReader::getEventData()
{
	readEvent();
	RawData<> *l_outputData=new RawData<>();
	fillRawData(l_outputData);
	return l_outputData;
}


void AlibavaDataReader::readEvent()
{
	std::string l_errorStr="";
	if(0 != m_alibava.read_event(l_errorStr))
	{
		throw IDataReader::ReadEventError(l_errorStr);
	}
}

void AlibavaDataReader::fillRawData(RawData<>* p_outputData)
{
	m_alibava.process_event();
	p_outputData->setTime( m_alibava.time() );
	p_outputData->setTemp( m_alibava.temp() );
	for(int chan = RawData<>::getMinChannel(); chan < RawData<>::getMaxChannel(); chan++ )
		p_outputData->setSignal( m_alibava.data(chan));

}

