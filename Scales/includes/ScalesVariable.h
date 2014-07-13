/*
 * ScalesVariable.h
 *
 *  Created on: 08.07.2014
 *      Author: Niklas Weissner
 */

#ifndef SCALESVARIABLE_H_
#define SCALESVARIABLE_H_

#include "ScalesUtil.h"
#include "ScalesType.h"

namespace Scales
{

	class Scope
	{
	public:
		Scope(uint32_t pNestId, uint32_t pRowId, uint32_t pUniqueId);

		uint32_t getNestId() const;
		uint32_t getRowId() const;
		uint32_t getUniqueId() const;

		bool isVisibleIn(const Scope &s) const;
		bool isGlobal() const;

		static const Scope GLOBAL;

	private:

		uint32_t nestId;
		uint32_t rowId;
		uint32_t uniqueId;
	};

	class VariablePrototype
	{
	public:
		VariablePrototype(const String &name, const DataType &type, bool priv, const Scope &pScope);

		String getName() const;
		DataType getType() const;
		bool isPrivate() const;
		Scope getScope() const;

	private:

		String name;
		DataType type;
		bool priv;
		Scope scope;
	};


	class Variable
	{
	public:
		Variable(VariablePrototype *proto);

		const VariablePrototype *getPrototype() const;

	private:

		VariablePrototype *prototype;
	};

}


#endif /* SCALESVARIABLE_H_ */
