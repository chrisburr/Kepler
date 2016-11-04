/*
 * TbUTPedestalFileValidator.h
 *
 *  Created on: Jan 2, 2015
 *      Author: ADendek
 */

#include "TbUTIFileValidator.h"
#include <boost/filesystem.hpp>
#include <string>

namespace TbUT
{

class PedestalFileValidator: public IFileValidator
{
public:
	PedestalFileValidator(const std::string& p_filename);
	bool validateFile();

private:
	bool isfileExist();
	bool isRegularFile();
	bool hasNonZeroSize();

	const std::string& m_filename;
	boost::filesystem::path  m_path;
};

} /* namespace TbUT */

