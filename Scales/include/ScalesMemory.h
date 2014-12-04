/*
 * ScalesMemory.h
 *
 *  Created on: 13.10.2014
 *      Author: Zalasus
 */

#ifndef SCALESMEMORY_H_
#define SCALESMEMORY_H_

#include <vector>

#include "ScalesUtil.h"
#include "ScalesType.h"

#define SCALES_BIND_TYPE(cType, scalesType) \
	template <>\
	DataType ValuePrimitive<cType>::getDataType()\
	{\
		return scalesType;\
	}\
	\
	template class ValuePrimitive<cType>;

namespace Scales
{

	class IValue //interface for polymorphic memory elements
	{
	public:
		enum value_type_t
		{
			VT_NULL,
			VT_REFERENCE,
			VT_PRIMITVE,
			VT_OBJECT
		};

		virtual ~IValue();

		virtual IValue *copy() = 0;
		virtual value_type_t getValueType() = 0;
		virtual DataType getDataType() = 0;

		static IValue *getNewPrimitiveFromType(const DataType &t);
	};

	template <typename T>
	class ValuePrimitive : public IValue
	{
	public:

		ValuePrimitive(T pData);

		/**
		 * Allocates memory and creates a new value with same type and data and returns it.
		 */
		IValue *copy();
		value_type_t getValueType();
		DataType getDataType();

		T &getData();

	private:

		T data;
	};

	class ValueRef : public IValue
	{
	public:

		ValueRef(IValue *&pRef);

		/**
		 * Calls the copy method of the referenced value, thus dereferencing it.
		 */
		IValue *copy();
		value_type_t getValueType();
		DataType getDataType();

		IValue *&getReferencedThingy();

	private:

		IValue *&ref;
	};

	class Object;

	class ValueObject : public IValue
	{
	public:

		/**
		 * Creates an object value of the given object with custom class mask. If the mask is set to
		 * null, the value is an abstract object. The object pointer must not be null.
		 */
		ValueObject(Object *pObj);

		/**
		 * Creates an object value of the given object with custom class mask. If the mask is set to
		 * null, the value is an abstract object. The object pointer must not be null.
		 */
		ValueObject(Object *pObj, const Class *pMask);

		/**
		 * Creates a new pointer pointing to the same object with the same mask.
		 */
		IValue *copy();
		/**
		 * Returns the mask type of this object, or the real type if object is unmasked.
		 */
		DataType getDataType();
		value_type_t getValueType();

		/**
		 * Returns the real type of the object(the deepest level of inheritance).
		 */
		DataType getAbsoluteDataType();

		Object *getObject();

	private:

		Object * const obj;
		const Class *mask;
	};

	class ValueNull : public IValue
	{
	public:

		ValueNull();

		IValue *copy();
		DataType getDataType();
		value_type_t getValueType();
	};

	typedef int32_t primitive_int_t;
	typedef int64_t primitive_long_t;
	typedef float   primitive_float_t;
	typedef double  primitive_double_t;
	typedef String  primitive_string_t;

}


#endif /* SCALESMEMORY_H_ */
