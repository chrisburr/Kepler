/*
 * TbUTChannelMaskProvider.h
 *
 *  Created on: Oct 11, 2014
 *      Author: ADendek
 */

#pragma once

#include "TbUTIChannelMaskProvider.h"
#include "TbUTIFileValidator.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>


namespace TbUT
{
class ChannelMaskProvider: public IChannelMaskProvider
{
public:

	class InputFileError: public std::runtime_error	{
	public:
		InputFileError(const std::string& p_errorMsg ):
				std::runtime_error(p_errorMsg)
		{
		}
	};

	class NotEnoughtChannel: public std::runtime_error	{
	public:
		NotEnoughtChannel(const std::string& p_error) :
				std::runtime_error(p_error)
		{
		}
	};

	class ChannelOutOfRange: public std::runtime_error	{
	public:
		ChannelOutOfRange(const std::string& p_error) :
				std::runtime_error(p_error)
		{
		}
	};

	ChannelMaskProvider(IFileValidator & p_fileValidator);
	void getMaskFromFile(const std::string& p_filename=TbUT::DataLocations::MaskLocation);
	bool isMasked(int p_channel);

private:
	typedef std::vector<unsigned short> MaskVector;

	void validateFile();
	void openfile(const std::string& p_filename, std::ifstream& p_inputFile);
	void retreiveMasksFromFile(std::ifstream& p_file);
	void checkMaskVector();
	bool isChannelInRange(int p_channel);



	IFileValidator & m_fileValidator;
	MaskVector m_masks;
};
}


