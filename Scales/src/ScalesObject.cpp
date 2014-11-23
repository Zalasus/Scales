/*
 * ScalesObject.cpp
 *
 *  Created on: 07.11.2014
 *      Author: Zalasus
 */

#include "ScalesObject.h"
#include "ScalesUtil.h"
#include "ScalesException.h"

#include <iostream>

namespace Scales
{

	Object::Object(const Class *pClass)
	: myClass(pClass),
	  fields(nullptr),
	  storedFieldCount(0)
	{
		if(myClass == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to create object of null class");
		}

		storedFieldCount = pClass->getFieldCount(); //don't risk a leak. we need to securely delete all elements we create. the class object may still be changed from somewhere else
		fields = SCALES_NEW IValue*[storedFieldCount];

		for(uint32_t i = 0; i < storedFieldCount; i++)
		{
			const Field *f = myClass->getFieldWithID(i);

			if(f == nullptr)
			{
				SCALES_EXCEPT(Exception::ET_RUNTIME, String("Error during class instantiation. Missing definition of field index ") + i);
			}

			if((f->getType().getBase() == DataType::DTB_OBJECT) || (f->getType().getBase() == DataType::DTB_ABSTRACT_OBJECT))
			{
				//objects are inititalized to null. They must be created and registered via the root object. everything else can be initialized here

				fields[i] = nullptr;

			}else
			{
				/*const IValueFactory *factory = IValueFactory::getFactoryForType(f->getType());

				if(factory != nullptr)
				{*/
					fields[i] = IValue::getNewPrimitiveFromType(f->getType());

				/*}else
				{
					SCALES_EXCEPT(Exception::ET_RUNTIME, String("Error during class instantiation. Missing factory for type ") + f->getType().toString());
				}*/
			}
		}
	}

	Object::~Object()
	{
		for(uint32_t i = 0; i < storedFieldCount; i++)
		{
			SCALES_DELETE fields[i];
		}

		SCALES_DELETE[] fields;
	}

}




