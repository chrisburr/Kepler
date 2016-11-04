/*
 * TbUTPedestalRetreiver.h
 *
 *  Created on: Jan 2, 2015
 *      Author: ADendek
 */

#pragma once

#include "TbUTIPedestalFollowing.h"
#include "TbUTPedestal.h"
#include "TbUTIFileValidator.h"

namespace TbUT
{

class PedestalRetreiver: public IPedestalFollowing
{
public:
	PedestalRetreiver(Pedestal& p_pedestal, IFileValidator& p_fileValidator, const std::string& p_filename);

	StatusCode processEvent(RawData<>* p_data);
	void savePedestalToFile(const std::string& p_filename);

private:
	void getPedestalFromFile();

	bool m_isFillingPedestalRequited;
	Pedestal & m_pedestal;
	IFileValidator& m_fileValidator;
	const std::string& m_filename;

};

} /* namespace TbUT */

