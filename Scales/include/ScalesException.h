/*
 * ScalesException.h
 *
 *  Created on: 08.10.2014
 *      Author: Niklas Weissner
 */

#ifndef SCALESEXCEPTION_H_
#define SCALESEXCEPTION_H_

#include "ScalesString.h"

#ifdef _DEBUG

	#define SCALES_EXCEPT(type, msg) throw Scales::Exception(type, msg, __FILE__ , __LINE__ )

#else

	#define SCALES_EXCEPT(type, msg) throw Scales::Exception(type, msg)

#endif


namespace Scales
{

	class Exception
	{
	public:

		enum exceptionType_t
		{
			ET_UNKNOWN,

			ET_IO,
			ET_DATA_FORMAT,
			ET_FILE_NOT_FOUND,

			ET_COMPILER,

			ET_RUNTIME,

			ET_NOT_IMPLEMENTED
		};

#ifdef _DEBUG

		Exception(exceptionType_t pType, const String &pMsg, const String &file, uint32_t line);

#else

		Exception(exceptionType_t pType, const String &pMsg);

#endif

		exceptionType_t getType() const;
		String getMessage() const;


	private:

		exceptionType_t type;
		String msg;

	};

}


#endif /* SCALESEXCEPTION_H_ */
