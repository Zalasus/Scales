/*
 * ScalesObject.h
 *
 *  Created on: 08.07.2014
 *      Author: Zalasus
 */

namespace Scales
{
	class Object;
}

#ifndef SCALESOBJECT_H_
#define SCALESOBJECT_H_

#include "ScalesClass.h"
#include "ScalesValue.h"
#include "ScalesUtil.h"
#include "ScalesVariable.h"

#include <vector>

namespace Scales
{
	/**
	 * Container for pointers to virtual memory and their associated variable sketches
	 */
	class VarThingy
	{
	public:
		VarThingy(const VariableSketch &pSketch, Value *pValue);

		VariableSketch getSketch();
		Value *getValuePtr();

	private:

		VariableSketch sketch;
		Value *value;
	};


	class Object : public Value
	{
	public:
		Object(Class *pClass);
		~Object();

		ValueType getValueType() const;

		Class *getClass();

	private:

		Class *myClass;

		std::vector<VarThingy> variables;
	};

}



#endif /* SCALESOBJECT_H_ */
