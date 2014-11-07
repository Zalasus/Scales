/*
 * ScalesObject.h
 *
 *  Created on: 08.10.2014
 *      Author: Zalasus
 */

#ifndef SCALESOBJECT_H_
#define SCALESOBJECT_H_

#include "ScalesClass.h"
#include "ScalesMemory.h"
#include "ScalesUtil.h"

namespace Scales
{

	class Object
	{
	public:
		Object(const Class &pMyClass);
		~Object();

		const Class &getClass() const;

		IValue *getField(const Field &field);
		IValue *getField(const String &name);

		IValue *_getFieldByIndex(uint32_t i);

	private:

		const Class &myClass;

		IValue **fields;
		uint32_t storedFieldCount;
	};

	typedef CheckedPtr<Object> ObjectPtr;

}


#endif /* SCALESOBJECT_H_ */
