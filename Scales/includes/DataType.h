

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
        bool equals(const DataType &t) const;

        bool canCastImplicitlyTo(const DataType &t);

        void initSpecifier(const String &s);
        String getSpecifier() const;

        String toString() const;

        static DataType mathCast(const DataType &a, const DataType &b);
        static DataType byName(const String &s);
        static DataType byID(uint8_t id);

    private:

        DataType(uint8_t id, const String &name);

        uint8_t typeID;
        String typeName;

        String specifier;

        static vector<DataType> values;

    };


    class AccessType
    {

    public:

        	static const AccessType PRIVATE;
        	static const AccessType PUBLIC;
        	static const AccessType UNIVERSAL;

        	AccessType(const AccessType &t);

        	AccessType &operator=(const AccessType &t);

            uint8_t getTypeID() const;
            String getTypeName() const;

            bool equals(const AccessType &t) const;

            static AccessType byName(const String &s);
            static AccessType byID(uint8_t id);

        private:

            AccessType(uint8_t id, const String &name);

            uint8_t typeID;
            String typeName;

            static vector<AccessType> values;
    };

}


#endif
