/*
 * ScalesFunction.cpp
 *
 *  Created on: 08.07.2014
 *      Author: Zalasus
 */

#include "ScalesFunction.h"

namespace Scales
{

	FunctionSketch::FunctionSketch(const String &pName, const TypeList &pParamTypes, const DataType &pReturnType, bool pPrivate, bool pNative)
		: name(pName),
		  paramTypes(pParamTypes),
		  returnType(pReturnType),
		  native(pNative),
		  hasAdr(false),
		  adress(0)
	{
	}

	FunctionSketch::FunctionSketch(const String &pName, const TypeList &pParamTypes, const DataType &pReturnType, bool pPrivate, bool pNative, uint32_t pAdress)
			: name(pName),
			  paramTypes(pParamTypes),
			  returnType(pReturnType),
			  native(pNative),
			  hasAdr(true),
			  adress(pAdress)
	{
	}

	void FunctionSketch::setAdress(uint32_t adr)
	{
		adress = adr;
		hasAdr = true;
	}

}
