/*
 * ScalesFunction.h
 *
 *  Created on: 04.10.2014
 *      Author: Zalasus
 */

#ifndef SCALESFUNCTION_H_
#define SCALESFUNCTION_H_

#include "ScalesString.h"
#include "ScalesAccess.h"
#include "ScalesType.h"
#include "ScalesBytecode.h"

namespace Scales
{

	class Function : public AccessElement
	{
	public:

		Function(const String &pName, const TypeList &pParamTypes);

		String getName() const;
		TypeList getParamTypes() const;

		DataType getReturnType() const;
		void setReturnType(const DataType &t);

		progAdress_t getAdress() const;
		void setAdress(progAdress_t a);
		bool hasAdress() const; //only true if function is not native and adress is not 0

		void setLocalCount(uint32_t l);
		uint32_t getLocalCount() const; // returns the maximum amount of local variables that reside in memory during execution of this function

		void setStackSize(uint32_t s);
		uint32_t getStackSize() const; // returns the amount of elements the will be on the stack at most

		bool isConstructor() const; //only true if function name is "init"

		bool matches(const String &pName, const TypeList &pParamTypes) const;

		static String getInfoString(const String &pName, const TypeList &paramTypes);
		static const String CONSTRUCTOR_NAME;

	private:

		String name;
		TypeList paramTypes;

		DataType returnType; //void by default

		progAdress_t adress; //A value of 0 means the function has no adress (the adress of a function should never be 0, as it is always preceeded by a JMP)
		uint32_t localCount;
		uint32_t stackSize;
	};

}


#endif /* SCALESFUNCTION_H_ */
