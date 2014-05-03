/*
 * Script.cpp
 *
 *  Created on: 29.04.2014
 *      Author: Niklas Weissner
 */

#include "Script.h"

#include <iostream>

namespace Scales
{

	Function::Function(const String &pName, const vector<DataType> parameterTypes, DataType pReturnType, AccessType pAccessType, bool pNative, bool pEvent, uint32_t pAdress)
	:
			functionName(pName),
			paramTypes(parameterTypes),
			returnType(pReturnType),
			accessType(pAccessType),
			native(pNative),
			event(pEvent),
			adress(pAdress)
	{

	}

	bool Function::isEvent() const
	{
		return event;
	}

	bool Function::isNative() const
	{
		return native;
	}

	uint32_t Function::getAdress() const
	{
		return adress;
	}

	DataType Function::getReturnType() const
	{
		return returnType;
	}

	AccessType Function::getAccessType() const
	{
		return accessType;
	}

	String Function::getName() const
	{
		return functionName;
	}

	bool Function::is(const String &name, const vector<DataType> &parameterTypes) const
	{
		if(parameterTypes.size() != paramTypes.size())
		{
			return false;
		}

		for(uint32_t i = 0; i < parameterTypes.size(); i++)
		{
			if(!(parameterTypes[i].equals(paramTypes[i])))
			{
				return false;
			}
		}

		return functionName.equals(name);
	}

	String Function::createInfoString(const String &name, vector<DataType> &paramTypes)
	{
		String result = name + "(";

		for(uint32_t i = 0; i < paramTypes.size(); i++)
		{
			result += paramTypes[i].getTypeName();

			if(i < paramTypes.size() - 1)
			{
				result += ", ";
			}
		}

		return result + ")";
	}


	Script::Script(const String &pName, bool pStatic)
	:
			scriptname(pName),
			staticScript(pStatic)
	{

	}

	void Script::declareFunction(const Function &func)
	{
		functions.push_back(func);
	}

	Function *Script::getFunction(const String &name, const vector<DataType> &paramTypes)
	{

		for(uint32_t i = 0; i < functions.size(); i++)
		{

			if(functions[i].is(name, paramTypes))
			{
				return &functions[i];
			}
		}

		return null;
	}

	Variable *Script::getVariable(const String &name)
	{
		Variable *local = getLocal(name);

		if(local == null)
		{
			return getGlobal(name);
		}

		return null;
	}

	void Script::declareGlobal(Variable &v)
	{
		globals.push_back(v);
	}

	Variable *Script::getGlobal(const String &name)
	{
		for(uint32_t i = 0; i < globals.size(); i++)
		{

			if(globals[i].getName().equals(name))
			{
				return &globals[i];
			}
		}

		return null;
	}

	Variable *Script::getLocal(const String &name)
	{
		//TODO: Think of something really good to implement locals with correct scoping so they are usable for prototyping

		return null;
	}

	bool Script::isStatic() const
	{
		return staticScript;
	}

	String Script::getName() const
	{
		return scriptname;
	}
}


