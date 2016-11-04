/*
 * TbUTMamdaDataReader.cpp
 *
 *  Created on: Jun 24, 2015
 *      Author: ADendek
 */

#include "TbUTMambaDataReader.h"
#include <iostream>

using namespace TbUT;
using namespace std;

MambaDataReader::MambaDataReader(std::string& p_fileName, IFileValidator& p_fileValidator, bool& p_isAType):
				m_fileName(p_fileName),
				m_fileValidator(p_fileValidator),
				m_decoder(p_isAType)
{
}

void MambaDataReader::checkInput()
{
	if( ! m_fileValidator.validateFile() ){
		std::string errorMsg= "file validation error";
		throw IDataReader::InputFileError(errorMsg);

	}
	if(! m_decoder.open(m_fileName.c_str())){
		std::string errorMsg= "cannot open input file";
		throw IDataReader::InputFileError(errorMsg);
	}
}

RawData<>* MambaDataReader::getEventData()
{
	while(true){
		if(m_decoder.eof()){
			string l_errorMsg ="No more event!";
			throw IDataReader::NoMoreEvents(l_errorMsg);
		}
		if(0==m_decoder.read_event()){
			continue;
		}
		else break;
	}
	RawData<> *l_outputData=new RawData<>();
	fillRawData(l_outputData);
	return l_outputData;
}

void MambaDataReader::fillRawData(RawData<>* p_outputData)
{
	auto signalVector=m_decoder.ADC();
	auto headerVector0=m_decoder.BHeader0();
	auto headerVector1=m_decoder.BHeader1();
	auto headerVector2=m_decoder.BHeader2();
	auto headerVector3=m_decoder.BHeader3();
	auto headerVector3P1=m_decoder.BHeader3P1();
	auto headerVector3P2=m_decoder.BHeader3P2();
	for( const auto& l_adc : signalVector) 	p_outputData->setSignal(l_adc);
	for( const auto& l_hdr0 : headerVector0) 	p_outputData->setHeader0(l_hdr0);
	for( const auto& l_hdr1 : headerVector1) 	p_outputData->setHeader1(l_hdr1);
	for( const auto& l_hdr2 : headerVector2) 	p_outputData->setHeader2(l_hdr2);
	for( const auto& l_hdr3 : headerVector3) 	p_outputData->setHeader3(l_hdr3);
	for( const auto& l_hdr3P1 : headerVector3P1) 	p_outputData->setHeader3P1(l_hdr3P1);
	for( const auto& l_hdr3P2 : headerVector3P2) 	p_outputData->setHeader3P2(l_hdr3P2);
	p_outputData->setTime(m_decoder.TsTimestamp());
	p_outputData->setTDC(m_decoder.TDC());
}
