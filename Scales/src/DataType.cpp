
#include "DataType.h"


namespace Scales
{

    DataType::DataType(uint8_t id, String name)
    {
        typeID = id;
        typeName = name;
    }

    DataType::DataType() : DataType(0, String("null"))
    {

    }

    uint8_t DataType::getTypeID()
    {
        return typeID;
    }

    String DataType::getTypeName()
    {
        return typeName;
    }

    bool DataType::equals(DataType t)
    {
        return (getTypeID() == t.getTypeID());
    }

    bool DataType::isNumeric()
    {
        return (
                equals(DataType::INT) ||
                equals(DataType::FLOAT) ||
                equals(DataType::LONG) ||
                equals(DataType::DOUBLE)
               );
    }

    DataType DataType::mathCast(DataType a, DataType b)
    {
    	//TODO: Do usefull stuff here

    	return NULL_TYPE;
    }

    DataType DataType::byName(String name)
    {
    	//TODO: Return usefull stuff here

    	return NULL_TYPE;
    }

    const DataType DataType::NULL_TYPE = DataType(0,String("null"));
    const DataType DataType::INT = DataType(1,String("int"));
    const DataType DataType::LONG = DataType(2,String("long"));
    const DataType DataType::FLOAT = DataType(3,String("float"));
    const DataType DataType::DOUBLE = DataType(4,String("double"));
    const DataType DataType::STRING = DataType(5,String("string"));
    const DataType DataType::OBJECT = DataType(6,String("object"));

}
