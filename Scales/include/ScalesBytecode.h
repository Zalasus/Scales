/*
 * ScalesBytecode.h
 *
 *  Created on: 08.10.2014
 *      Author: Zalasus
 */

#ifndef SCALESBYTECODE_H_
#define SCALESBYTECODE_H_

#include <stdint.h>

namespace Scales
{

	typedef uint32_t progAdress_t;

	typedef uint8_t progUnit_t;

	enum opcode_t
	{
		OP_JUMP,
		OP_JUMP_IF_FALSE,
		OP_RETURN,
		OP_NOP,

		OP_BEGIN,
		OP_END,
		OP_DECLARELOCAL,

		OP_CALL,
		OP_CALL_MEMBER,
		OP_CALL_STATIC,

		OP_DISCARD,
		OP_CLONE,
		OP_POP_VAR,
		OP_POP_LOCAL_VAR,
		OP_POP_REF,
		OP_POP_REF_SOFT,
		OP_PUSH_REF,
		OP_PUSH_LOCAL_REF,
		OP_PUSH_STATIC_REF,
		OP_GET_MEMBER,
		OP_GET_INDEX,

		OP_ADD,
		OP_SUBTRACT,
		OP_MULTIPLY,
		OP_DIVIDE,
		OP_NEGATE,
		OP_LOGIC_OR,
		OP_LOGIC_AND,
		OP_INVERT,
		OP_COMPARE,
		OP_LESS,
		OP_GREATER,
		OP_LESS_EQUAL,
		OP_GREATER_EQUAL,

		//Attention! Order of the following opcodes is essential!
		OP_TO_INT,
		OP_TO_LONG,
		OP_TO_FLOAT,
		OP_TO_DOUBLE,
		OP_TO_STRING,
		OP_TO_ABSTRACT_OBJECT,
		OP_TO_OBJECT,

		OP_PUSH_INT,
		OP_PUSH_LONG,
		OP_PUSH_FLOAT,
		OP_PUSH_DOUBLE,
		OP_PUSH_STRING,
		OP_PUSH_NULL,
		OP_PUSH_THIS,
		OP_PUSH_PARENT,
		OP_NEW

	};
}

#endif /* SCALESBYTECODE_H_ */
