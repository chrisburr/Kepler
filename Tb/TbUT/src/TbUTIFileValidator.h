/*
 * TbUTIFileValidator.h
 *
 *  Created on: Oct 5, 2014
 *      Author: ADendek
 */

#pragma once

#include <string>

namespace TbUT
{
class IFileValidator
{
public:
	virtual ~IFileValidator(){};
	virtual bool validateFile()=0;
};
}
