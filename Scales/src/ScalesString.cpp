/*
 * ScalesString.cpp
 *
 *  Created on: 31.10.2014
 *      Author: Zalasus
 */

#include <sstream>

#include "ScalesString.h"

namespace Scales
{

	bool StringUtils::startsWith(const String &s, const String &begin)
	{
		if(begin.length() > s.length() || begin.length() == 0 || s.length() == 0)
		{
			return false;
		}

		for(uint32_t i = 0; i < begin.length(); i++)
		{
			if(s[i] != begin[i])
			{
				return false;
			}
		}

		return true;
	}

	bool StringUtils::endsWith(const String &s, const String &end)
	{
		if(end.length() > s.length() || end.length() == 0 || s.length() == 0)
		{
			return false;
		}

		for(uint32_t i = 0; i < end.length(); i++)
		{
			if(s[i + (s.length() - end.length())] != end[i])
			{
				return false;
			}
		}

		return true;
	}

	int32_t StringUtils::indexOf(const String &s, char find)
	{
		return indexOf(s, find, 0);
	}

	int32_t StringUtils::indexOf(const String &s, char find, int32_t startIndex)
	{
		for(uint32_t i = startIndex; i<s.length(); i++)
		{
			if(s[i] == find)
			{
				return i;
			}
		}

		return -1;
	}

	int32_t StringUtils::indexOf(const String &s, const String &find)
	{
		return indexOf(s, find, 0);
	}

	int32_t StringUtils::indexOf(const String &s, const String &find, int32_t startIndex)
	{
		if((find.length() + startIndex) > s.length() || find.length() == 0)
		{
			return -1;
		}

		for(uint32_t i = startIndex ; i < (s.length() - find.length()); i++)
		{
			bool found = true;

			for(uint32_t j = 0 ; j < s.length(); j++)
			{
				if(s[i + j] != find[j])
				{
					found = false;
					break;
				}
			}

			if(found)
			{
				return i;
			}
		}

		return -1;
	}

	String operator+(const String &s, uint32_t i)
	{
		std::ostringstream out;

		out << s << i;
		return out.str();
	}

	String operator+(const String &s, int32_t i)
	{
		std::ostringstream out;

		out << s << i;
		return out.str();
	}

}
