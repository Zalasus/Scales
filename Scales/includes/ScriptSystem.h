
#ifndef SCRIPTSYSTEM_H_
#define SCRIPTSYSTEM_H_

#include "Script.h"
#include "Variable.h"

#include <vector>

namespace Scales
{

    class ScriptSystem //TODO: Make this class abstract if possible
    {

    public:

    	Script *getScript(const String &scriptname);

    	Variable *getUniversal(const String &name);

    	void declareScript(Script &script);

    private:

    	vector<Script> scripts;

    };

}

#endif
