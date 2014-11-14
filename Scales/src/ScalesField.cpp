/*
 * ScalesField.cpp
 *
 *  Created on: 01.11.2014
 *      Author: Zalasus
 */

#include "ScalesField.h"

#include "ScalesObject.h"
#include "ScalesMemory.h"
#include "ScalesUtil.h"
#include "ScalesException.h"

namespace Scales
{

	Field::Field(const String &pName, const DataType &pType, uint32_t pIndex)
	 : name(pName),
	   type(pType),
	   index(pIndex)
	{
	}

	String Field::getName() const
	{
		return name;
	}

	DataType Field::getType() const
	{
		return type;
	}

	uint32_t Field::getIndex() const
	{
		return index;
	}

	void Field::assign(Object *obj, IValue *val) const
	{
		//FIXME: Very unsafe. We don't check if this field really is a member of the given object etc.

		if(obj == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to assign to field in null object");
		}

		IValue *oldVal = obj->fields[index];

		if(oldVal != nullptr)
		{
			SCALES_DELETE oldVal;
		}

		if(val == nullptr)
		{
			if(type.getBase() != DataType::DTB_OBJECT)
			{
				SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to assign null to non-object type");
			}

			obj->fields[index] = val;

		}else
		{
			if(!(val->getType() == type.getBase())) //still too lazy to implement the != operator
			{
				SCALES_EXCEPT(Exception::ET_RUNTIME, "Type mismatch in assignment");
			}
		}

		obj->fields[index] = val;
	}

}

