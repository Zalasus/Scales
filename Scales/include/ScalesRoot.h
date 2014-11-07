/*
 * ScalesRoot.h
 *
 *  Created on: 03.10.2014
 *      Author: Zalasus
 */

#ifndef SCALESROOT_H_
#define SCALESROOT_H_

#include <vector>

#include "ScalesString.h"
#include "ScalesClass.h"
#include "ScalesObject.h"
#include "ScalesMemory.h"
#include "ScalesThread.h"
#include "compiler/ScalesCompiler.h"

namespace Scales
{
	//Every instantiation of major objects should be done through Root,
	//so Root can clean up afterwards and the user only needs to clean up Root properly
	class Root
	{

	public:

		Root();
		~Root();

		void loadLibrary(const String &filename);

		Compiler *getCompiler(compilerFlags_t compilerFlags);

		Class *createClass(const ClassID &classId, const Class *pSuperclass = nullptr);
		const Class *getClass(const ClassID &classId);

		const std::vector<const Class*> listClasses();
		const std::vector<const Class*> listClassesInNamespace(const String &nspace);

		/**
		 * Creates a new object. Attention: Once the program looses track of the returned pointer, there is no way
		 * of retrieving it, thus rendering the Object useless. Unused Objects are cleaned up automatically if the GC
		 * is running, or at the latest after the destruction of the Root object.
		 */
		Object *createObject(const Class &c);
		Object *createObject(const ClassID &classId);

		Thread *createThread();
		Thread *getThread(threadID_t tid);

	private:

		std::vector<Class*> classes;

		std::vector<Object*> objects;

		Compiler *compiler;
	};

}


#endif /* SCALESROOT_H_ */
