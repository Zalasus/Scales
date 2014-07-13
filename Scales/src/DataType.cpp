
#include "DataType.h"

namespace Scales
{

	//public class DataType

    DataType::DataType(uint8_t id, const String &name)
    :
			typeID(id),
			typeName(name),
			objectType("","")
    {
        values.push_back(*this);
    }

    DataType::DataType(const DataType &t)
    :
    		typeID(t.typeID),
    		typeName(t.typeName),
    		objectType(t.getObjectType())
    {
    }

    DataType &DataType::operator=(const DataType &t)
    {
    	typeID = t.getTypeID();
    	typeName = t.getTypeName();
    	objectType = t.getObjectType();

    	return *this;
    }

    uint8_t DataType::getTypeID() const
    {
        return typeID;
    }

    const String &DataType::getTypeName() const
    {
        return typeName;
    }

    bool DataType::equals(const DataType &t) const
    {
    	if(typeID != OBJECT.getTypeID())
    	{
    		return (typeID == t.getTypeID());

    	}else
    	{
    		return (t.getTypeID() == OBJECT.getTypeID()) && (objectType == t.getObjectType());
    	}
    }

    bool DataType::isNumeric() const
    {
        return !(typeID & 0xFC); //Bits 2-7 have to be 0 for numeric types
    }

    bool DataType::isArray() const
    {
    	return false; //TODO: implement arrays
    }

    bool DataType::canCastImplicitlyTo(const DataType &t)
    {
    	if(isNumeric() && t.isNumeric())
		{
    		//An implicit cast(a to b) is allowed if all type bits that are set in a are also set in b
    		// -> Mask out all bits of b that are zero in a and look if the result is equal a
			return (getTypeID() & t.getTypeID()) == getTypeID();
		}

    	if(typeID == OBJECT.getTypeID() && t.getTypeID() == ABSTRACT_OBJECT.getTypeID())
    	{
    		return true; //All object types may be implicitly cast to abstract object
    	}

    	return equals(t); //No other types are implicitly castable, except they are equal
    }

    void DataType::initObjectType(const ClassId &id)
    {
    	objectType = id;
    }

    const ClassId &DataType::getObjectType() const
    {
    	return objectType;
    }

    const String DataType::toString() const
    {
    	if(typeID == DataType::OBJECT.getTypeID())
    	{

    		return objectType.toString();

    	}else if(typeID == DataType::ABSTRACT_OBJECT.getTypeID())
    	{

    		return String("abstract object");

    	}else
    	{
    		return typeName;
    	}
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
    DataType DataType::mathCast(const DataType &a, const DataType &b)
    {
    	if(a.isNumeric() && b.isNumeric())
    	{
    		return DataType::byID(a.getTypeID() | b.getTypeID()); //See above note
    	}


    	return NOTYPE;
    }

    DataType DataType::byName(const String &name)
    {
    	uint32_t size = values.size();

    	for(uint16_t i = 0; i < size; i++)
		{
			if(!values[i].getTypeName().compare(name))
			{
				return values[i];
			}
		}

		return NOTYPE;
    }

    DataType DataType::byID(uint8_t id)
    {
    	uint32_t size = values.size();

    	for(uint16_t i = 0; i < size; i++)
    	{
    		if(values[i].getTypeID() == id)
    		{
    			return values[i];
    		}
    	}

    	return NOTYPE;
    }

    vector<DataType> DataType::values;

    const DataType DataType::NOTYPE(255, String("notype"));
    const DataType DataType::INT(0, String("int"));
    const DataType DataType::LONG(1, String("long"));
    const DataType DataType::FLOAT(2, String("float"));
    const DataType DataType::DOUBLE(3, String("double"));
    const DataType DataType::STRING(4, String("string"));
    const DataType DataType::OBJECT(5, String("<I've got more rubber ducks than you!>"));
    const DataType DataType::ABSTRACT_OBJECT(6, String("object"));
}
