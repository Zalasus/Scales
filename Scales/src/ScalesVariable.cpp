/*
 * ScalesVariable.cpp
 *
 *  Created on: 09.07.2014
 *      Author: Niklas Weissner
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


	VariablePrototype::VariablePrototype(const String &pName, const DataType &pType, bool pPriv, const Scope &pScope)
		: name(pName),
		  type(pType),
		  priv(pPriv),
		  scope(pScope)
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

	Scope VariablePrototype::getScope() const
	{
		return scope;
	}


	Variable::Variable(VariablePrototype *proto)
		: prototype(proto)
	{

	}

}

