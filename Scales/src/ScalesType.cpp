/*
 * ScalesType.cpp
 *
 *  Created on: 22.07.2014
 *      Author: Zalasus
 */

#include "ScalesType.h"

namespace Scales
{

	bool DataType::operator==(const DataType &t) const
	{
		return base == t.getTypeBase() && classId == t.getClassId();
	}

	bool DataType::operator==(const DataTypeBase tb) const
	{
		return base == tb;
	}

	String DataType::toString() const
	{
		switch(base)
		{
		case DTB_INT:
			return "int";

		case DTB_LONG:
			return "long";

		case DTB_FLOAT:
			return "float";

		case DTB_DOUBLE:
			return "double";

		case DTB_OBJECT:
			return classId.toString();

		case DTB_ABSTRACT_OBJECT:
			return "object";

		default:
			return "unknown";
		}
	}

	bool DataType::isNumeric() const
	{
		return !(base & 0x03);
	}

	bool DataType::isArray() const
	{
		return false; //No arrays allowed yet TODO: implement arrays here
	}


	bool DataType::canCastImplicitly(const DataType &from, const DataType &to)
	{
		if(from.isNumeric() && to.isNumeric())
		{
			//An implicit cast(a to b) is allowed if all type bits that are set in a are also set in b
			// -> Mask out all bits of b that are zero in a and look if the result is equal a
			return (from.getTypeBase() & to.getTypeBase()) == from.getTypeBase();
		}

		if(from.getTypeBase() == DTB_OBJECT && to.getTypeBase() == DTB_ABSTRACT_OBJECT)
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
			return DataType((DataType::DataTypeBase)(left.getTypeBase() | right.getTypeBase())); //See above note
		}

		return DataType(DTB_NOTYPE);
	}

}


