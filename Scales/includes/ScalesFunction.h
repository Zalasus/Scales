/*
 * ScalesFunction.h
 *
 *  Created on: 08.07.2014
 *      Author: Zalasus
 */

namespace Scales
{
	class FunctionSketch;
}

#ifndef SCALESFUNCTION_H_
#define SCALESFUNCTION_H_

#include "ScalesType.h"
#include "ScalesVariable.h"

namespace Scales
{

	class FunctionSketch
	{
	public:
		FunctionSketch(const String &pName, const TypeList &pParamTypes, const DataType &pReturnType, bool pPrivate, bool pNative);
		FunctionSketch(const String &pName, const TypeList &pParamTypes, const DataType &pReturnType, bool pPrivate, bool pNative, uint32_t pAdress);

		String getName() const;
		const TypeList getParameterTypes() const;
		DataType getReturnType() const;
		bool isPrivate() const;
		bool isNative() const;
		uint32_t getAdress() const;

		bool hasAdress() const;
		void setAdress(uint32_t adr);

		void declareLocal(const VariableSketch &proto);
		VariableSketch *getLocal(const String &name, const Scope &scope); //No need for recursive parameter, locals are obviously not inherited

		inline bool isConstructor() const
		{
			return getName() == "init";
		}

		static String createInfoString(const String &name, const TypeList &paramTypes);

	private:

		String name;
		TypeList paramTypes;
		DataType returnType;
		bool native;

		bool hasAdr;
		uint32_t adress;
	};

	/*
	 * This class is used by a runtime Class for locking, calling and referencing
	 * it's own functions and the functions of others. It handles stuff like where
	 * to put the parameters, where to send the call to (bytecode runner or implementation) etc.
	 */
	class FunctionHandle
	{
	public:
		virtual ~FunctionHandle();

		String getName() const;

		void setLocked(bool l);
		bool isLocked() const;

		virtual Value *invoke(uint32_t paramCount, Value** params) = 0;

	};

	class NativeFunctionHandle : public FunctionHandle
	{
	public:

		Value *invoke(uint32_t paramCount, Value** params);

	private:

		Value *(*target)(uint32_t, Value**);

	};

}


#endif /* SCALESFUNCTION_H_ */
