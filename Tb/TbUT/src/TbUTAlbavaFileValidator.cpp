/*
 * TbUTAlbavaFileValidator.cpp
 *
 *  Created on: Oct 5, 2014
 *      Author: ADendek
 */
#include "TbUTAlbavaFileValidator.h"
#include <string>
#include <iostream>
using namespace TbUT;
using namespace boost::filesystem;
using namespace std;


 AlbavaFileValidator:: AlbavaFileValidator( std::string& p_filename):
		m_filename(p_filename)
{
}

 bool AlbavaFileValidator::validateFile()
 {
	 m_path=path(m_filename);
	 bool l_result=
			  isfileExist() &&
			  isRegularFile() &&
			  hasNonZeroSize();
	 return l_result;
 }

 bool AlbavaFileValidator::isfileExist()
 {
	 return exists(m_path);
 }

 bool AlbavaFileValidator::isRegularFile()
 {
	 return  is_regular_file(m_path);
 }

 bool AlbavaFileValidator::hasNonZeroSize()
 {
	 return !(0==file_size(m_path));
 }
