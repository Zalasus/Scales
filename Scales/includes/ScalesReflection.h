/*
 * ScalesReflection.h
 *
 *  Created on: 06.07.2014
 *      Author: Niklas Weissner
 */

#ifndef SCALESREFLECTION_H_
#define SCALESREFLECTION_H_

#include "ScalesUtil.h"

namespace Scales
{
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

SCALES_EXPOSE_BEGIN(_foo, Scales::foo)

SCALES_EXPOSE_END(_foo, Scales::foo)

#endif /* SCALESREFLECTION_H_ */
