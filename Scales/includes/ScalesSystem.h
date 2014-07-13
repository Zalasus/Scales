/*
 * ScalesSystem.h
 *
 *  Created on: 06.07.2014
 *      Author: Niklas Weissner
 */

#ifndef SCALESSYSTEM_H_
#define SCALESSYSTEM_H_

#include <vector>

#include "ScalesClass.h"
#include "ScalesUtil.h"


namespace Scales
{

	class ScalesSystem
	{
	public:

		ScalesSystem();
		~ScalesSystem();


		Class *createClass(const ClassPrototype &proto);
		Class *getClass(const ClassId &id);


		void loadLibaray(const String &filename);

		uint32_t getMemoryUsage() const;
		void forceGarbageCollection();


	private:

		std::vector<Class*> classes;
	};

}



#endif /* SCALESSYSTEM_H_ */
