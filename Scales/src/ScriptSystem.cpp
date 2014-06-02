
#include "ScriptSystem.h"


namespace Scales
{

	Script *ScriptSystem::getScript(const ScriptIdent &scriptident)
	{
		for(uint32_t i = 0; i < scripts.size(); i++)
		{

			if(scripts[i].getIdent().equals(scriptident))
			{
				return &scripts[i];
			}
		}

		return null;
	}

	void ScriptSystem::declareScript(Script &script)
	{
		if(getScript(script.getIdent()) == null)
		{
			scripts.push_back(script);
		}
	}

	bool ScriptSystem::isNamespaceDeclared(const String &nspace)
	{
		for(uint32_t i = 0; i < scripts.size(); i++)
		{
			if(scripts[i].getIdent().getNamespace().equals(nspace))
			{
				return true;
			}
		}

		return false;
	}

}
