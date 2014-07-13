/*
 * ScalesClass.cpp
 *
 *  Created on: 06.07.2014
 *      Author: Niklas Weissner
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

	bool ClassId::operator==(const ClassId &left, const ClassId &right) const
	{
		return (left.getClassname()== right.getClassname()) && (left.getNamespace() == right.getNamespace());
	}


	//----------------------------------------------------------------------------------------


	Class::Class(const ClassPrototype &proto, ScalesSystem *system)
		: prototype(proto),
		  parentSystem(system)
	{

	}

	Class::~Class()
	{

	}

	ClassPrototype Class::getPrototype() const
	{
		return prototype;
	}

	Object *Class::createInstance() const
	{
		return null;
	}

};



