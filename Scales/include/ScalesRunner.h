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

		/**
		 * Returns a new StackElement with a copied IValue or nullptr, if this Element's value is also null
		 */
		StackElement clone();

		bool isReference();

		IValue *operator->();

		void free();

	private:

		IValue *value;

	};

	class Runner
	{
		typedef std::vector<StackElement> AStackType;
		typedef std::vector<IValue*> LStackType;

	public:

		Runner(Object *pObj, Root *pRoot, const Function *pFunc);
		~Runner();

		void run();

	protected:
		void ensureAStackSize(uint32_t size);
		AStackType &getAStack();

	private:

		void functionCall(Object *target, const String &name, uint32_t paramCount);
		void memberFunctionCall(const String &name, uint32_t paramCount);

		void popRef(bool soft);

		void popAndFreeAStack();
		IValue *&getLStackElement(uint32_t i);

		bool checkCondition();

		void destroyLocals(uint32_t amount);

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

		AStackType aStack;

		IValue **lStack;
		uint32_t lStackSize;
		uint32_t lStackTop;

	};

}


#endif /* SCALESRUNNER_H_ */
