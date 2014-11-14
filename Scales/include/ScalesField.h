/*
 * ScalesField.h
 *
 *  Created on: 04.10.2014
 *      Author: Zalasus
 */

#ifndef SCALESFIELD_H_
#define SCALESFIELD_H_

#include "ScalesString.h"
#include "ScalesAccess.h"
#include "ScalesType.h"

namespace Scales
{

	class Object;
	class IValue;

	class Field : public AccessElement
	{

	public:

		Field(const String &pName, const DataType &pType, uint32_t pIndex);

		String getName() const;
		DataType getType() const;

		uint32_t getIndex() const;

		void assign(Object *obj, IValue *val) const;

	private:

		String name;
		DataType type;

		uint32_t index;

	};

}


#endif /* SCALESFIELD_H_ */
