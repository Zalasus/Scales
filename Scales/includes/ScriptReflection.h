

#ifndef SCRIPTREFLECTION_H_
#define SCRIPTREFLECTION_H_

#include "Nein.h"

#include <unordered_map>
/*
using std::unordered_map;

namespace Scales
{

	class ReflectedClass
	{
	public:


	};


	template<typename T>
	ReflectedClass *createReflectedClass()
	{
		return new T;
	}

	class ReflectedClassFactory
	{
	public:

		ReflectedClass newInstance;

	protected:

		static unordered_map<String, ReflectedClass*(*)> getMap();

	private:

		unordered_map<String, ReflectedClass*(*)> typeMap;
	};

	template<class T>
	class ReflectedClassRegisterer : public ReflectedClassFactory
	{
	public:

		ReflectedClassRegisterer(const String &reflectionUnitName)
		{
			getMap()[reflectionUnitName] = &createReflectedClass<T>;
		}
	};

}



#define EXPOSE_CLASS_FOR_REFLECTION(realClassname, reflectedClassname) \
	static ReflectedClass<realClassname> reflectedClassname( #reflectedClassname );

//define EXPOSE_STATIC_FOR_REFLECTION(realClassname, reflectedClassname)

*/

#endif
