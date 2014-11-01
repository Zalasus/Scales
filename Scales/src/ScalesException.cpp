/*
 * ScalesException.cpp
 *
 *  Created on: 28.04.2014
 *      Author: Zalasus
 */

#include "ScalesException.h"

namespace Scales
{

	ScalesException::ScalesException(ExceptionType t, String msg) : type(t), message(msg)
	{

	}

	String ScalesException::getMessage()
	{
		return message;
	}

	ScalesException::ExceptionType ScalesException::getType()
	{
		return type;
	}

}


