/*
 * TbUTPedestalRetreiver.cpp
 *
 *  Created on: Jan 2, 2015
 *      Author: ADendek
 */
#include "TbUTPedestalRetreiver.h"

#include "TbUTRawData.h"
#include <iostream>
#include <fstream>

using namespace TbUT;
using namespace std;

PedestalRetreiver::PedestalRetreiver(Pedestal& p_pedestal, IFileValidator& p_fileValidator, const std::string& p_filename):
		m_isFillingPedestalRequited(true),
		m_pedestal(p_pedestal),
		m_fileValidator(p_fileValidator),
		m_filename(p_filename)
{
}

void PedestalRetreiver::savePedestalToFile(const std::string& /*p_filename*/)
{
	cout<<"Not saving Pedestal!"<<endl;
// do not save nothing!
}


StatusCode PedestalRetreiver::processEvent(RawData<>* /*p_data*/)
{
	if(!m_isFillingPedestalRequited){
		cout<<"PedestalRetreiver===> pedestals ok!"<<endl;
		return StatusCode::SUCCESS;
	}

	if(!m_fileValidator.validateFile() ){
		cout<<"validation error!"<<endl;
		return StatusCode::FAILURE;
	}
	getPedestalFromFile();
	return StatusCode::SUCCESS;
}

void PedestalRetreiver::getPedestalFromFile()
{
	cout<<"PedestalRetreiver===>getting pedestals from file: "<< m_filename<<endl;
	int l_channelsNumber=RawData<>::getnChannelNumber();
	ifstream l_file(m_filename);
	if(!l_file.good()){
		std::string lerrorMsg="Cannot open input pedestal file: "+m_filename;
		throw PedestalCalculatorError(lerrorMsg);
	}

	for(int channel=0;channel<l_channelsNumber;channel++)
	{
		double l_pedestalFromFile=0;
		l_file >> l_pedestalFromFile;
		m_pedestal.setPedestal(channel,l_pedestalFromFile);
		cout<<"PedestalRetreiver===> channel: "<< channel <<"pedestal: "<<l_pedestalFromFile<<endl;
	}
	m_pedestal.setNormalizationFlag(true);
	m_isFillingPedestalRequited=false;
}
