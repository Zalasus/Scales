/*
 * ScalesReflection.h
 *
 *  Created on: 06.07.2014
 *      Author: Zalasus
 */

namespace Scales
{
	class LinkedClass;
}

#ifndef SCALESREFLECTION_H_
#define SCALESREFLECTION_H_

#include "ScalesUtil.h"

namespace Scales
{

	/**
	 * Class that is extended by all classes that should be able to be linked to scripts
	 */
	class LinkedClass
	{

	};


	// \/ old     /\ new

	class LinkedVariable
	{
	public:
		LinkedVariable(String varname, void *ptr);
	};

	class ReflectedClassFactory
	{
	public:
		static std::vector<ReflectedClassFactory*> reflectedClasses;

		~ReflectedClassFactory();

		String getLinkName();

	protected:

		String linkName;
		std::vector<LinkedVariable*> vars;


	};

	template <class T>
	class ReflectedClass : public ReflectedClassFactory
	{
	public:
		ReflectedClass();
	};

	class foo
	{
	private:
		int bar;
	};

};

#define SCALES_EXPOSE_BEGIN(reflectionUnitName, reflectedClass) \
	Scales::ReflectedClass<reflectedClass>::ReflectedClass()\
	{\
		linkName = #reflectionUnitName;

#define SCALES_EXPOSE_END(reflectionUnitName, reflectedClass) \
		reflectedClasses.push_back((ReflectedClassFactory*)this);\
	}\
	static Scales::ReflectedClass<reflectedClass> reflectionUnitName;

#define SCALES_EXPOSE_VAR(varname, varptr) vars.push_back(new Scales::LinkedVariable(#varname, &varptr));


#endif /* SCALESREFLECTION_H_ */
