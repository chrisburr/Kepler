/*
 * TbUTChannelMaskProvider.cpp
 *
 *  Created on: Oct 11, 2014
 *      Author: ADendek
 */

#include "TbUTChannelMaskProvider.h"
#include"TbUTRawData.h"
#include <iostream>

using namespace TbUT;
using namespace std;


ChannelMaskProvider::ChannelMaskProvider(IFileValidator & p_fileValidator):
		m_fileValidator(p_fileValidator),
		m_masks()
{
}

void ChannelMaskProvider::getMaskFromFile(const string& p_filename)
{
	validateFile();
	ifstream l_file;
	openfile(p_filename,l_file);
	retreiveMasksFromFile(l_file);
	checkMaskVector();
}

bool ChannelMaskProvider::isMasked(int p_channel)
{
	if(!isChannelInRange(p_channel)){
		std::cout<<"channel out of range"<<p_channel<<std::endl;
		throw ChannelOutOfRange("");
	}
	return (m_masks[p_channel]==0);
}



void ChannelMaskProvider::validateFile()
{
	if(!m_fileValidator.validateFile())
		throw InputFileError("Mask File validation error!");

}

void ChannelMaskProvider::openfile(const std::string& p_filename, std::ifstream& p_inputFile)
{
	p_inputFile.open(p_filename.c_str(),ios::in);
	if(!p_inputFile.good())
		throw InputFileError("Cannot open mask file!"+p_filename);
}

void ChannelMaskProvider::retreiveMasksFromFile(std::ifstream& p_file)
{
	int l_sensorNumber= RawData<>::getnChannelNumber();
	for(int channel=0;channel<l_sensorNumber;channel++){
		unsigned short l_mask;
		p_file>>l_mask;
		m_masks.push_back(l_mask);
	}
}

void ChannelMaskProvider::checkMaskVector()
{
	const unsigned int l_sensorNumber=RawData<>::getnChannelNumber();
	if (m_masks.size() !=l_sensorNumber)
		throw NotEnoughtChannel("");
}

bool ChannelMaskProvider::isChannelInRange(int p_channel)
{
	int l_sensorNumber= RawData<>::getnChannelNumber();
	return p_channel<l_sensorNumber;
}
