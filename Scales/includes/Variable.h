
#ifndef VARIABLE_H_
#define VARIABLE_H_

#include "DataType.h"

namespace Scales
{

	class Value
	{

	public:

		DataType getType();

	};

    class VariablePrototype
    {

    public:

			VariablePrototype(const String &pName, const DataType &pType, const AccessType &pAccessType);

        	String getName() const;

        	DataType getType() const;

        	AccessType getAccessType() const;

        	void setScope(uint32_t i);
        	uint32_t getScope() const;

        private:

        	String name;
        	DataType type;
        	AccessType accessType;

        	uint32_t scope;

    };

}

#endif
