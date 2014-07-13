/*
 * ScalesObject.h
 *
 *  Created on: 08.07.2014
 *      Author: Niklas Weissner
 */

#ifndef SCALESOBJECT_H_
#define SCALESOBJECT_H_

#include <vector>

#include "ScalesValue.h"
#include "ScalesUtil.h"

namespace Scales
{

	class Object : public Value
	{
	public:
		Object(const Class *pClass);
		~Object();

		ValueType getValueType() const;

		Class *getClass();

		Variable *getVariable(const String &name);

	private:

		Class *myClass;

		std::vector<Variable*> variables;
	};

}



#endif /* SCALESOBJECT_H_ */
