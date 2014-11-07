/*
 * ScalesClass.h
 *
 *  Created on: 03.10.2014
 *      Author: Zalasus
 */

#include <cstring>

#include "ScalesString.h"
#include "ScalesUtil.h"
#include "ScalesClass.h"

namespace Scales
{

	Class::Class(const ClassID &pClassID, const Class *pSuper)
	: classID(pClassID),
	  super(pSuper),
	  program(nullptr),
	  programSize(0)
	{
	}

	Class::~Class()
	{
		for(uint32_t i = 0; i < fields.size(); i++)
		{
			SCALES_DELETE fields[i];
		}

		for(uint32_t i = 0; i < functions.size(); i++)
		{
			SCALES_DELETE functions[i];
		}

		SCALES_DELETE[] program;
	}

	ClassID Class::getID() const
	{
		return classID;
	}

	void Class::setNativeTarget(const String &target)
	{
		nativeTarget = target;
	}

	String Class::getNativeTarget() const
	{
		return nativeTarget;
	}

	const Class *Class::getSuperclass() const
	{
		return super;
	}

	Function *Class::createFunction(const String &functionName, const TypeList &paramTypes)
	{
		if(getFunction(functionName, paramTypes) != nullptr)
		{
			return nullptr; //double definition
		}

		Function *f = SCALES_NEW Function(functionName, paramTypes);

		functions.push_back(f);

		return f;
	}

	const std::vector<const Function*> Class::listFunctions() const
	{
		//TODO: Check if there is better way of making the vector elements const than shoving them into another vector
		std::vector<const Function*> newList;

		for(auto iter = functions.begin(); iter != functions.end(); iter++)
		{
			newList.push_back(*iter);
		}

		return newList;
	}

	const std::vector<const Function*> Class::listFunctionsByName(const String &pName) const
	{
		//TODO: Check if there is better way of making the vector elements const than shoving them into another vector
		std::vector<const Function*> newList;

		for(auto iter = functions.begin(); iter != functions.end(); iter++)
		{
			if((*iter)->getName() == pName)
			{
				newList.push_back(*iter);
			}
		}

		return newList;
	}

	void Class::removeFunction(const Function *func)
	{
		std::vector<Function*>::iterator iter = functions.begin();
		while(iter != functions.end())
		{
			if(*iter == func)
			{
				iter = functions.erase(iter);

			}else
			{
			   iter++;
			}
		}
	}

	const Function *Class::getFunction(const String &functionName, const TypeList &paramTypes) const
	{
		for(uint32_t i = 0; i < functions.size(); i++)
		{
			if(functions[i]->matches(functionName, paramTypes))
			{
				return functions[i];
			}
		}

		return nullptr;
	}

	const Function *Class::getJoinedFunction(const String &functionName, const TypeList &paramTypes) const
	{
		const Function *f = getFunction(functionName, paramTypes);

		if(f != nullptr)
		{
			return f;

		}else if(super != nullptr)
		{
			return super->getJoinedFunction(functionName, paramTypes);

		}else
		{
			return nullptr;
		}
	}


	Field *Class::createField(const String &pFieldName, const DataType &pType)
	{
		if(getField(pFieldName) != nullptr) //Check if field already exists
		{
			return nullptr;
		}

		uint32_t fIndex = getJoinedFieldCount(); //new field is stored at the end of memory
		Field *f = SCALES_NEW Field(pFieldName, pType, fIndex);

		fields.push_back(f);

		return f;
	}

	const Field *Class::getField(const String &pFieldName) const
	{
		for(uint32_t i = 0; i < fields.size(); i++)
		{
			if(fields[i]->getName() == pFieldName)
			{
				return fields[i];
			}
		}

		return nullptr;
	}

	const Field *Class::getFieldWithID(uint32_t i) const
	{
		for(auto it = fields.begin(); it != fields.end(); i++)
		{
			if((*it)->getIndex() == i)
			{
				return *it;
			}
		}

		return nullptr;
	}

	const Field *Class::getJoinedField(const String &fieldName) const
	{
		const Field *f = getField(fieldName);

		if(f != nullptr)
		{
			return f;

		}else if(super != nullptr)
		{
			return super->getJoinedField(fieldName);

		}else
		{
			return nullptr;
		}
	}

	uint32_t Class::getFieldCount() const
	{
		return fields.size();
	}

	uint32_t Class::getJoinedFieldCount() const
	{
		if(super == nullptr)
		{
			return getFieldCount();
		}else
		{
			return getFieldCount() + (super->getJoinedFieldCount());
		}
	}

	void Class::copyProgramArray(const progUnit_t *data, progAdress_t size)
	{
		if(program != nullptr)
		{
			SCALES_DELETE[] program;
		}

		program = SCALES_NEW progUnit_t[size];
		programSize = size;

		memcpy(program, data, size);
	}

	progAdress_t Class::getProgramSize() const
	{
		return programSize;
	}

	progAdress_t Class::getJoinedProgramSize() const
	{
		if(super == nullptr)
		{
			return getProgramSize();

		}else
		{
			return getProgramSize() + super->getJoinedProgramSize();
		}
	}

	const progUnit_t *Class::getJoinedProgramArray() const
	{
		//This is a huge pile of shit. This function has to be const, as during runtime, a class is finalized. But we need to create the joined program if it has not been created yet, so we need to access members which is impossible.
		//solution: TODO: find a way of joining the bytecode without having to copy it (or do that during construction. QAD solution, but not worse than what we tried here)
		if(program == nullptr)
		{
			return nullptr; //there is no bytecode -> instantiation of this class is impossible
		}

		if(super == nullptr)
		{
			return program; //since there is no superclass, we don't need to join any bytecode arrays
		}

		// assume there is a superclass and this class has some bytecode

		/*if(joinedProgram == nullptr) //joined program hasn't been created yet -> create it
		{
			progAdress_t progSize = getProgramSize();
			progAdress_t superProgSize = super->getJoinedProgramSize();
			progAdress_t joinedProgSize = progSize + superProgSize; //this is how much memory we need for the joined program in this class (same as this->getJoinedProgramSize() )

			const progUnit_t *superProg = super->getJoinedProgramArray(); //TODO: Heavy memory usage and data redundance and stuff. See PS-444a.

			//joinedProgram = SCALES_NEW progUnit_t[joinedProgSize];

			memcpy(joinedProgram, superProg, superProgSize); //TODO: Bad memcpy and shit
			memcpy(joinedProgram + superProgSize, program, progSize);
		}*/

		//return joinedProgram;
		return nullptr;
	}


}


