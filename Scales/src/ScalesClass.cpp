/*
 * ScalesClass.cpp
 *
 *  Created on: 06.07.2014
 *      Author: Zalasus
 */

#include "ScalesClass.h"

namespace Scales
{

	ClassId::ClassId(const String &classname, const String &nspace)
		: name(classname),
		  space(nspace)
	{
	}

	String ClassId::getClassname() const
	{
		return name;
	}

	String ClassId::getNamespace() const
	{
		return space;
	}

	String ClassId::toString() const
	{
		if(space.size() > 0)
		{
			return space + ":" + name;

		}else
		{
			return name;
		}
	}

	bool ClassId::operator==(const ClassId &right) const
	{
		return (name == right.getClassname()) && (space == right.getNamespace());
	}

	const ClassId ClassId::EMPTY = ClassId("","");

	//------------------------------------------------------

	ClassSketch::ClassSketch(const ClassId &pId, ClassSketch *pSuperclass, const String &pNativeLinkTarget)
		: classId(pId),
		  superclass(pSuperclass),
		  nativeLinkTarget(pNativeLinkTarget)
	{

	}

	ClassId ClassSketch::getClassId() const
	{
		return classId;
	}

	ClassSketch *ClassSketch::getSuperclass() const
	{
		return superclass;
	}

	String ClassSketch::getNativeLinkTarget() const
	{
		return nativeLinkTarget;
	}

	void ClassSketch::declareGlobal(const VariableSketch &proto)
	{
		globals.push_back(proto);
	}

	VariableSketch *ClassSketch::getGlobal(const String &name, bool recursive)
	{
		for(uint32_t i = 0; i < globals.size(); i++)
		{
			if(globals[i].getName() == name)
			{
				return &globals[i];
			}
		}

		return null;
	}

	const std::vector<VariableSketch> ClassSketch::getGlobals(bool recursive)
	{
		if(!recursive || superclass == null)
		{
			return globals;

		}else
		{
			std::vector<VariableSketch> vect = superclass->getGlobals(true);

			for(uint32_t i = 0; i < globals.size(); i++)
			{
				vect.push_back(globals[i]);
			}

			return vect;
		}
	}

	uint32_t ClassSketch::getGlobalCount() const
	{
		return globals.size();
	}

	void ClassSketch::declareFunction(const FunctionSketch &proto)
	{
		functions.push_back(proto);
	}

	FunctionSketch *ClassSketch::getFunction(const String &name, const TypeList &paramTypes, bool recursive)
	{
		for(uint32_t i = 0; i < functions.size(); i++)
		{
			const TypeList &protoParams = functions[i].getParameterTypes();

			if(functions[i].getName() != name || protoParams.size() != paramTypes.size())
			{
				continue;
			}

			for(uint32_t i2 = 0; i2 < protoParams.size(); i2++)
			{
				if(!(protoParams[i2] == paramTypes[i2]))
				{
					continue;
				}
			}

			return &functions[i];
		}

		if(recursive && superclass != null)
		{
			return superclass->getFunction(name, paramTypes, true);

		}else
		{
			return null;
		}
	}

	uint32_t ClassSketch::getFunctionCount() const
	{
		return functions.size();
	}

	bool ClassSketch::isComplete()
	{
		for(uint32_t i = 0; i < functions.size(); i++)
		{
			if(!functions[i].hasAdress())
			{
				return false;
			}
		}

		return true;
	}

	//-----------------------------------------------------------

	Class::Class(const ClassSketch &pSketch, ScalesSystem *pSystem)
		: sketch(pSketch),
		  system(pSystem)
	{

		//get the linked class from the reflection manager and establish connections to static elements here
	}

	Class::~Class()
	{
	}

	ClassSketch Class::getSketch() const
	{
		return sketch;
	}

	ScalesSystem *Class::getSystem() const
	{
		return system;
	}

	FunctionHandle *Class:getStaticFunctionHandle()
	{

	}

	Object *Class::createInstance()
	{
		Object *obj = new Object(this);
		parentSystem->registerValueForGC(obj);

		return obj;
	}

};



