/*
 * TbUTPedestalFileValidator.cpp
 *
 *  Created on: Jan 2, 2015
 *      Author: ADendek
 */

#include "TbUTPedestalFileValidator.h"

using namespace TbUT;
using namespace boost::filesystem;


PedestalFileValidator::PedestalFileValidator(const std::string& p_filename):
		m_filename(p_filename)
{
}


bool PedestalFileValidator::validateFile()
{
	m_path=path(m_filename);
	 bool l_result=
			  isfileExist() &&
			  isRegularFile() &&
			  hasNonZeroSize();
	 return l_result;
}

bool PedestalFileValidator::isfileExist()
{
	 return exists(m_path);
}

bool PedestalFileValidator::isRegularFile()
{
	return  is_regular_file(m_path);
}

bool PedestalFileValidator::hasNonZeroSize()
{
	return !(0==file_size(m_path));
}
