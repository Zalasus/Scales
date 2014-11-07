/*
 * ScalesRoot.cpp
 *
 *  Created on: 03.10.2014
 *      Author: Zalasus
 */

#include "ScalesUtil.h"
#include "ScalesRoot.h"
#include "compiler/ScalesDefaultCompiler.h"

#include <iostream>

namespace Scales
{

	Root::Root()
	 : compiler(nullptr)
	{
	}

	Root::~Root()
	{
		SCALES_DELETE compiler;

		for(auto iter = classes.begin(); iter != classes.end(); iter++)
		{
			SCALES_DELETE *iter;
		}

		for(auto iter = objects.begin(); iter != objects.end(); iter++)
		{
			SCALES_DELETE *iter;
		}
	}

	Compiler *Root::getCompiler(compilerFlags_t compilerFlags)
	{
		if(compilerFlags != CF_GENERIC) //No other compilers than generic defined at this time
		{
			return nullptr;
		}

		if(compiler == nullptr)
		{
			compiler = SCALES_NEW DefaultCompiler(this, compilerFlags);
		}

		return compiler;
	}

	Class *Root::createClass(const ClassID &classId, const Class *pSuperclass)
	{
		if(getClass(classId) != nullptr)
		{
			return nullptr; //double definition. return nullptr
		}

		Class *c = SCALES_NEW Class(classId, pSuperclass);

		classes.push_back(c);

		return c;
	}

	const Class *Root::getClass(const ClassID &classId)
	{
		for(uint32_t i = 0; i < classes.size(); i++)
		{
			Class *c = classes[i];

			if(c->getID() == classId)
			{
				return c;
			}
		}

		return nullptr;
	}

	const std::vector<const Class*> Root::listClasses()
	{
		//TODO: Check if there is better way of making the vector elements const than shoving them into another vector

		std::vector<const Class*> newList;

		for(uint32_t i = 0; i < classes.size(); i++)
		{
			newList.push_back(classes[i]);
		}

		return newList;
	}

	const std::vector<const Class*> Root::listClassesInNamespace(const String &s)
	{

		//TODO: Check if there is better way of making the vector elements const than shoving them into another vector

		std::vector<const Class*> newList;

		for(uint32_t i = 0; i < classes.size(); i++)
		{
			if(classes[i]->getID().getNamespace() == s)
			{
				newList.push_back(classes[i]);
			}
		}

		return newList;
	}

	Object *Root::createObject(const Class &c)
	{
		Object *o = SCALES_NEW Object(c);

		objects.push_back(o);

		return o;
	}

}


