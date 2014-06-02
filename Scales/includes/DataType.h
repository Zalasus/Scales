

#ifndef DATATYPE_H_
#define DATATYPE_H_

#include "Nein.h"

#include <vector>

using std::vector;

namespace Scales
{

	/* Note: This would require us to allocate every single DataType dynamically
	class DataType
	{
	public:

		enum DataTypeGroup
		{
			DT_PRIMITIVE,
			DT_OBJECT,
			DT_ARRAY
		};

		virtual ~DataType();

		virtual DataTypeGroup getGroup() const;

		virtual String toString() const;

	};

	class PrimitiveType : DataType
	{
	public:

		DataTypeGroup getGroup() const
		{
			return DT_PRIMITIVE;
		}

		String toString() const;

	};

	class ObjectType : DataType
	{
	public:

		DataTypeGroup getGroup() const
		{
			return DT_OBJECT;
		}

		String toString() const;

		String getNamespace() const;
		String getScript() const;

	private:

		String nspace;
		String script;

	};

	class ArrayType : DataType
	{
	public:

		ArrayType(DataType *pElementType, int pSize);

		DataTypeGroup getGroup() const
		{
			return DT_ARRAY;
		}

		String toString() const;

		int getSize() const;
		DataType *getElementType() const;

	private:

		DataType *elementType;
		int size;

	};*/

	class ScriptIdent
	{
	public:

		ScriptIdent(String pNspace, String pSname);

		const String &getNamespace() const;
		const String &getScriptname() const;

		bool equals(const ScriptIdent &scriptid) const;

		const String toString() const;

		ScriptIdent &operator=(const ScriptIdent& si);

	private:

		String nspace;
		String sname;
	};

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
        const String &getTypeName() const;

        bool isNumeric() const;
        bool equals(const DataType &t) const;

        bool canCastImplicitlyTo(const DataType &t);

        void initObjectType(const ScriptIdent &scriptident);
        const ScriptIdent &getObjectType() const;

        const String toString() const;

        //No references on these, we want the types to be copied!
        static DataType mathCast(const DataType &a, const DataType &b);
        static DataType byName(const String &s);
        static DataType byID(uint8_t id);

    private:

        DataType(uint8_t id, const String &name);

        uint8_t typeID;
        String typeName;

        ScriptIdent objectType;

        static vector<DataType> values;

    };


    class AccessType
    {

    public:

        	static const AccessType PRIVATE;
        	static const AccessType PUBLIC;

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
