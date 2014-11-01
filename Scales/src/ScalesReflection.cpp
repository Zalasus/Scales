/*
 * ScalesReflection.cpp
 *
 *  Created on: 06.07.2014
 *      Author: Zalasus
 */

#include "ScalesReflection.h"

namespace Scales
{

	ReflectedClassFactory::~ReflectedClassFactory()
	{
		for(uint32_t i = 0; i < vars.size(); i++)
		{
			delete vars[i];
		}
	}

	String ReflectedClassFactory::getLinkName()
	{
		return linkName;
	}

}

