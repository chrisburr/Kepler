/*
 * TbUTAlbavaFileValidator.h
 *
 *  Created on: Oct 5, 2014
 *      Author: ADendek
 */

#pragma once
#include "TbUTIFileValidator.h"
#include <boost/filesystem.hpp>
#include <string>

namespace TbUT
{
class AlbavaFileValidator: public IFileValidator
{
public:
	AlbavaFileValidator( std::string& p_filename);
	bool validateFile();

private:
	bool isfileExist();
	bool isRegularFile();
	bool hasNonZeroSize();

	std::string& m_filename;
	boost::filesystem::path  m_path;
};
}
