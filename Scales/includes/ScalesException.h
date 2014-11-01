/*
 * Exception
 *
 *  Created on: 28.04.2014
 *      Author: Zalasus
 */

namespace Scales
{
	class ScalesException;
}

#ifndef SCALESEXCEPTION_H_
#define SCALESEXCEPTION_H_

#include "ScalesUtil.h"

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


#endif /* SCALESEXCEPTION_H_ */
