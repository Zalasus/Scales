/*
 * ScalesRunner.cpp
 *
 *  Created on: 13.11.2014
 *      Author: Zalasus
 */


#include "ScalesRunner.h"

#include "ScalesException.h"
#include "ScalesUtil.h"

namespace Scales
{

	Runner::Runner(Object *pObj, const Function *pFunc)
	: obj(pObj),
	  func(pFunc)
	{
		prog = obj->getClass().getProgramArray();
		progSize = obj->getClass().getProgramSize();

		if(func != nullptr)
		{
			if(func->isNative())
			{
				SCALES_EXCEPT(Exception::ET_RUNTIME, "Can't run natives using bytecode runner");
			}

			pc = func->getAdress();

		}else
		{
			pc = 0;
		}
	}

	Runner::~Runner()
	{
		for(auto iter = lStack.begin(); iter != lStack.end(); iter++)
		{
			SCALES_DELETE *iter;
		}

		for(auto iter = aStack.begin(); iter != aStack.end(); iter++)
		{
			SCALES_DELETE *iter;
		}
	}

	void Runner::run()
	{
		bool finished = false;

		while(pc < progSize)
		{

			progUnit_t op = prog[pc];

			switch(op)
			{
			case OP_JUMP_IF_FALSE:
			case OP_JUMP:
				pc = readUInt();
				break;

			case OP_RETURN:
				finished = true;
				break;


			case OP_NOP:
				break;


			case OP_BEGIN:
				uint32_t blockLocalCount = readUInt();
				lStack.reserve(lStack.capacity() + blockLocalCount);
				break;

			case OP_END:
				uint32_t blockLocalCount = readUInt();
				destroyLocals(blockLocalCount);
				break;

			case OP_DECLARELOCAL:
				String varname = readBString();
				DataType vartype = readDataType();
				lStack.push_back(IValue::getInstanceFromType(vartype));
				break;


			case OP_CALL:
				String funcName = readBString();
				uint8_t paramCount = readUByte();
				functionCall(funcName, paramCount);
				break;

			case OP_CALL_MEMBER:
				String funcName = readBString();
				uint8_t paramCount = readUByte();
				memberFunctionCall(funcName, paramCount);
				break;

			case OP_CALL_STATIC:

			case OP_DISCARD:
			case OP_DEREFER:
			case OP_CLONE:
			case OP_POPVAR:
			case OP_POPREF:
			case OP_SOFTPOPREF:
			case OP_PUSHREF:
			case OP_PUSHREF_STATIC:
			case OP_GETMEMBER:
			case OP_GETINDEX:

			case OP_ADD:
			case OP_SUBTRACT:
			case OP_MULTIPLY:
			case OP_DIVIDE:
			case OP_NEGATE:
			case OP_LOGIC_OR:
			case OP_LOGIC_AND:
			case OP_INVERT:
			case OP_COMPARE:
			case OP_LESS:
			case OP_GREATER:
			case OP_LESS_EQUAL:
			case OP_GREATER_EQUAL:

			case OP_TO_OBJECT_INSTANCE:
			case OP_TO_INT:
			case OP_TO_LONG:
			case OP_TO_FLOAT:
			case OP_TO_DOUBLE:

			case OP_PUSH_INT:
			case OP_PUSH_LONG:
			case OP_PUSH_FLOAT:
			case OP_PUSH_DOUBLE:
			case OP_PUSH_STRING:
			case OP_PUSH_NULL:
			case OP_PUSH_THIS:
			case OP_PUSH_PARENT:
			case OP_NEW:

			}

			if(finished)
			{
				break;
			}

		}

	}

	void Runner::functionCall(const String &name, uint32_t paramCount)
	{
		ensureAStackSize(paramCount);

		TypeList paramTypes;
		for(uint32_t i = (aStack.size() - 1); i > (aStack.size() - paramCount); i++)
		{
			paramTypes.push_back(aStack[i]->getType());
		}

		const Function *func = obj->getClass().getFunction(name, paramTypes);

		if(func == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Bad bytecode: Function not found");
		}

		if(func->isNative())
		{
			//TODO: Implement native calls here

			SCALES_EXCEPT(Exception::ET_NOT_IMPLEMENTED, "Natives can't be called right now");
		}

		Runner r = Runner(obj, func);

		for(uint32_t i = (aStack.size() - 1); i > (aStack.size() - paramCount); i++) // Transfer parameters to aStack of new runner TODO: Implement this using iterators
		{
			r.getAStack().push_back(aStack[i]->copy());
		}

		r.run();

		if(func->getReturnType().getBase() != DataType::DTB_VOID) //If func is non-void, transfer the return value back to our aStack
		{
			r.ensureAStackSize(1); //ensure there is a return value on the runner's stack

			aStack.push_back(r.getAStack().back()->copy());
		}
	}

	void Runner::memberFunctionCall(const String &name, uint32_t paramCount)
	{
		ensureAStackSize(paramCount + 1); //ensure all parameters and the member object are there

		IValue *v = *(aStack.back()-paramCount); //get the member object from stack

		//TODO: ensure value is not null before proceeding
		if(v->getType().getBase() != DataType::DTB_OBJECT)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Can't call member functions in non-object value");
		}

		IValueImpl<Object*> *ov = static_cast< IValueImpl<Object*> >(v); //TODO: A bit unsafe. Please check.

		Object *o
	}

	void Runner::destroyLocals(uint32_t amount)
	{

		for(uint32_t i = lStack.size() - amount; i < lStack.size(); i++)
		{
			delete lStack[i];
		}

	}

}

