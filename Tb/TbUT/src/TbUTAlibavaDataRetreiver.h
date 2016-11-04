/*
 * TbUTAlibava.h
 *
 *  Created on: Oct 1, 2014
 *      Author: ADendek
 */

#pragma once

#include "alibava/TbAsciiRoot.h"
#include "TbUTIDataRetreiver.h"

namespace TbUT
{
class AlibavaDataRetreiver: public IDataRetreiver
{

public:
	AlibavaDataRetreiver();
	void open(std::string & p_filename);
	bool valid();
	int	read_event(std::string & error_code);
	void process_event();
	double	time();
	double	temp();
	unsigned short data(int i);

private:
	TbAsciiRoot m_assciRoot;
};

}

