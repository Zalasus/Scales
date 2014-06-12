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

	class ScalesException
	{
	public:

		enum ExceptionType
		{
			ET_COMPILER,
			ET_RUNTIME
		};

		ScalesException(ExceptionType t, String msg);

		String getMessage();
		ExceptionType getType();

	private:

		ExceptionType type;
		String message;
	};

}


#endif /* EXCEPTION_H_ */
