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

	StackElement::StackElement(IValue *pValue, stackElementType_t pType)
	: value(pValue),
	  type(pType)
	{
	}

	IValue *StackElement::getValue() const
	{
		return value;
	}

	StackElement::stackElementType_t StackElement::getType() const
	{
		return type;
	}

	StackElement StackElement::clone() const
	{
		if(type == SE_REFERENCE || value == nullptr)
		{
			return StackElement(value, SE_REFERENCE);

		}else
		{
			return StackElement(value->copy(), SE_VALUE);
		}
	}

	void StackElement::free()
	{
		if(type == SE_VALUE && value != nullptr)
		{
			SCALES_DELETE value;

			value = nullptr;
		}
	}

	IValue *StackElement::operator->()
	{
		if(value == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Invalid access of null stack value");

		}else
		{
			return value;
		}
	}

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
			(*iter).free();
		}
	}

	void Runner::run()
	{
		bool finished = false;

		while(pc < progSize)
		{

			progUnit_t op = prog[pc++];

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
				functionCall(obj, funcName, paramCount);
				break;

			case OP_CALL_MEMBER:
				String funcName = readBString();
				uint8_t paramCount = readUByte();
				memberFunctionCall(funcName, paramCount);
				break;

			case OP_CALL_STATIC:
				String nspace = readBString();
				String classname = readBString();
				String funcName = readBString();
				uint8_t paramCount = readUByte();
				//TODO: Make static call here
				break;


			case OP_DISCARD:
				ensureAStackSize(1);
				popAndFreeAStack();
				break;

			case OP_CLONE:
				ensureAStackSize(1);
				aStack.push_back(aStack.back().clone());
				break;

			case OP_POP_VAR:
				ensureAStackSize(1);
				String fieldName = readBString();
				const Field *f = obj->getClass().getField(fieldName);
				if(f == nullptr)
				{
					SCALES_EXCEPT(Exception::ET_RUNTIME, "Bad bytecode: Field not found");
				}
				f->assign(obj, aStack.back()->copy());
				popAndFreeAStack();
				break;

			case OP_POP_REF:
				ensureAStackSize(2);
				StackElement value = aStack.back();
				StackElement refer = aStack[aStack.size()-2];
				if(refer.getType() != StackElement::SE_REFERENCE)
				{
					SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to assign to value");
				}
				//TODO: not sure if this whole ref system works. please continue here
				popAndFreeAStack();
				popAndFreeAStack();
				break;

			case OP_POP_REF_SOFT:
			case OP_PUSH_REF:
			case OP_PUSH_STATIC_REF:
			case OP_GET_MEMBER:
			case OP_GET_INDEX:

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

	void Runner::functionCall(Object *target, const String &name, uint32_t paramCount)
	{
		ensureAStackSize(paramCount);

		TypeList paramTypes;
		for(uint32_t i = (aStack.size() - 1); i > (aStack.size() - paramCount); i++)
		{
			if(aStack[i].getValue() == nullptr)
			{
				//a null instance is represented by a void type. TODO: we could replace this by a dedicated NULL type later
				paramTypes.push_back(DataType::_VOID);

			}else
			{
				//TODO: If a value is null, there is no way of determining which function to call, as we can't get it's type. We need to make a more dynamic lookup for the function, so we pick the first fitting function
				paramTypes.push_back(aStack[i]->getType());
			}
		}

		const Function *func = target->getClass().getFunction(name, paramTypes);

		if(func == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Bad bytecode: Function not found");
		}

		if(func->isNative())
		{
			//TODO: Implement native calls here

			SCALES_EXCEPT(Exception::ET_NOT_IMPLEMENTED, "Natives can't be called right now");
		}

		Runner r = Runner(target, func);

		for(uint32_t i = (aStack.size() - 1); i > (aStack.size() - paramCount); i++) // Transfer parameters to aStack of new runner TODO: Implement this using iterators
		{

			if(aStack[i].getValue() != nullptr)
			{
				r.getAStack().push_back(StackElement(aStack[i]->copy(), StackElement::SE_VALUE));

			}else
			{
				r.getAStack().push_back(StackElement(nullptr, StackElement::SE_VALUE));
			}

			aStack[i].free();
		}
		aStack.erase(aStack.end() - paramCount, aStack.end());

		r.run();

		if(func->getReturnType().getBase() != DataType::DTB_VOID) //If func is non-void, transfer the return value back to our aStack
		{
			r.ensureAStackSize(1); //ensure there is a return value on the runner's stack

			aStack.push_back(StackElement(r.getAStack().back()->copy(), StackElement::SE_VALUE));

			//No need to delete element in secondary runner; it is deleted when Runner is destroyed
		}
	}

	void Runner::memberFunctionCall(const String &name, uint32_t paramCount)
	{
		ensureAStackSize(paramCount + 1); //ensure all parameters and the member object are there

		IValue *v = *(aStack.end()-paramCount-1); //get the member object from stack

		if(v == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to call member in null value");
		}

		if(v->getType().getBase() != DataType::DTB_OBJECT)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to call member in non-object value");
		}

		IValueImpl<Object*> *ov = static_cast< IValueImpl<Object*> >(v); //TODO: A bit unsafe. Please check.
		Object *o = ov->getData();

		//we don't want the containing object on stack anymore. it is safely stored above, so delete the stack element
		SCALES_DELETE v;
		aStack.erase(aStack.end()-paramCount-1); // ...and remove it

		functionCall(o, name, paramCount);
	}

	void Runner::destroyLocals(uint32_t amount)
	{
		//TODO: Join loop and erase to one iterator

		for(uint32_t i = lStack.size() - amount; i < lStack.size(); i++)
		{
			delete lStack[i];
		}

		lStack.erase(lStack.end() - amount, lStack.end());

	}

	void Runner::popAndFreeAStack()
	{
		aStack.back().free();
		aStack.pop_back();
	}

}

