/*
 * Exception.cpp
 *
 *  Created on: 28.04.2014
 *      Author: Niklas Weissner
 */

#include "comp/Exception.h"

namespace Scales
{

	Exception::Exception(String msg) : message(msg)
	{

	}

	String Exception::getMessage()
	{
		return message;
	}

}


