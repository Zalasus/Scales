/*
 * ScalesException.cpp
 *
 *  Created on: 01.11.2014
 *      Author: Zalasus
 */

#include "ScalesException.h"

namespace Scales
{

#ifdef _DEBUG

	Exception::Exception(exceptionType_t pType, const String &pMsg, const String &file, uint32_t line)
	: type(pType),
	  msg(pMsg)
	{
		msg += " (" + file + " @ line " + line + ")";
	}

#else

	Exception::Exception(exceptionType_t pType, const String &pMsg)
	: type(pType),
	  msg(pMsg)
	{
	}

#endif

	Exception::exceptionType_t Exception::getType() const
	{
		return type;
	}

	String Exception::getMessage() const
	{
		return msg;
	}

}


