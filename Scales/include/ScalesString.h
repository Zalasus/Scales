/*
 * ScalesString.h
 *
 *  Created on: 03.10.2014
 *      Author: Niklas Weissner
 */

#ifndef SCALESSTRING_H_
#define SCALESSTRING_H_

#include <string>

namespace Scales
{

	typedef std::string String;

	class StringUtils
	{
	public:

		static bool startsWith(const String &s, const String &begin);
		static bool endsWith(const String &s, const String &end);

		static int32_t indexOf(const String &s, char find);
		static int32_t indexOf(const String &s, char find, int32_t startIndex);
		static int32_t indexOf(const String &s, const String &find);
		static int32_t indexOf(const String &s, const String &find, int32_t startIndex);

		static inline String substring(const String &s, uint32_t startIndex, uint32_t endIndex)
		{
			return s.substr(startIndex, endIndex - startIndex);
		}

	};

	String operator+(const String &left, uint32_t i);
	String operator+(const String &left, int32_t i);

}


#endif /* SCALESSTRING_H_ */
