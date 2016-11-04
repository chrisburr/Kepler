/*
 * TbUTMamdaDataReader.h
 *
 *  Created on: Jun 24, 2015
 *      Author: ADendek
 */
#pragma once

#include "TbUTIDataReader.h"
#include "TbUTIFileValidator.h"
#include "TbUTRawData.h"
#include "mamba/mamba_decoder.h"
#include <string>
#include <fstream>
#include <vector>
#include <map>



namespace TbUT
{

class MambaDataReader: public IDataReader
{
public:
	MambaDataReader(std::string& p_fileName, IFileValidator& p_fileValidator, bool& p_isAType );

	void checkInput();
	RawData<>* getEventData();


private:
	void fillRawData(RawData<>* p_outputData);

	std::string& m_fileName;
	IFileValidator& m_fileValidator;
	mamba_decoder m_decoder;
};



} /* namespace TbUT */

