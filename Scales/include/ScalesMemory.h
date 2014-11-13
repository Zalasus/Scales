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

//TODO: move dynamic type binding system into different branch

#define SCALES_BIND_TYPE(cType, scalesType) \
	template <>\
	DataType IValueImpl<cType>::getType()\
	{\
		return scalesType;\
	}\
	\
	/*static IValue *type_create_ ## cType ## _IValue()\
	{\
		return SCALES_NEW IValueImpl<cType>();\
	}\
	\
	static IValueFactory type_ ## cType ## _factory = IValueFactory(scalesType, type_create_ ##cType## _IValue);*/

namespace Scales
{

	class IValue //interface for polymorphic memory elements
	{
	public:
		virtual ~IValue();

		virtual IValue *copy() = 0;

		virtual DataType getType() = 0;

		static IValue *getInstanceFromType(const DataType &t); //This replaces the not working factory based system
	};

	template <typename T>
	class IValueImpl : public IValue
	{
	public:

		IValueImpl(T pData);

		IValue *copy();

		DataType getType();

		T &getData();

	private:

		T data;
	};


	/*class IValueFactory
	{
	public:
		IValueFactory(const DataType &pType, IValue* (*pCreatorFunction)());

		DataType getType() const;
		IValue *create() const;


		static const IValueFactory *getFactoryForType(const DataType &pType);

	private:

		DataType type;
		IValue* (*creatorFunction)(void);

		static std::vector<IValueFactory*> factories;
	};*/
}


#endif /* SCALESMEMORY_H_ */
