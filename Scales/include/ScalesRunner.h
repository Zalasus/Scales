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

	class Root;

	class StackElement
	{
	public:

		StackElement(IValue *pValue);

		/**
		 * Returns the value of the stack element. Dereferences the value if it is a reference.
		 */
		IValue *getValue() const;

		/**
		 * Returns the value of the stack element. Does not dereference references.
		 */
		IValue *getRaw() const;

		bool isReference();

		/**
		 * Shorthand for .getValue()->
		 */
		IValue *operator->();

		/**
		 * Frees the memory pointed by this stack element. The element becomes invalid after this operation.
		 * Any access to functions of element after calling free() results in undefined behaviour.
		 */
		void free();

	private:

		IValue * const value;

	};

	class Runner
	{
	public:

		Runner(Object *pObj, Root *pRoot, const Function *pFunc);
		~Runner();

		void run();

	protected:
		void ensureAStackSize(uint32_t size);
		StackElement aStackPop();
		StackElement aStackPeek();
		void aStackPush(StackElement e);

	private:

		IValue *functionCall(Object *target, const String &name, uint32_t paramCount);
		IValue *memberFunctionCall(const String &name, uint32_t paramCount);

		void popRef(bool soft);

		IValue *&getLStackElement(uint32_t i);

		bool checkCondition();

		void destroyLocals(uint32_t amount);

		template <typename T>
		IValue *numericCast(IValue *v);

		uint8_t readUByte();
		uint32_t readUInt();
		int32_t readInt();
		int64_t readLong();
		float readFloat();
		double readDouble();
		String readBString();
		String readIString();
		DataType readDataType();

		Object *obj;
		Root *root;
		const Function *func;
		progUnit_t *prog;
		progAdress_t progSize;
		progAdress_t pc;

		StackElement *aStack;
		uint32_t aStackSize;
		uint32_t aStackTop;

		IValue **lStack;
		uint32_t lStackSize;
		uint32_t lStackTop;

	};

}


#endif /* SCALESRUNNER_H_ */
