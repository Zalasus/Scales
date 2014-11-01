/*
 * ScalesSystem.h
 *
 *  Created on: 06.07.2014
 *      Author: Zalasus
 */

namespace Scales
{
	class ScalesSystem;
}

#ifndef SCALESSYSTEM_H_
#define SCALESSYSTEM_H_

#include "ScalesClass.h"
#include "ScalesLibrary.h"
#include "ScalesValue.h"
#include "ScalesUtil.h"
#include "ScalesType.h"

#include <vector>

namespace Scales
{

	class ClassSketch;

	class ScalesSystem
	{
	public:

		ScalesSystem();
		~ScalesSystem();

		Class *createClass(const ClassSketch &proto);
		Class *getClass(const ClassId &id);

		void loadLibrary(const Library &lib);

		void forceGarbageCollection();
		/**
		 * Registers a dynamically allocated value for garbage collection, removing the need for the caller to
		 * deallocate it properly (as long as the ScalesSystem is deallocated properly)
		 */
		ValuePtr *registerValueForGC(Value *val);
		/**
		 * Unregisters a dynamically allocated value from garbage collection and deallocates it.
		 */
		void deallocateValue(Value *val);

	private:

		std::vector<Class*> classes;

		std::vector<ValuePtr*> memory;
	};

}



#endif /* SCALESSYSTEM_H_ */
