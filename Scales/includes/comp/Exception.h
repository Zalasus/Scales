/*
 * Exception
 *
 *  Created on: 28.04.2014
 *      Author: Niklas Weissner
 */

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include "Nein.h"

namespace Scales
{

	class Exception
	{
	public:
		Exception(String msg);

		String getMessage();

	private:

		String message;
	};

}


#endif /* EXCEPTION_H_ */
