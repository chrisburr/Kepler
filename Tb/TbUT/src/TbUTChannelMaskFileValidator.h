/*
 * TbUTChannelMaskFileValidator.h
 *
 *  Created on: Oct 10, 2014
 *      Author: ADendek
 */

#pragma once
#include "TbUTIFileValidator.h"
#include <boost/filesystem.hpp>
#include <string>

namespace TbUT
{
class ChannelMaskFileValidator: public IFileValidator
{
public:
	ChannelMaskFileValidator( std::string& p_filename);
	bool validateFile();

private:
	bool isfileExist();
	bool isRegularFile();
	bool hasProperSize();


	std::string& m_filename;
	boost::filesystem::path  m_path;
};
}

