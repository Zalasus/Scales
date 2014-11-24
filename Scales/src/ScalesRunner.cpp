/*
 * ScalesRunner.cpp
 *
 *  Created on: 13.11.2014
 *      Author: Zalasus
 */


#include "ScalesRunner.h"

#include "ScalesException.h"
#include "ScalesUtil.h"
#include "ScalesRoot.h"

namespace Scales
{

	StackElement::StackElement(IValue *pValue)
	: value(pValue)
	{
		if(pValue == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to push pure null value on stack");
		}
	}

	IValue *StackElement::getValue() const
	{
		if(value->getValueType() == IValue::VT_REFERENCE)
		{
			return *((static_cast<ValueRef*>(value))->getReference());

		}else
		{
			return value;
		}
	}

	IValue *StackElement::getRaw() const
	{
		return value;
	}

	bool StackElement::isReference()
	{
		return value->getValueType() == IValue::VT_REFERENCE;
	}

	IValue *StackElement::operator->()
	{
		return getValue();
	}

	void StackElement::free()
	{
		SCALES_DELETE value;
	}

	Runner::Runner(Object *pObj, Root *pRoot, const Function *pFunc)
	: obj(pObj),
	  root(pRoot),
	  func(pFunc),
	  prog(nullptr),
	  progSize(0),
	  pc(0),
	  lStack(nullptr),
	  lStackSize(0),
	  lStackTop(0),
	  aStack(nullptr),
	  aStackSize(0),
	  aStackTop(0)
	{
		if(obj == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to run on null object");
		}

		if(func != nullptr)
		{
			if(func->isNative())
			{
				SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to run natives using bytecode runner");
			}

			pc = func->getAdress();

		}else
		{
			pc = 0; //if no function is given, we want to run the global scope program
		}

		prog = obj->getClass()->getProgramArray();
		progSize = obj->getClass()->getProgramSize();
	}

