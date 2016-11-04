/*
 * TbUTRawData<>Factory.h
 *
 *  Created on: Mar 16, 2015
 *      Author: ADendek
 */

#pragma once

#include "TbUTIDataReader.h"
#include "TbUTRawData.h"
#include "TbUTAlibavaDataRetreiver.h"
#include "TbUTAlbavaFileValidator.h"
#include <memory>

namespace TbUT
{

namespace InputDataOption
{
static const std::string& Alibava = "alibava";
static const std::string& NoiseGenerator = "Generator";
static const std::string& Mamba = "Mamba";
}


class RawDataFactory
{
public:
	typedef std::shared_ptr<IDataReader> DataReaderPtr;

	class NoSuchState: public std::runtime_error
	{
	public:
		NoSuchState(const std::string& p_errorMsg ):
			std::runtime_error(p_errorMsg)
		{
		}
	};

	RawDataFactory(std::string& p_filename,
			IDataRetreiver & p_alibava,
			IFileValidator& p_fileValidator,
			bool& p_isAType,
			double& p_mean,
			double& p_sigma);
	DataReaderPtr createDataEngine(const std::string& p_inputDataOption);
	private:
	std::string& m_filename;
	IDataRetreiver & m_alibava;
	IFileValidator& m_fileValidator;
	bool& m_isAType;
	double& m_mean;
	double& m_sigma;

};

} /* namespace TbUT */

