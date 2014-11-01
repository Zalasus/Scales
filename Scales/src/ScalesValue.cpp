/*
 * ScalesValue.cpp
 *
 *  Created on: 10.07.2014
 *      Author: Zalasus
 */

#include "ScalesValue.h"

namespace Scales
{

	ValuePtr::ValuePtr(Value *ref)
		: value(ref),
		  users(0)
	{

	}

	Value *ValuePtr::getValue()
	{
		return value;
	}

	uint32_t ValuePtr::getUserCount()
	{
		return users;
	}

	void ValuePtr::addUser(Variable *v)
	{
		users++;
	}

	void ValuePtr::removeUser(Variable *v)
	{
		if(users > 0)
		{
			users--;
		}
	}

}


