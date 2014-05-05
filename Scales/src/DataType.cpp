
#include "DataType.h"

namespace Scales
{

    DataType::DataType(uint8_t id, const String &name)
    {
        typeID = id;
        typeName = name;

        values.push_back(*this);
    }

    DataType::DataType(const DataType &t)
    {
    	typeID = t.typeID;
    	typeName = t.typeName;
    	specifier = t.specifier;
    }

    DataType &DataType::operator=(const DataType &t)
    {
    	typeID = t.getTypeID();
    	typeName = t.getTypeName();
    	specifier = t.getSpecifier();

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

    bool DataType::equals(const DataType &t) const
    {
        return (typeID == t.getTypeID()) && (specifier.equals(t.getSpecifier()) || specifier.isEmpty() || t.getSpecifier().isEmpty());
    }

    bool DataType::isNumeric() const
    {
        return !(typeID & 0xFC); //Bits 2-7 have to be 0 for numeric types
    }

    bool DataType::canCastImplicitlyTo(const DataType &t)
    {
    	if(isNumeric() && t.isNumeric())
		{
    		//An implicit cast(a to b) is allowed if all type bits that are set in a are also set in b
    		// -> Mask out all bits of b that are zero in a and look if the result is equal a
			return (getTypeID() & t.getTypeID()) == getTypeID();
		}

    	return equals(t); //No other types are implicitly castable, except they are equal
    }

    void DataType::initSpecifier(const String &s)
    {
    	specifier = s;
    }

    String DataType::getSpecifier() const
    {
    	return specifier;
    }

    String DataType::toString() const
    {
    	if(equals(DataType::OBJECT) && !specifier.isEmpty())
    	{
    		return typeName + "<" + specifier + ">";
    	}

    	return typeName;
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



    //public class AccessType

    AccessType::AccessType(uint8_t id, const String &name)
	{
	   typeID = id;
	   typeName = name;

	   values.push_back(*this);
	}

    AccessType::AccessType(const AccessType &t)
	{
		typeID = t.typeID;
		typeName = t.typeName;
	}

    AccessType &AccessType::operator=(const AccessType &t)
	{
		typeID = t.getTypeID();
		typeName = t.getTypeName();

		return *this;
	}

	uint8_t AccessType::getTypeID() const
	{
	   return typeID;
	}

	String AccessType::getTypeName() const
	{
	   return typeName;
	}

	bool AccessType::equals(const AccessType &t) const
	{
	   return (getTypeID() == t.getTypeID());
	}

	AccessType AccessType::byName(const String &name)
	{
		uint32_t size = values.size();

		for(uint16_t i = 0; i < size; i++)
		{
			if(values[i].getTypeName().equals(name))
			{
				return values[i];
			}
		}

		return PRIVATE;
	}

	AccessType AccessType::byID(uint8_t id)
	{
		uint32_t size = values.size();

		for(uint16_t i = 0; i < size; i++)
		{
			if(values[i].getTypeID() == id)
			{
				return values[i];
			}
		}

		return PRIVATE;
	}

	vector<AccessType> AccessType::values;

	const AccessType AccessType::PRIVATE(0, String("private"));
	const AccessType AccessType::PUBLIC(1, String("public"));
	const AccessType AccessType::UNIVERSAL(2, String("universal"));
}
