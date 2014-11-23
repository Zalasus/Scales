/*
 * ScalesType.cpp
 *
 *  Created on: 31.10.2014
 *      Author: Zalasus
 */

#include "ScalesType.h"

namespace Scales
{

	ClassID::ClassID(const String &pNspace, const String &pClassname)
	: nspace(pNspace),
	  classname(pClassname)
	{
	}

	String ClassID::getNamespace() const
	{
		return nspace;
	}

	String ClassID::getClassname() const
	{
		return classname;
	}

	bool ClassID::operator==(const ClassID &right)
	{
		return (getNamespace() == right.getNamespace()) && (getClassname() == right.getClassname());
	}

	String ClassID::toString() const
	{
		if(nspace.empty())
		{
			return classname;

		}else
		{
			return nspace + ":" + classname;
		}
	}

	const ClassID ClassID::EMPTY = ClassID("","");



	DataType::DataType(DataType::dataTypeBase_t pBase, const Class *pTypeClass)
	: base(pBase),
	  typeClass(pTypeClass)
	{
	}

	DataType::dataTypeBase_t DataType::getBase() const
	{
		return base;
	}

	const Class *DataType::getTypeClass() const
	{
		return typeClass;
	}

	bool DataType::isNumeric() const
	{
		return base < DTB_STRING;
	}

	bool DataType::isArray() const
	{
		return false;
	}

	String DataType::toString() const
	{
		switch(base)
		{
		case DTB_INT:
			return String("int");

		case DTB_LONG:
			return String("long");

		case DTB_FLOAT:
			return String("float");

		case DTB_DOUBLE:
			return String("double");

		case DTB_STRING:
			return String("string");

		case DTB_OBJECT:
			return (typeClass == nullptr) ? "weird object indeed" : typeClass->getID().toString();

		case DTB_ABSTRACT_OBJECT:
			return String("object");

		case DTB_VOID:
			return String("void");

		default:
			return String("I've got more rubber ducks than you");
		}
	}

	bool DataType::operator==(const DataType &t) const
	{
		if(t.getBase() != getBase())
		{
			return false;
		}

		if(getBase() == DTB_OBJECT)
		{
			if(getTypeClass() != nullptr && t.getTypeClass() != nullptr)
			{
				return getTypeClass() == t.getTypeClass();

			}else
			{
				return false;
			}
		}

		return true;
	}

	bool DataType::operator==(const dataTypeBase_t tb) const
	{
		return tb == getBase();
	}

	bool DataType::canCastImplicitly(const DataType &from, const DataType &to)
	{
		if(from.isNumeric() && to.isNumeric())
		{
			//An implicit cast(a to b) is allowed if all type bits that are set in a are also set in b
			// -> Mask out all bits of b that are zero in a and look if the result is equal a
			return (from.getBase() & to.getBase()) == from.getBase();
		}

		if(from.getBase() == DTB_OBJECT && to.getBase() == DTB_ABSTRACT_OBJECT)
		{
			return true; //All object types may be implicitly cast to abstract object
		}

		return from == to; //No other types are implicitly castable, except they are equal
	}

	/*
	 * Type ID is used as a cast mask and is made up of the two bits:
	 *
	 * 		type    | fp  | big
	 * 		--------+-----+-----
	 * 		int     | 0   |  0
	 * 		long    | 0   |  1
	 * 		float   | 1   |  0
	 * 		double  | 1   |  1
	 *
	 * 		fp indicates a floating point type
	 * 		big indicates a big number type (64 bit)
	 *
	 * 	The type of any mathematical operation on these numeric types can be resolved by
	 * 	a simple logical OR operation between the two types.
	 *
	 * 	If at least one of the bits 2-7 is one, the type is not numeric and thus the above
	 * 	definition is not valid anymore. Operation types of non-numeric types are determined
	 * 	differently.
	 */
	DataType DataType::mathCast(const DataType &left, const DataType &right)
	{
		if(left.isNumeric() && right.isNumeric())
		{
			return DataType(static_cast<DataType::dataTypeBase_t>(left.getBase() | right.getBase())); //See above note
		}

		return DataType(DTB_VOID);
	}

	DataType::dataTypeBase_t DataType::byName(const String &name)
	{
		if(name == "int")
		{
			return DTB_INT;

		}else if(name == "long")
		{
			return DTB_LONG;

		}else if(name == "float")
		{
			return DTB_FLOAT;

		}else if(name == "double")
		{
			return DTB_DOUBLE;

		}else if(name == "string")
		{
			return DTB_STRING;

		}else if(name == "object")
		{
			return DTB_ABSTRACT_OBJECT;

		}else
		{
			return DTB_VOID;
		}
	}

	const DataType DataType::INT = DataType(DataType::DTB_INT);
	const DataType DataType::LONG = DataType(DataType::DTB_LONG);
	const DataType DataType::FLOAT = DataType(DataType::DTB_FLOAT);
	const DataType DataType::DOUBLE = DataType(DataType::DTB_DOUBLE);
	const DataType DataType::STRING = DataType(DataType::DTB_STRING);
	const DataType DataType::ABSTRACT_OBJECT = DataType(DataType::DTB_ABSTRACT_OBJECT);
	const DataType DataType::_VOID = DataType(DataType::DTB_VOID);

}
