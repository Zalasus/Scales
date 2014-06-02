/*
 * Script.cpp
 *
 *  Created on: 29.04.2014
 *      Author: Niklas Weissner
 */

#include "Script.h"

namespace Scales
{

	//public class ScriptInstance

	ScriptInstance::ScriptInstance(const Script &s)
	:
			myClass(s),
			programCounter(0)
	{
	}


	//public class Function

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

	const DataType &Function::getReturnType() const
	{
		return returnType;
	}

	const AccessType &Function::getAccessType() const
	{
		return accessType;
	}

	const String &Function::getName() const
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

	const String Function::createInfoString(const String &name, vector<DataType> &paramTypes)
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


	//public class Script

	Script::Script(const ScriptIdent &scriptident)
	:
			ident(scriptident),
			currentLocalScope(0)
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

	void Script::declareGlobal(Variable &v)
	{
		globals.push_back(v);
	}

	void Script::declareLocal(Variable &v)
	{
		v.setScope(currentLocalScope);

		locals.push_back(v);
	}

	//TODO: Maybe rename everything here so one can easily recognize this stuff is only for prototyping
	void Script::leaveLocalScope()
	{
		for(uint32_t i = 0; i < locals.size(); i++)
		{
			if(locals[i].getScope() >= currentLocalScope)
			{
				locals.erase(locals.begin() + i);
			}
		}

		if(currentLocalScope > 0)
		{
			currentLocalScope--;
		}
	}

	void Script::enterLocalScope()
	{
		currentLocalScope++;
	}

	void Script::destroyAllLocals()
	{
		locals.clear();
		currentLocalScope = 0;
	}


	//TODO: Rework this. Variables are not part the script class anymore. they are stored in ScriptInstances, Scripts just contain prototypes
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
		for(uint32_t i = 0; i < locals.size(); i++)
		{

			if(locals[i].getName().equals(name))
			{
				return &locals[i];
			}
		}

		return null;
	}


	const ScriptIdent &Script::getIdent() const
	{
		return ident;
	}

}


