/*
 * ScalesVariable.h
 *
 *  Created on: 08.07.2014
 *      Author: Zalasus
 */

namespace Scales
{
	class Scope;
	class VariableSketch;
	class Variable;
}

#ifndef SCALESVARIABLE_H_
#define SCALESVARIABLE_H_

#include "ScalesUtil.h"
#include "ScalesType.h"
#include "ScalesSystem.h"
#include "ScalesValue.h"

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

	class VariableSketch
	{
	public:
		VariableSketch(const String &name, const DataType &type, bool priv, const Scope &pScope);

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

	/**
	 * Just a wrapper. Deallocation is up to the classes user or the GC. Class is copyable, the pointed data is NOT copied.
	 */
	class Variable
	{
	public:
		Variable(const VariableSketch &proto, ScalesSystem *system);
		Variable(const Variable &v); //Better override it. I don't trust the compiler

		VariableSketch getSketch() const;

		ValuePtr *getValuePtr() const;

		void leavesScope();
		void assignByReference(ValuePtr *v);

	private:

		VariableSketch sketch;

		ValuePtr *value;
	};

}


#endif /* SCALESVARIABLE_H_ */
