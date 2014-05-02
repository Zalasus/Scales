
#ifndef VARIABLE_H_
#define VARIABLE_H_

#include "DataType.h"

namespace Scales
{

    class Variable
    {

    public:

    		Variable(const String &pName, const DataType &pType, const AccessType &pAccessType);

        	String getName() const;

        	DataType getType() const;

        	AccessType getAccessType() const;

        private:

        	String name;
        	DataType type;
        	AccessType accessType;

    };

}

#endif
