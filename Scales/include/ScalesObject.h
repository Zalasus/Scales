/*
 * ScalesObject.h
 *
 *  Created on: 08.10.2014
 *      Author: Zalasus
 */

#ifndef SCALESOBJECT_H_
#define SCALESOBJECT_H_

#include "ScalesMemory.h"

namespace Scales
{

	class Object : public Managed
	{
	public:
		Object(const Class *pMyClass);
		~Object();

		const Class *getClass() const;

		Value *getField(const Field *field);

		Value *_getFieldByIndex(uint32_t i);

	private:

		const Class *myClass;

		Value **fields;

	};

}


#endif /* SCALESOBJECT_H_ */
