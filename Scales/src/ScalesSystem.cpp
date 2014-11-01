/*
 * ScalesSystem.h
 *
 *  Created on: 06.07.2014
 *      Author: Zalasus
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

		//Delete all memory cells that weren't garbage collected already
		for(uint32_t i  = 0; i < memory.size(); i++)
		{
			if(memory[i] != null) //check is actually unnecessary, just included for clarity
			{
				delete memory[i];
			}
		}
	}

	Class *ScalesSystem::createClass(const ClassPrototype &proto)
	{
		Class *cl = new Class(proto, this);

		classes.push_back(cl);

		return cl;
	}

	Class *ScalesSystem::getClass(const ClassId &id)
	{
		for(uint32_t i = 0; i < classes.size(); i++)
		{
			if(classes[i]->getPrototype().getClassId() == id)
			{
				return classes[i];
			}
		}

		return null;
	}

	bool ScalesSystem::registerValueForGC(Value *val)
	{
		if(val == null)
		{
			return false;
		}

		//Ensure we are not already managing this instance
		for(uint32_t i = 0; i < memory.size(); i++)
		{
			if(val == memory[i])
			{
				return false;
			}
		}

		memory.push_back(val);

		return true;
	}

	bool ScalesSystem::deallocateValue(Value *val)
	{
		if(val == null)
		{
			return false;
		}

		//Ensure we are really managing this instance
		for(uint32_t i = 0; i < memory.size(); i++)
		{
			if(val == memory[i])
			{
				delete val;
				memory[i] = null;

				//TODO: We could remove the pointer from memory now, but this may cause more swapping and delay than 4 bytes of pointer memory are actually worth of.
			}
		}

		return false;
	}

}
