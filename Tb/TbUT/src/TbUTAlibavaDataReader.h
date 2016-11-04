/*
 * TbUTDataReader.h
 *
 *  Created on: Oct 1, 2014
 *      Author: ADendek
 */

#pragma once

#include "TbUTIDataReader.h"
#include "TbUTRawData.h"
#include "TbUTIFileValidator.h"
#include "TbUTIDataRetreiver.h"
#include <stdexcept>
#include <string>

namespace TbUT
{
class AlibavaDataReader : public IDataReader
{
public:

	AlibavaDataReader(std::string& p_filename ,IDataRetreiver & p_alibava,IFileValidator& p_fileValidator);
	void checkInput();
	RawData<>* getEventData(); // unfortunately have to be pointer

private:

	void readEvent();
	void fillRawData(RawData<>* p_outputData);

	std::string & m_filaname;
	IDataRetreiver & m_alibava;
	IFileValidator& m_fileValidator;
};

}

