

#ifndef DATATYPE_H_
#define DATATYPE_H_

#include "Nein.h"

#include <vector>

using std::vector;

namespace Scales
{

    class DataType
    {
    public:

    	static const DataType NOTYPE;
        static const DataType INT;
        static const DataType LONG;
        static const DataType FLOAT;
        static const DataType DOUBLE;
        static const DataType STRING;
        static const DataType OBJECT;

        DataType(const DataType &t);

        DataType &operator=(const DataType &t);

        uint8_t getTypeID() const;
        String getTypeName() const;

        bool isNumeric() const;
        bool equals(DataType t) const;

        static DataType mathCast(DataType a, DataType b);
        static DataType byName(String s);
        static DataType byID(uint8_t id);

    private:

        DataType(uint8_t id, String name);


        uint8_t typeID;
        String typeName;

        static String test;

        static vector<DataType> values;

    };

}


#endif
