
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

    	Script *getScript(const ScriptIdent &scriptident);

    	void declareScript(Script &script);

    	bool isNamespaceDeclared(const String &nspace);

    private:

    	vector<Script> scripts;

    };

}

#endif
