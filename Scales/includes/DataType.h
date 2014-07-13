

#ifndef DATATYPE_H_
#define DATATYPE_H_

#include <vector>

#include "ScalesUtil.h"
#include "ScalesClass.h"

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
        static const DataType ABSTRACT_OBJECT;

        DataType(const DataType &t);

        DataType &operator=(const DataType &t);

        uint8_t getTypeID() const;
        const String &getTypeName() const;

        bool isNumeric() const;
        bool equals(const DataType &t) const;

        bool isArray() const;

        bool canCastImplicitlyTo(const DataType &t);

        void initObjectType(const ClassId &id);
        const ClassId &getObjectType() const;

        const String toString() const;

        //No references on these, we want the types to be copied!
        static DataType mathCast(const DataType &a, const DataType &b);
        static DataType byName(const String &s);
        static DataType byID(uint8_t id);

    private:

        DataType(uint8_t id, const String &name);

        uint8_t typeID;
        String typeName;

        ClassId objectType;

        static vector<DataType> values;

    };

}


#endif
