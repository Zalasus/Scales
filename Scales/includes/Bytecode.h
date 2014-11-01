/*
 * Bytecode.h
 *
 *  Created on: 04.06.2014
 *      Author: Zalasus
 */

#ifndef BYTECODE_H_
#define BYTECODE_H_

namespace Scales
{

	enum Opcode
	{
		OP_NOP = 0,

		OP_NEW = 4,
		OP_PUSHVAR = 10,
		OP_POPVAR = 12,

		OP_ADD = 13,
		OP_SUBTRACT = 14,
		OP_MULTIPLY = 15,
		OP_DIVIDE = 16,
		OP_NEGATE = 17,

		OP_COMPARE = 18,
		OP_INVERT = 19,
		OP_LESS = 20,
		OP_GREATER = 21,
		OP_LESSEQUAL = 22,
		OP_GREATEREQUAL = 23,
		OP_LOGICOR = 24,
		OP_LOGICAND = 25,

		OP_GETMEMBER = 28,
		OP_CLONE = 30,

		OP_JUMP = 35,
		OP_JUMPFALSE = 37,
		OP_CALL = 38,
		OP_RETURN = 39,
		OP_BEGIN = 40,
		OP_END = 41,
		OP_CALLMEMBER = 44,

		OP_GETINDEX = 50,

		OP_TOINT = 60,
		OP_TOLONG = 61,
		OP_TOFLOAT = 62,
		OP_TODOUBLE = 63,
		OP_TOSTRING = 64,
		OP_TOOBJECTINSTANCE = 65,
		OP_TOOBJECT = 66,

		OP_PUSHINT = 70,
		OP_PUSHLONG = 71,
		OP_PUSHFLOAT = 72,
		OP_PUSHDOUBLE = 73,
		OP_PUSHSTRING = 74,

		OP_DEREFER = 80,
		OP_POPREF = 85,

		OP_PUSHNULL = 90,
		OP_DISCARD = 91,
		OP_PUSHTHIS = 92
	};

}

#endif /* BYTECODE_H_ */