	Runner::~Runner()
	{
		for(uint32_t i = 0; i < lStackSize; i++)
		{
			SCALES_DELETE lStack[i];
		}

		SCALES_DELETE[] lStack;


		for(uint32_t i = 0; i < aStackSize; i++)
		{
			aStack[i].free();
		}

		SCALES_DELETE[] aStack;
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
			{
				uint32_t adr = readIntegral<uint32_t>();
				if(!checkCondition())
				{
					pc = adr;
				}
				break;
			}

			case OP_JUMP:
			{
				pc = readIntegral<uint32_t>();
				break;
			}

			case OP_RETURN:
			{
				finished = true;
				break;
			}

			case OP_NOP:
			{
				break;
			}

			case OP_BEGIN:
			{
				uint32_t blockLocalCount = readIntegral<uint32_t>();
				//This operation does nothing at the moment
				break;
			}

			case OP_END:
			{
				uint32_t blockLocalCount = readIntegral<uint32_t>();
				destroyLocals(blockLocalCount);
				break;
			}

			case OP_DECLARELOCAL:
			{
				String varname = readString<uint8_t>();
				DataType vartype = readDataType();
				getLStackElement(lStackTop++) = IValue::getNewPrimitiveFromType(vartype);
				break;
			}

			case OP_CALL:
			{
				String funcName = readString<uint8_t>();
				uint8_t paramCount = readIntegral<uint8_t>();
				functionCall(obj, funcName, paramCount);
				break;
			}

			case OP_CALL_MEMBER:
			{
				String funcName = readString<uint8_t>();
				uint8_t paramCount = readIntegral<uint8_t>();
				memberFunctionCall(funcName, paramCount);
				break;
			}

			case OP_CALL_STATIC:
			{
				String nspace = readString<uint8_t>();
				String classname = readString<uint8_t>();
				String funcName = readString<uint8_t>();
				uint8_t paramCount = readIntegral<uint8_t>();
				//TODO: Make static call here
				break;
			}


			case OP_DISCARD:
			{
				ensureAStackSize(1);
				aStackPop().free();
				break;
			}

			case OP_CLONE:
			{
				ensureAStackSize(1);
				aStackPush(aStackPeek()->copy());
				break;
			}

			case OP_POP_VAR:
			{
				ensureAStackSize(1);
				uint32_t globalIndex = readIntegral<uint32_t>();
				IValue *oldValue = obj->fields[globalIndex]; //store old value, we need to delete it after the assignment

				if(aStackPeek().isReference())
				{
					//TODO: type checking? although the compiler does that, and it's not fatal if type mismatches occur during runtime, we should check that (natives aren't checked by compiler for instance)

					obj->getFieldByIndex(globalIndex) = aStackPeek().getValue()->copy(); //A referenced value always needs to be copied. Otherwise variables could be re-linked to the same value

					aStackPop().free();

				}else
				{
					obj->getFieldByIndex(globalIndex) = aStackPeek().getValue(); //we don't need to copy a pure value on the stack. we just need to re-link the variable to it.

					aStackPop(); //since we have not copied the stack value, deleting it would result in a bad field. just throw it off the stack
				}

				SCALES_DELETE oldValue;

				break;
			}

			case OP_POP_LOCAL_VAR:
			{
				ensureAStackSize(1);
				uint32_t localIndex = readIntegral<uint32_t>();
				IValue *oldVal = getLStackElement(localIndex);

				if(aStackPeek().isReference())
				{
					getLStackElement(localIndex) = aStackPeek().getValue()->copy();

					aStackPop().free();

				}else
				{
					getLStackElement(localIndex) = aStackPeek().getRaw();

					aStackPop();
				}

				SCALES_DELETE oldVal;

				break;
			}

			case OP_POP_REF:
			{
				popRef(false);
				break;
			}

			case OP_POP_REF_SOFT:
			{
				popRef(true);
				break;
			}

			case OP_PUSH_REF:
			{
				uint32_t globalIndex = readIntegral<uint32_t>();
				aStackPush(StackElement(SCALES_NEW ValueRef(&(obj->getFieldByIndex(globalIndex))))); //TODO: Look at all these parentheses! Wonderful, isn't it? Like back in my LISP-days...
				break;
			}

			case OP_PUSH_LOCAL_REF:
			{
				uint32_t localIndex = readIntegral<uint32_t>();
				aStackPush(StackElement(SCALES_NEW ValueRef(&(getLStackElement(localIndex)))));
				break;
			}

			case OP_PUSH_STATIC_REF:
			{
				//static refs currently unsupported. just read names and push null value
				readString<uint8_t>();
				readString<uint8_t>();
				readString<uint8_t>();
				aStackPush(StackElement(nullptr));
				break;
			}

			case OP_GET_MEMBER:
			{
				ensureAStackSize(1);
				uint32_t memberIndex = readIntegral<uint32_t>();
				if(aStackPeek()->getValueType() != IValue::VT_OBJECT)
				{
					SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to access members of non-object type");
				}
				ValueObject *iobj = static_cast<ValueObject*>(aStackPeek().getValue());
				IValue **mref = &(iobj->getObject()->fields[memberIndex]);
				aStackPop().free();
				aStackPush(StackElement(SCALES_NEW ValueRef(mref)));
				break;
			}

			case OP_GET_INDEX:
			{
				ensureAStackSize(1);
				uint32_t index = readIntegral<uint32_t>();
				//TODO: further implement array/string access
				break;
			}

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

			case OP_TO_OBJECT:
			{
				ensureAStackSize(1);
				String nspace = readString<uint8_t>();
				String classname = readString<uint8_t>();
				const Class *cl = root->getClass(ClassID(nspace, classname));
				if(cl == nullptr)
				{
					SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to cast to non-existent class");
				}
				if(aStackPeek()->getValueType() != IValue::VT_OBJECT)
				{
					SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to cast non-object to object");
				}
				Object *iobj = static_cast<ValueObject*>(aStackPeek().getValue())->getObject();
				aStackPop().free();
				if(!(iobj->getClass()->is(cl) || iobj->getClass()->isSubclassOf(cl)))
				{
					SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to cast object to incompatible type");
				}
				aStackPush(StackElement(SCALES_NEW ValueObject(iobj, cl)));
				break;
			}

			case OP_TO_ABSTRACT_OBJECT:
			{
				ensureAStackSize(1);
				if(aStackPeek()->getValueType() != IValue::VT_OBJECT)
				{
					SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to cast non-object to abstract object");
				}
				Object *iobj = static_cast<ValueObject*>(aStackPeek().getValue())->getObject();
				aStackPop().free();
				aStackPush(StackElement(SCALES_NEW ValueObject(iobj, nullptr)));
				break;
			}


			case OP_TO_INT:
			{
				ensureAStackSize(1);
				IValue *val = aStackPeek().getValue();
				IValue *castedVal = numericCast<primitive_int_t>(val);
				aStackPop().free();
				aStackPush(StackElement(castedVal));
				break;
			}

			case OP_TO_LONG:
			{
				ensureAStackSize(1);
				IValue *val = aStackPeek().getValue();
				IValue *castedVal = numericCast<primitive_long_t>(val);
				aStackPop().free();
				aStackPush(StackElement(castedVal));
				break;
			}

			case OP_TO_FLOAT:
			{
				ensureAStackSize(1);
				IValue *val = aStackPeek().getValue();
				IValue *castedVal = numericCast<primitive_float_t>(val);
				aStackPop().free();
				aStackPush(StackElement(castedVal));
				break;
			}

			case OP_TO_DOUBLE:
			{
				ensureAStackSize(1);
				IValue *val = aStackPeek().getValue();
				IValue *castedVal = numericCast<primitive_double_t>(val);
				aStackPop().free();
				aStackPush(StackElement(castedVal));
				break;
			}


			case OP_PUSH_INT:
			{
				int32_t i = readIntegral<int32_t>();
				aStackPush(StackElement(SCALES_NEW ValuePrimitive<int32_t>(i)));
				break;
			}

			case OP_PUSH_LONG:
			{
				int64_t l = readIntegral<int64_t>();
				aStackPush(StackElement(SCALES_NEW ValuePrimitive<int64_t>(l)));
				break;
			}

			case OP_PUSH_FLOAT:
			{
				float f = readFloat();
				aStackPush(StackElement(SCALES_NEW ValuePrimitive<float>(f)));
				break;
			}

			case OP_PUSH_DOUBLE:
			{
				double d = readDouble();
				aStackPush(StackElement(SCALES_NEW ValuePrimitive<double>(d)));
				break;
			}

			case OP_PUSH_STRING:
			{
				String s = readString<uint32_t>();
				aStackPush(StackElement(SCALES_NEW ValuePrimitive<String>(s)));
				break;
			}

			case OP_PUSH_NULL:
			{
				aStackPush(StackElement(nullptr));
				break;
			}

			case OP_PUSH_THIS:
			case OP_PUSH_PARENT:
			{
				//TODO: make parent work as expected
				aStackPush(StackElement(SCALES_NEW ValueObject(obj)));
				break;
			}

			case OP_NEW:
			{
				String nspace = readString<uint8_t>();
				String classname = readString<uint8_t>();
				uint32_t paramCount = readIntegral<uint32_t>();
				const Class *cl = root->getClass(ClassID(nspace, classname));
				if(cl == nullptr)
				{
					SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to instantiate non-existent class");
				}
				Object *o = root->createObject(*cl);
				//TODO: call constructor here
				aStackPush(StackElement(SCALES_NEW ValueObject(o)));
				break;
			}

			}

