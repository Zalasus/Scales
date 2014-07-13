/*
 * ScalesSystem.h
 *
 *  Created on: 06.07.2014
 *      Author: Niklas Weissner
 */


#include "ScalesSystem.h"


namespace Scales
{

	ScalesSystem::ScalesSystem()
	{

	}

	ScalesSystem::~ScalesSystem()
	{
		//Delete all allocated prototype objects
		for(uint32_t i = 0; i < classes.size(); i++)
		{
			delete classes[i];
		}
	}

	Class *ScalesSystem::createClass(const ClassId &id, Class* superclass = 0, const String &nativeLinkTarget = "")
	{
		Class *cl = new Class(id, superclass, nativeLinkTarget, this);

		classes.push_back(cl);

		return cl;
	}

	Class *ScalesSystem::getClass(const ClassId &id)
	{
		for(uint32_t i = 0; i < classes.size(); i++)
		{
			if(classes[i]->getId() == id)
			{
				return classes[i];
			}
		}

		return null;
	}

}
