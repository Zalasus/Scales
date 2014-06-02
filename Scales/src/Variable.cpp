/*
 * Variable.cpp
 *
 *  Created on: 01.05.2014
 *      Author: Niklas Weissner
 */


#include "Variable.h"

namespace Scales
{

	Variable::Variable(const String &pName, const DataType &pType, const AccessType &pAccessType)
	:
			name(pName),
			type(pType),
			accessType(pAccessType),
			scope(0)
	{

	}

	String Variable::getName() const
	{
		return name;
	}

	DataType Variable::getType() const
	{
		return type;
	}

	AccessType Variable::getAccessType() const
	{
		return accessType;
	}

	void Variable::setScope(uint32_t i)
	{
		scope = i;
	}

	uint32_t Variable::getScope() const
	{
		return scope;
	}
}