			if(finished)
			{
				break;
			}

		}

	}

	IValue *Runner::functionCall(Object *target, const String &name, uint32_t paramCount)
	{
		ensureAStackSize(paramCount);

		TypeList paramTypes;
		//extract type parameters from stack
		for(uint32_t i = (aStackTop - paramCount); i < aStackTop; i++)
		{
			//TODO: If a value is null, there is no way of determining which function to call, as we can't get it's type. We need to make a more dynamic lookup for the function, so we pick the first fitting function
			paramTypes.push_back(aStack[i]->getDataType()); //for a null value this returns a void type
		}

		const Function *func = target->getClass()->getFunction(name, paramTypes);

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

		//Transfer parameters to aStack of new runner
		for(uint32_t i = 0; i < paramCount; i++)
		{
			r.aStackPush(aStackPeek()->copy());

			aStackPop().free();
		}

		r.run();

		if(func->getReturnType().getBase() != DataType::DTB_VOID) //If func is non-void, copy and return the return value
		{
			r.ensureAStackSize(1); //ensure there is a return value on the runner's stack

			return r.aStackPeek()->copy(); //return a copy of the value, so it can be pushed afterwards

			//No need to delete element in secondary runner; it is deleted when Runner is destroyed
		}

		return nullptr; //no return -> tell caller by returning null
	}

	IValue *Runner::memberFunctionCall(const String &name, uint32_t paramCount)
	{
		ensureAStackSize(paramCount + 1); //ensure all parameters and the member object are there

		StackElement element = (aStack[aStackTop - paramCount - 1]); //get the member object from stack

		if(element->getValueType() != IValue::VT_OBJECT)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to call member in non-object value");
		}

		ValueObject *ov = static_cast<ValueObject>(element.getValue());
		Object *o = ov->getObject();
		IValue *ret = functionCall(o, name, paramCount);

		aStackPop().free(); // pop the member object and free it

		return ret;
	}

	//TODO: This function may not be working. check it when uncommenting
	/*
	void Runner::destroyLocals(uint32_t amount)
	{


		for(uint32_t i = 0; i < amount; i++)
		{
			if((lStackTop < lStackSize) && (lStackTop > 0))
			{
				lStackTop--;

				SCALES_DELETE lStack[lStackTop];
				lStack[lStackTop] = nullptr;
			}
		}

	}*/

	void Runner::ensureAStackSize(uint32_t size)
	{
		if(aStackTop < size)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Stack corruption");
		}
	}

	StackElement Runner::aStackPop()
	{
		if(aStackTop <= 0)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Stack corruption");
		}

		return aStack[--aStackTop];
	}

	StackElement Runner::aStackPeek()
	{
		if(aStackTop <= 0)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Stack corruption");
		}

		return aStack[aStackTop - 1];

	}

	void Runner::aStackPush(StackElement e)
	{
		if(aStackTop > aStackSize)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Stack overflow");
		}

		aStack[aStackTop++] = e;
	}

	IValue *&Runner::getLStackElement(uint32_t i)
	{
		if(i >= lStackTop)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Stack corruption");
		}

		return lStack[i];
	}

	void Runner::popRef(bool soft)
	{
		ensureAStackSize(2);
		StackElement value = aStackPop();
		StackElement refer = aStackPop();
		if(!refer.isReference())
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to assign to value");
		}

		//TODO: We need to check if source is a reference to the target, and cancel the old-value-deletion if so

		IValue **irefer = (static_cast<ValueRef*>(refer.getRaw()))->getReference(); //we could include this reference-getter in StackElement
		IValue *oldValue = refer.getValue();

		if(value.isReference())
		{
			(*irefer) = value.getValue()->copy();

			value.free(); //references always need to be deleted

		}else
		{
			(*irefer) = value.getRaw();

			//no need to pop/free, as value is re-linked
		}

		SCALES_DELETE oldValue;

		if(!soft)
		{
			refer.free();

		}else //TODO: according to specs, we need to keep the value. here, we keep the reference. need to review the specs
		{
			aStackPush(refer);
		}
	}

	bool Runner::checkCondition()
	{
		IValue *iv = aStackPeek().getValue();

		if(iv->getValueType() != IValue::VT_PRIMITVE || iv->getDataType().isNumeric())
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to evaluate condition with non-numeric type");
		}

		if(iv->getDataType() == DataType::DTB_INT)
		{
			ValuePrimitive<primitive_int_t> *piv = static_cast< ValuePrimitive<primitive_int_t> >(iv);

			return piv->getData() != 0;

		}else if(iv->getDataType() == DataType::DTB_LONG)
		{
			ValuePrimitive<primitive_long_t> *piv = static_cast< ValuePrimitive<primitive_long_t> >(iv);

			return piv->getData() != 0;

		}else if(iv->getDataType() == DataType::DTB_FLOAT)
		{
			ValuePrimitive<primitive_float_t> *piv = static_cast< ValuePrimitive<primitive_float_t> >(iv);

			return piv->getData() != 0;

		}else if(iv->getDataType() == DataType::DTB_DOUBLE)
		{
			ValuePrimitive<primitive_double_t> *piv = static_cast< ValuePrimitive<primitive_double_t> >(iv);

			return piv->getData() != 0;
		}

		aStackPop().free();

		return false; //should never happen
	}

	template <typename T>
	IValue *Runner::numericCast(IValue *v)
	{
		if(v->getValueType() != IValue::VT_PRIMITVE || !v->getDataType().isNumeric())
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Tried to cast non-numerical to numerical value");
		}

		T newData = 0;

		if(v->getDataType() == DataType::DTB_INT)
		{
			ValuePrimitive<primitive_int_t> *piv = static_cast< ValuePrimitive<primitive_int_t> >(v);

			newData = static_cast<T>(piv->getData());

		}else if(v->getDataType() == DataType::DTB_LONG)
		{
			ValuePrimitive<primitive_long_t> *piv = static_cast< ValuePrimitive<primitive_long_t> >(v);

			newData = static_cast<T>(piv->getData());

		}else if(v->getDataType() == DataType::DTB_FLOAT)
		{
			ValuePrimitive<primitive_float_t> *piv = static_cast< ValuePrimitive<primitive_float_t> >(v);

			newData = static_cast<T>(piv->getData());

		}else if(v->getDataType() == DataType::DTB_DOUBLE)
		{
			ValuePrimitive<primitive_double_t> *piv = static_cast< ValuePrimitive<primitive_double_t> >(v);

			newData = static_cast<T>(piv->getData());
		}

		return SCALES_NEW ValuePrimitive<T>(newData);
	}

	void Runner::ensureProgSize(progAdress_t s)
	{
		if((pc + s) > progSize)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Program counter overflow");
		}
	}

	template <typename T>
	T Runner::readIntegral()
	{
		ensureProgSize(sizeof(T));

		T v = 0;

		for(uint32_t i = 0; i < sizeof(T); i++)
		{
			v |= prog[pc++] << (i * 8);
		}

		return v;
	}

	template <>
	progUnit_t Runner::readIntegral<progUnit_t>()
	{
		ensureProgSize(1);

		return prog[pc++];
	}

	template <>
	uint8_t Runner::readIntegral<uint8_t>()
	{
		ensureProgSize(1);

		return prog[pc++];
	}

	float Runner::readFloat()
	{
		ensureProgSize(sizeof(float));

		pc += sizeof(float);

		return 0;
	}

	double Runner::readDouble()
	{
		ensureProgSize(sizeof(double));

		pc += sizeof(double);

		return 0;
	}

	template <typename T>
	String Runner::readString()
	{
		T l = readIntegral<T>();
		ensureProgSize(l);

		String s = String(l, '\0');
		for(uint32_t i = 0; i < l; i++)
		{
			s[i] = prog[pc++];
		}

		return s;
	}

	DataType Runner::readDataType()
	{
		uint8_t typeID = readIntegral<uint8_t>();

		const Class *typeClass = nullptr;

		if(typeID == DataType::DTB_OBJECT)
		{
			String nspace = readString<uint8_t>();
			String classname = readString<uint8_t>();

			typeClass = root->getClass(ClassID(nspace, classname));

			if(typeClass == nullptr)
			{
				SCALES_EXCEPT(Exception::ET_RUNTIME, "Unknown class in bytecode");
			}
		}

		return DataType(typeID, typeClass);
	}
}

