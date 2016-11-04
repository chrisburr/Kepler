/*
 * TbUTChannelMaskFileValidator.cpp
 *
 *  Created on: Oct 10, 2014
 *      Author: ADendek
 */

#include "TbUTChannelMaskFileValidator.h"
#include <iostream>

using namespace TbUT;
using namespace boost::filesystem;
using namespace std;

ChannelMaskFileValidator:: ChannelMaskFileValidator( std::string& p_filename):
		m_filename(p_filename)
{
}

 bool ChannelMaskFileValidator::validateFile()
 {
	 m_path=path(m_filename);
	 bool l_result=
			  isfileExist() &&
			  isRegularFile() &&
			  hasProperSize();
	  return l_result;
 }

 bool ChannelMaskFileValidator::isfileExist()
 {
	 return exists(m_path);
 }

 bool ChannelMaskFileValidator::isRegularFile()
 {
	return  is_regular_file(m_path);
 }

 bool ChannelMaskFileValidator::hasProperSize()
 {
	unsigned int l_expectedSieze=1024;
	cout<<file_size(m_path)<<endl;
	return l_expectedSieze==file_size(m_path);
 }
