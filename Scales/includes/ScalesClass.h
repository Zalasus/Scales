/*
 * ScalesClass.h
 *
 *  Created on: 06.07.2014
 *      Author: Zalasus
 */

namespace Scales
{
	class ClassId;
	class ClassPrototype;
	class Class;
}

#ifndef SCALESCLASS_H_
#define SCALESCLASS_H_

#include "ScalesUtil.h"
#include "ScalesType.h"
#include "ScalesFunction.h"
#include "ScalesSystem.h"
#include "ScalesVariable.h"
#include "ScalesConstants.h"

namespace Scales
{

	typedef std::vector<DataType> TypeList;

	class ClassId
	{
	public:
		ClassId(const String &classname, const String &nspace);

		String getClassname() const;
		String getNamespace() const;

		String toString() const;

		bool operator==(const ClassId &right) const;

		static const ClassId EMPTY;

	private:

		String name;
		String space;
	};

	/*
	 * This class contains all the declarations in a class like they are made in source.
	 * It can be compared to a header file in memory, and is the only thing needed for compiling
	 * other sources referencing to the class it is representing.
	 *
	 * Bytecode may be coupled with this container, which is required for creating runtime class
	 * instances of it. If no bytecode is included, it just serves prototyping purposes.
	 *
	 * This classes state may change after it is created.
	 */
	class ClassSketch
	{
	public:
		ClassSketch(const ClassId &pId, ClassSketch *pSuperclass, const String &pNativeLinkTarget);

		ClassId getClassId() const;
		ClassSketch *getSuperclass() const;
		String getNativeLinkTarget() const;

		void declareGlobal(const VariableSketch &proto);
		VariableSketch *getGlobal(const String &name, bool recursive = false);
		const std::vector<VariableSketch> getGlobals(bool recursive = false);
		uint32_t getGlobalCount() const;

		void declareFunction(const FunctionSketch &proto);
		FunctionSketch *getFunction(const String &name, const TypeList &paramTypes, bool recursive = false);
		uint32_t getFunctionCount() const;

		bool isComplete();

		/**
		 * Calculates the amount of value pointers required in an instance of this class.
		 * This basically equals the amount of non-native variables in this class.
		 */
		uint32_t getSize() const;

	private:

		ClassId classId;
		ClassSketch *superclass;
		String nativeLinkTarget;

		std::vector<VariableSketch> globals;

		std::vector<FunctionSketch> functions;
	};


	class Object; //Forward declaration from ScalesObject.h (as of cross reference)

	/*
	 * This class creates data needed for execution like memory-mappings
	 * from the clear-structured ClassSketch
	 *
	 * This classes state must be unchanged after it is created
	 * (except for variable values)
	 */
	class Class
	{
	public:
		Class(const ClassSketch &pSketch, ScalesSystem *pSystem);
		~Class();

		ClassSketch getSketch() const;
		ScalesSystem *getSystem() const;

		FunctionHandle *getStaticHandle(const String &name, const TypeList &paramTypes);

		Object *createInstance();

	private:

		Class(const Class &c); //This class is not copyable

		ClassSketch sketch;

		ScalesSystem *system;
	};

}

#endif /* SCALESCLASS_H_ */
