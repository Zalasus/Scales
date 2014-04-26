

#ifndef DATATYPE_H_
#define DATATYPE_H_

#include "Nein.h"

namespace Scales
{

    class DataType
    {
    public:

    	static const DataType NULL_TYPE;
        static const DataType INT;
        static const DataType LONG;
        static const DataType FLOAT;
        static const DataType DOUBLE;
        static const DataType STRING;
        static const DataType OBJECT;

        static DataType mathCast(DataType a, DataType b);
        static DataType byName(String s);

        DataType(uint8_t id, String name);
        DataType();

        uint8_t getTypeID();
        String getTypeName();

        bool isNumeric();
        bool equals(DataType t);

    private:

        uint8_t typeID;
        String typeName;
    };

}


#endif
