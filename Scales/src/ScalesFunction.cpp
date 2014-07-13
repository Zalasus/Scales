/*
 * ScalesFunction.cpp
 *
 *  Created on: 08.07.2014
 *      Author: Niklas Weissner
 */


#include "ScalesFunction.h"


namespace Scales
{

	Function::Function(const String &pName, const TypeList &pParamTypes, const DataType &pReturnType, bool pNative)
		: name(pName),
		  paramTypes(pParamTypes),
		  returnType(pReturnType),
		  native(pNative),
		  hasAdr(false),
		  adress(0)
	{
	}

	Function::Function(const String &pName, const TypeList &pParamTypes, const DataType &pReturnType, bool pNative, uint32_t pAdress)
			: name(pName),
			  paramTypes(pParamTypes),
			  returnType(pReturnType),
			  native(pNative),
			  hasAdr(true),
			  adress(pAdress)
	{
	}

	void Function::setAdress(uint32_t adr)
	{
		adress = adr;
		hasAdr = true;
	}

}
