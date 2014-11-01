/*
 * ScalesUtil.h
 *
 *  Created on: 06.07.2014
 *      Author: Zalasus
 */

namespace Scales
{
	class StringUtils;
}

#ifndef SCALESUTIL_H_
#define SCALESUTIL_H_

#include <string>
#include <sstream>
#include <vector>

#define null 0

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

		template <typename T>
		static String append(const String &s, T i);
	};

	String operator+(const String &s, uint32_t i);
	String operator+(const String &s, int32_t i);

};


#endif /* SCALESUTIL_H_ */
