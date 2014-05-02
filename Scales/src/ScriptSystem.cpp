
#include "ScriptSystem.h"
#include "Variable.h"

namespace Scales
{


	Script *ScriptSystem::getScript(const String &name)
	{
		for(uint32_t i = 0; i < scripts.size(); i++)
		{
			if(scripts[i].getName().equals(name))
			{
				return &scripts[i];
			}
		}

		return null;
	}

	Variable *ScriptSystem::getUniversal(const String &name)
	{
		return null;
	}

	void ScriptSystem::declareScript(Script &script)
	{
		if(getScript(script.getName()) == null)
		{
			scripts.push_back(script);
		}
	}

}
