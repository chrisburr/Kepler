/*
 * TbUTIDataRetreiver.h
 *
 *  Created on: Oct 1, 2014
 *      Author: ADendek
 */

#pragma once
#include <string>

namespace TbUT
{

class IDataRetreiver
{
public:

	virtual ~IDataRetreiver()
	{
	}
	;
	virtual void open(std::string & p_filename)=0;
	virtual bool
	valid()=0;
	virtual int
	read_event(std::string & error_code)=0;
	virtual void
	process_event()=0;
	virtual double
	time()=0;
	virtual double
	temp()=0;
	virtual unsigned short
	data(int i) =0;

};

}
