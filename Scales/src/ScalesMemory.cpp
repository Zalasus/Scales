/*
 * ScalesMemory.cpp
 *
 *  Created on: 07.11.2014
 *      Author: Zalasus
 */

#include "ScalesMemory.h"
#include "ScalesUtil.h"
#include "ScalesException.h"
#include "ScalesObject.h"

namespace Scales
{

	IValue::~IValue()
	{
	}

	template <typename T>
	ValuePrimitive<T>::ValuePrimitive(T pData)
	: data(pData)
	{
	}

	template <typename T>
	T &ValuePrimitive<T>::getData()
	{
		return data;
	}

	template <typename T>
	IValue::value_type_t ValuePrimitive<T>::getValueType()
	{
		return VT_PRIMITVE;
	}

	template <typename T>
	IValue *ValuePrimitive<T>::copy()
	{
		return SCALES_NEW ValuePrimitive<T>(data);
	}


	ValueRef::ValueRef(IValue **pRef)
	: ref(pRef)
	{
		if(pRef == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to create nullreference");
		}

		if((*pRef) == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to reference nullvalue");
		}

		if((*pRef)->getValueType() == VT_REFERENCE)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to cascade references");
		}
	}

	IValue *ValueRef::copy()
	{
		if(*ref == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to access referenced nullvalue");
		}

		return (*ref)->copy();
	}

	DataType ValueRef::getDataType()
	{
		if(*ref == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to access referenced nullvalue");
		}

		return (*ref)->getDataType();
	}

	IValue::value_type_t ValueRef::getValueType()
	{
		return VT_REFERENCE;
	}

	IValue **ValueRef::getReference()
	{
		if(*ref == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to access referenced nullvalue");
		}

		return ref;
	}

	IValue *IValue::getNewPrimitiveFromType(const DataType &t)
	{
		switch(t.getBase())
		{
		case DataType::DTB_INT:
			return SCALES_NEW ValuePrimitive<int32_t>(0);

		case DataType::DTB_LONG:
			return SCALES_NEW ValuePrimitive<int64_t>(0);

		case DataType::DTB_FLOAT:
			return SCALES_NEW ValuePrimitive<float>(0);

		case DataType::DTB_DOUBLE:
			return SCALES_NEW ValuePrimitive<double>(0);

		case DataType::DTB_STRING:
			return SCALES_NEW ValuePrimitive<String>("");

		default:
			return nullptr; //TODO: better return new ValueNull here

		}

		return nullptr; //same here
	}



	ValueObject::ValueObject(Object *pObj)
	: ValueObject(pObj, pObj->getClass())
	{
	}

	ValueObject::ValueObject(Object *pObj, const Class *pMask)
	 : obj(pObj),
	   mask(pMask)
	{
		if(obj == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to create null-object");
		}
	}

	IValue *ValueObject::copy()
	{
		return SCALES_NEW ValueObject(obj, mask);
	}

	IValue::value_type_t ValueObject::getValueType()
	{
		return VT_OBJECT;
	}

	DataType ValueObject::getDataType()
	{
		return DataType(mask == nullptr ? DataType::DTB_ABSTRACT_OBJECT : DataType::DTB_OBJECT, mask);
	}

	DataType ValueObject::getAbsoluteDataType()
	{
		return DataType(DataType::DTB_OBJECT, obj->getClass());
	}

	Object *ValueObject::getObject()
	{
		return obj;
	}


	ValueNull::ValueNull()
	{
	}

	IValue *ValueNull::copy()
	{
		return SCALES_NEW ValueNull();
	}

	DataType ValueNull::getDataType()
	{
		return DataType::_VOID; //TODO: change to dedicated null type
	}

	IValue::value_type_t ValueNull::getValueType()
	{
		return VT_NULL;
	}

	//provide specializations for every primitive Scales data type

	SCALES_BIND_TYPE(primitive_int_t, DataType::INT)
	SCALES_BIND_TYPE(primitive_long_t, DataType::LONG)
	SCALES_BIND_TYPE(primitive_float_t, DataType::FLOAT)
	SCALES_BIND_TYPE(primitive_double_t, DataType::DOUBLE)
	SCALES_BIND_TYPE(primitive_string_t, DataType::STRING)

}

