/*
 * TbUTIProcessingEngine.h
 *
 *  Created on: Oct 15, 2014
 *      Author: ADendek
 */

#include "TbUTRawData.h"

namespace TbUT
{
template<typename InputDataType = int , typename OutputDataType = int >
class IProcessingEngine
{
public:
	virtual ~IProcessingEngine(){};
	virtual void processEvent(RawData<InputDataType>* p_data, RawData<OutputDataType> **p_output)=0;
};
}
