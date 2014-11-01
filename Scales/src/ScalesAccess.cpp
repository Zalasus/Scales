/*
 * ScalesAccess.cpp
 *
 *  Created on: 08.10.2014
 *      Author: Zalasus
 */

#include "ScalesAccess.h"

namespace Scales
{

	void AccessElement::setPublic(bool b)
	{
		vPublic = b;
	}

	bool AccessElement::isPublic() const
	{
		return vPublic;
	}


	void AccessElement::setStatic(bool b)
	{
		vStatic = b;
	}

	bool AccessElement::isStatic() const
	{
		return vStatic;
	}


	void AccessElement::setNative(bool b)
	{
		vNative = b;
	}

	bool AccessElement::isNative() const
	{
		return vNative;
	}

}


