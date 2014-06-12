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

	ScriptInstance::ScriptInstance(Script &s)
	:
			myClass(s),
			programCounter(0)
	{
	}

	void ScriptInstance::initialize()
	{

	}

	Value ScriptInstance::callFunction(const String &name, const vector<Value> &parameter)
	{
		vector<DataType> paramTypes;

		for(uint32_t i = 0; i < parameter.size(); i++)
		{
			DataType type = parameter[i].getType();

			paramTypes.push_back(type);
		}

		Function *func = myClass.getFunction(name, paramTypes);

		if(func != null)
		{
			if(func->getReturnType().equals(DataType::NOTYPE))
			{
				return Value::NULL_VALUE;

			}else
			{
				return astack[astack.size() - 1];
			}
		}

		throw ScalesException(ScalesException::ET_RUNTIME, "Function " + Function::createInfoString(name, paramTypes) + " was not declared in script " + myClass.getIdent().toString());
	}

	//public class Function

	Function::Function(const String &pName, const vector<DataType> &parameterTypes, const DataType &pReturnType, const AccessType &pAccessType, bool pNative, FunctionType pType, uint32_t pAdress)
	:
			functionName(pName),
			paramTypes(parameterTypes),
			returnType(pReturnType),
			accessType(pAccessType),
			native(pNative),
			type(pType),
			adress(pAdress)
	{

	}

	Function::FunctionType Function::getType() const
	{
		return type;
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
			currentLocalScope(0),
			bytecode(null),
			bytecodeLength(0)
	{
	}

	Script::Script(const Script &script)
	:
			ident(script.getIdent()),
			currentLocalScope(0),
			functions(script.functions),
			globals(script.globals),
			locals(script.locals)
	{
		//copy the scripts bytecode buffer to a new memory block

		bytecodeLength = script.getBytecodeLength();

		bytecode = new uint8_t[bytecodeLength];

		for(uint32_t i = 0; i < bytecodeLength; i++)
		{
			bytecode[i] = script.getBytecode()[i];
		}
	}

	Script::~Script()
	{
		delete[] bytecode;
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

	void Script::declareGlobal(VariablePrototype &v)
	{
		globals.push_back(v);
	}

	void Script::declareLocal(VariablePrototype &v)
	{
		v.setScope(currentLocalScope);

		locals.push_back(v);
	}

	void Script::leaveLocalScope()
	{

		for(vector<VariablePrototype>::iterator it = locals.begin(); it != locals.end();)
		{
		    if(it->getScope() >= currentLocalScope)
		    {
		        it = locals.erase(it);
		    }else
		    {
		        it++;
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

	VariablePrototype *Script::getGlobal(const String &name)
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

	VariablePrototype *Script::getLocal(const String &name)
	{
		for(uint32_t i = 0; i < locals.size(); i++)
		{

			if(locals[i].getName().equals(name) && locals[i].getScope() <= currentLocalScope)
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

	void Script::setBytecode(uint8_t *data, uint32_t size)
	{
		bytecode = data;
		bytecodeLength = size;
	}

	uint8_t *Script::getBytecode() const
	{
		return bytecode;
	}

	uint32_t Script::getBytecodeLength() const
	{
		return bytecodeLength;
	}

}


