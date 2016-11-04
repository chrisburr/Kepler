/*
 * TbUTRawDataFactory.cpp
 *
 *  Created on: Mar 16, 2015
 *      Author: ADendek
 */

#include "TbUTRawDataFactory.h"
#include "TbUTMambaDataReader.h"
#include "TbUTAlibavaDataReader.h"
#include "TbUTRandomNoiseGenerator.h"

using namespace TbUT;


RawDataFactory::RawDataFactory(std::string& p_filename,
		IDataRetreiver & p_alibava,
		IFileValidator& p_fileValidator,
		bool& p_isAType,
		double& p_mean,	double& p_sigma):
				m_filename(p_filename),
				m_alibava(p_alibava),
				m_fileValidator(p_fileValidator),
				m_isAType(p_isAType),
				m_mean(p_mean),
				m_sigma(p_sigma)
{
}

RawDataFactory::DataReaderPtr  RawDataFactory::createDataEngine(const std::string& p_inputDataOption)
{
	if(p_inputDataOption==InputDataOption::Alibava)
		return DataReaderPtr(new AlibavaDataReader(m_filename,m_alibava, m_fileValidator));
	if(p_inputDataOption == InputDataOption::Mamba)
		return DataReaderPtr(new MambaDataReader(m_filename,m_fileValidator,m_isAType ));
	if(p_inputDataOption==InputDataOption::NoiseGenerator)
		return DataReaderPtr(new RandomNoiseGenerator(m_mean,m_sigma));
	else
		throw NoSuchState(p_inputDataOption);
}
