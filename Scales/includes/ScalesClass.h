/*
 * ScalesClass.h
 *
 *  Created on: 06.07.2014
 *      Author: Niklas Weissner
 */

#ifndef SCALESCLASS_H_
#define SCALESCLASS_H_

#include "ScalesUtil.h"
#include "ScalesObject.h"
#include "ScalesType.h"
#include "ScalesFunction.h"
#include "ScalesVariable.h"

namespace Scales
{

	class ClassId
	{
	public:
		ClassId(const String &classname, const String &nspace);

		String getClassname() const;
		String getNamespace() const;

		String toString() const;

		bool operator==(const ClassId &left, const ClassId &right) const;

	private:

		String name;
		String space;
	};


	//This classes state may change after it is created
	class ClassPrototype
	{
	public:
		ClassPrototype(const ClassId &pId, Class *superclass, const String &nativeLinkTarget);

		ClassId getClassId() const;
		const Class *getSuperclass() const;
		String getNativeLinkTarget() const;

		void declareGlobalPrototype(const VariablePrototype &proto);
		VariablePrototype getGlobalPrototype(const String &name, bool recursive = false);
		const std::vector<VariablePrototype> getGlobalPrototypes(bool recursive = false);

		void declareStaticPrototype(const VariablePrototype &proto);
		VariablePrototype getStaticPrototype(const String &name, bool recursive = false); //According to the Scales v1 specification, statics are not inherited. Therefore the recursive param is ignored
		const std::vector<VariablePrototype> getStaticPrototypes(bool recursive = false); //It is, however, still provided as later language versions may allow static inheritance

	private:

		std::vector<VariablePrototype> globals;
		std::vector<VariablePrototype> statics;
	};


	//This classes state must be unchanged after it is created (except for variable values, if you count them as state of the class)
	class Class
	{
	public:
		Class(const ClassPrototype &proto, ScalesSystem *system);
		~Class();

		ClassPrototype getPrototype() const;

		Object *createInstance() const;

	private:

		ClassPrototype prototype;

		ScalesSystem *parentSystem;

	};

}

#endif /* SCALESCLASS_H_ */
