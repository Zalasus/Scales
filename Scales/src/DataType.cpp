
#include "DataType.h"

#include <iostream>

namespace Scales
{

    DataType::DataType(uint8_t id, String name)
    {
        typeID = id;
        typeName = name;

        values.push_back(*this);
    }

    DataType::DataType(const DataType &t)
    {
    	typeID = t.typeID;
    	typeName = t.typeName;
    }

    DataType &DataType::operator=(const DataType &t)
    {
    	typeID = t.getTypeID();
    	typeName = t.getTypeName();

    	return *this;
    }

    uint8_t DataType::getTypeID() const
    {
        return typeID;
    }

    String DataType::getTypeName() const
    {
        return typeName;
    }

    bool DataType::equals(DataType t) const
    {
        return (getTypeID() == t.getTypeID());
    }

    bool DataType::isNumeric() const
    {
        return !(typeID & 0xFC); //Bits 2-7 have to be 0 for numeric types
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
    DataType DataType::mathCast(DataType a, DataType b)
    {
    	if(a.isNumeric() && b.isNumeric())
    	{
    		return DataType::byID(a.getTypeID() | b.getTypeID()); //See above note
    	}


    	return NOTYPE;
    }

    DataType DataType::byName(String name)
    {
    	uint32_t size = values.size();

    	for(uint16_t i = 0; i < size; i++)
		{
			if(values[i].getTypeName().equals(name))
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
    const DataType DataType::STRING(5, String("string"));
    const DataType DataType::OBJECT(6, String("object"));

}
