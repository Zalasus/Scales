/*
 * ScalesObject.cpp
 *
 *  Created on: 10.07.2014
 *      Author: Niklas Weissner
 */


#include "ScalesObject.h"

namespace Scales
{

	Object::Object(const Class *pClass)
		: myClass(pClass)
	{
		//Allocate all global variables, including the ones of all the superclasses

		const std::vector<VariablePrototype*> globalProtos = myClass->getGlobals(true);
		for(uint32_t i = 0; i < globalProtos.size(); i++)
		{
			Variable *v = new Variable(globalProtos[i]);
			variables.push_back(v);
		}
	}

	Object::~Object()
	{
		for(uint32_t i = 0; i < variables.size(); i++)
		{
			delete variables[i];
		}
	}

	Value::ValueType Object::getValueType() const
	{
		return Value::VT_OBJECT;
	}

	Class *Object::getClass()
	{
		return myClass;
	}

	Variable *Object::getVariable(const String &name)
	{
		for(uint32_t i = 0; i < variables.size(); i++)
		{
			if(variables[i]->getPrototype()->getName() == name)
			{
				return variables[i];
			}
		}
	}

}

