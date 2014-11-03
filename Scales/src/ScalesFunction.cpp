/*
 * ScalesFunction.cpp
 *
 *  Created on: 01.11.2014
 *      Author: Zalasus
 */

#include "ScalesFunction.h"

namespace Scales
{

	Function::Function(const String &pName, const TypeList &pParamTypes)
	 : name(pName),
	   paramTypes(pParamTypes),
	   returnType(DataType::DTB_VOID),
	   adress(0)
	{
	}

	String Function::getName() const
	{
		return name;
	}

	TypeList Function::getParamTypes() const
	{
		return paramTypes;
	}

	DataType Function::getReturnType() const
	{
		return returnType;
	}

	void Function::setReturnType(const DataType &t)
	{
		returnType = t;
	}

	progAdress_t Function::getAdress() const
	{
		return adress;
	}

	void Function::setAdress(progAdress_t a)
	{
		adress = a;
	}

	bool Function::hasAdress() const
	{
		return !isNative() && adress != 0;
	}

	uint32_t Function::getLocalCount() const
	{
		return 0; //TODO: Locals here
	}

	uint32_t Function::getStackSize() const
	{
		return 0; //TODO: Stack size here
	}

	bool Function::matches(const String &pName, const TypeList &pParamTypes) const
	{
		if((pName != name) || (pParamTypes.size() != paramTypes.size()))
		{
			return false;
		}

		for(uint32_t i = 0; i < paramTypes.size(); i++)
		{
			if(!(paramTypes[i] == pParamTypes[i])) //i'm too lazy to define the != operator right now, so this has to do
			{
				return false;
			}
		}

		return true;
	}

	bool Function::isConstructor() const
	{
		return name == "init";
	}

	String Function::getInfoString(const String &pName, const TypeList &pParamTypes)
	{
		String result = pName + "(";

		for(uint32_t i = 0; i < pParamTypes.size(); i++)
		{
			result += pParamTypes[i].toString();

			if(i < (pParamTypes.size()-1))
			{
				//But wait! There's more!
				result += ", ";
			}
		}

		return result + ")";
	}

}



