/*
 * ScalesRunner.h
 *
 *  Created on: 13.11.2014
 *      Author: Zalasus
 */

#ifndef SCALESRUNNER_H_
#define SCALESRUNNER_H_

#include <vector>

#include "ScalesBytecode.h"

namespace Scales
{

	class StackElement
	{
	public:

		enum stackElementType_t
		{
			SE_VALUE,
			SE_REFERENCE
		};

		StackElement(IValue *pValue, stackElementType_t pType);

		IValue *getValue() const;
		stackElementType_t getType() const;

		IValue *operator->();

		/**
		 * Returns a new instance of StackElement, containing a copied value if the contained value
		 * of this element is a value, and the same pointer if the contained value is a reference.
		 */
		StackElement clone() const;

		void free();

	private:

		IValue *value;
		stackElementType_t type;

	};

	class Runner
	{
		typedef std::vector<StackElement> AStackType;
		typedef std::vector<IValue*> LStackType;

	public:

		Runner(Object *pObj, const Function *pFunc);
		~Runner();

		void run();

	protected:
		void ensureAStackSize(uint32_t size);
		AStackType &getAStack();

	private:

		void functionCall(Object *target, const String &name, uint32_t paramCount);
		void memberFunctionCall(const String &name, uint32_t paramCount);

		void destroyLocals(uint32_t amount);
		inline void popAndFreeAStack();

		uint8_t readUByte();
		uint32_t readUInt();

		String readBString();
		String readIString();

		DataType readDataType();

		Object *obj;
		const Function *func;
		progUnit_t *prog;
		progAdress_t progSize;
		progAdress_t pc;

		AStackType aStack;
		LStackType lStack;

	};

}


#endif /* SCALESRUNNER_H_ */
