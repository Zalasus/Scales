/*
 * ScalesMemory.cpp
 *
 *  Created on: 07.11.2014
 *      Author: Zalasus
 */

#include "ScalesMemory.h"
#include "ScalesUtil.h"
#include "ScalesException.h"

namespace Scales
{

	IValue *IValue::getInstanceFromType(const DataType &t)
	{
		switch(t.getBase())
		{
		case DataType::DTB_INT:
			return SCALES_NEW IValueImpl<int32_t>(0);

		case DataType::DTB_LONG:
			return SCALES_NEW IValueImpl<int64_t>(0);

		case DataType::DTB_FLOAT:
			return SCALES_NEW IValueImpl<float>(0);

		case DataType::DTB_DOUBLE:
			return SCALES_NEW IValueImpl<double>(0);

		case DataType::DTB_STRING:
			return SCALES_NEW IValueImpl<String>("");

		default:
			return nullptr;
		}

		return nullptr;
	}



	IValue::~IValue()
	{

	}

	template <typename T>
	IValueImpl<T>::IValueImpl(T pData)
	: data(pData)
	{
	}

	template <typename T>
	T &IValueImpl<T>::getData()
	{
		return data;
	}

	template <typename T>
	IValue *IValueImpl<T>::copy()
	{
		return SCALES_NEW IValueImpl<T>(data);
	}

	/*IValueFactory::IValueFactory(const DataType &pType, IValue* (*pCreatorFunction)(void))
	: type(pType),
	  creatorFunction(pCreatorFunction)
	{
		factories.push_back(this);
	}

	DataType IValueFactory::getType() const
	{
		return type;
	}

	IValue *IValueFactory::create() const
	{
		return creatorFunction();
	}

	const IValueFactory *IValueFactory::getFactoryForType(const DataType &pType)
	{
		for(auto iter = factories.begin(); iter != factories.end(); iter++)
		{
			if((*iter)->getType() == pType)
			{
				return *iter;
			}
		}

		return nullptr;
	}

	std::vector<IValueFactory*> IValueFactory::factories;*/


	//provide specializations for every default Scales data type

	SCALES_BIND_TYPE(int32_t, DataType::INT)
	SCALES_BIND_TYPE(int64_t, DataType::LONG)
	SCALES_BIND_TYPE(float, DataType::FLOAT)
	SCALES_BIND_TYPE(double, DataType::DOUBLE)
	SCALES_BIND_TYPE(String, DataType::STRING)

	//provide special specialization for the object data type

	template <>
	DataType IValueImpl<Object*>::getType()
	{
		if(data == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Access to non-null IValue pointing to null object. Most likely a bug.");
		}

		return DataType(DataType::DTB_OBJECT, data->getClass().getID());
	}


}

