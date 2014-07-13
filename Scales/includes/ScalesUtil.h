/*
 * ScalesUtil.h
 *
 *  Created on: 06.07.2014
 *      Author: Niklas Weissner
 */

#ifndef SCALESUTIL_H_
#define SCALESUTIL_H_

#include <string>

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
	};
};


#endif /* SCALESUTIL_H_ */
