/*
 * ScalesVariable.cpp
 *
 *  Created on: 09.07.2014
 *      Author: Zalasus
 */

#include "ScalesVariable.h"

namespace Scales
{

	Scope::Scope(uint32_t pNestId, uint32_t pRowId, uint32_t pUniqueId)
		: nestId(pNestId),
		  rowId(pRowId),
		  uniqueId(pUniqueId)
	{

	}

	uint32_t Scope::getNestId() const
	{
		return nestId;
	}

	uint32_t Scope::getRowId() const
	{
		return rowId;
	}

	uint32_t Scope::getUniqueId() const
	{
		return uniqueId;
	}

	bool Scope::isVisibleIn(const Scope &s) const
	{
		if(s.getNestId() == nestId)
		{
			return s.getUniqueId() == uniqueId;

		}else if(s.getNestId() > nestId)
		{
			return (s.getNestId() - nestId) == (s.getUniqueId() - uniqueId - s.getRowId());
		}

		return false;
	}

	bool Scope::isGlobal() const
	{
		return nestId == 0;
	}

	const Scope Scope::GLOBAL(0,0,0);


	VariableSketch::VariableSketch(const String &pName, const DataType &pType, bool pPriv, const Scope &pScope)
		: name(pName),
		  type(pType),
		  priv(pPriv),
		  scope(pScope)
	{

	}

	String VariableSketch::getName() const
	{
		return name;
	}

	DataType VariableSketch::getType() const
	{
		return type;
	}

	bool VariableSketch::isPrivate() const
	{
		return priv;
	}

	Scope VariableSketch::getScope() const
	{
		return scope;
	}


	Variable::Variable(const VariableSketch &pSketch, ScalesSystem *system)
		: sketch(pSketch),
		  value(null) //All variables are null by default
	{

	}

	VariableSketch Variable::getSketch() const
	{
		return sketch;
	}

	ValuePtr *Variable::getValuePtr() const
	{
		return value;
	}

	void Variable::assignByReference(ValuePtr *v)
	{
		if(value != null)
		{
			value->removeUser(this);
		}

		if(v != null)
		{
			v->addUser(this);
		}

		value = v;
	}

}

