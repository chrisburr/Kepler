/*
 * TbUTIDataReader.h
 *
 *  Created on: Oct 6, 2014
 *      Author: ADendek
 */

#pragma once

#include "TbUTRawData.h"
#include <stdexcept>
#include <string>

namespace TbUT
{
class IDataReader
{
public:

	class InputFileError: public std::runtime_error
	{
	public:
		InputFileError(std::string& msg) :
				std::runtime_error(msg)
		{
		}
	};

	class ReadEventError: public std::runtime_error
	{
	public:
		ReadEventError(std::string & ex) :
				std::runtime_error(ex)
		{
		}
	};

	class NoMoreEvents: public std::runtime_error
	{
	public:
		NoMoreEvents(std::string & ex) :
				std::runtime_error(ex)
		{
		}
	};


	virtual ~IDataReader(){};
	virtual void checkInput()=0;
	virtual RawData<>* getEventData()=0;
};

}



