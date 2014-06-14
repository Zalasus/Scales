/*
 * Variable.cpp
 *
 *  Created on: 01.05.2014
 *      Author: Niklas Weissner
 */


#include "Variable.h"

namespace Scales
{

	//public class Value

	Value::Value(const DataType &t)
	:
			type(t)
	{
	}

	DataType Value::getType() const
	{
		return type;
	}

	const Value Value::NULL_VALUE = Value(DataType::NOTYPE);

	VariablePrototype::VariablePrototype(const String &pName, const DataType &pType, bool pPriv)
	:
			name(pName),
			type(pType),
			priv(pPriv),
			scope(0)
	{

	}

	String VariablePrototype::getName() const
	{
		return name;
	}

	DataType VariablePrototype::getType() const
	{
		return type;
	}

	bool VariablePrototype::isPrivate() const
	{
		return priv;
	}

	void VariablePrototype::setScope(uint32_t i)
	{
		scope = i;
	}

	uint32_t VariablePrototype::getScope() const
	{
		return scope;
	}
}

