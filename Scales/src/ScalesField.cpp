/*
 * ScalesField.cpp
 *
 *  Created on: 01.11.2014
 *      Author: Zalasus
 */

#include "ScalesField.h"

namespace Scales
{

	Field::Field(const String &pName, const DataType &pType, uint32_t pIndex)
	 : name(pName),
	   type(pType),
	   index(pIndex)
	{
	}

	String Field::getName() const
	{
		return name;
	}

	DataType Field::getType() const
	{
		return type;
	}

	uint32_t Field::getIndex() const
	{
		return index;
	}

}

