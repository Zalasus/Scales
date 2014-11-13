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

	class Runner
	{
	public:

		Runner(Object *pObj, const Function *pFunc);
		~Runner();

		void run();

	protected:
		void ensureAStackSize(uint32_t size);
		std::vector<IValue*> &getAStack();

	private:

		void functionCall(Object *target, const String &name, uint32_t paramCount);
		void memberFunctionCall(const String &name, uint32_t paramCount);

		void destroyLocals(uint32_t amount);


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

		std::vector<IValue*> aStack;
		std::vector<IValue*> lStack;

	};

}


#endif /* SCALESRUNNER_H_ */
