/*
 * Nein.cpp
 *
 *  Created on: 21.04.2014
 *      Author: Niklas Weissner
 */

#include "Nein.h"
#include <cstdio>

namespace Scales
{

	String::String()
	{
		len = 0;

		data = new uint8_t[len];
	}

	String::String(const String &s)
	{
		len = s.length();

		data = new uint8_t[len];

		for(int32_t i = 0; i < len; i++)
		{
			data[i] = s.charAt(i);
		}
	}

	String::String(const char *s)
	{
		//Count bytes until null terminator for string length
		const char *sp = s;

		while(*(sp++))
		{
			if((sp - s) >= 0xFFFF)
			{
				break;
			}
		}

		len = sp - s - 1;

		data = new uint8_t[len];

		for(int32_t i = 0; i < len; i++)
		{
			data[i] = s[i];
		}
	}

	String::String(uint8_t *s, int32_t newLength)
	{
		len = newLength;

		data = s;
	}

	String::~String()
	{
		delete[] data;
	}

	int32_t String::length() const
	{
		return len;
	}

	uint8_t* String::getBytes() const
	{
		return data;
	}

	uint8_t String::charAt(int32_t index) const
	{
		return data[index];
	}

	int32_t String::indexOf(uint8_t c) const
	{
		return indexOf(c,0);
	}

	int32_t String::indexOf(uint8_t c, int32_t startIndex) const
	{
		for(int32_t i = startIndex; i<len; i++)
		{
			if(data[i] == c)
			{
				return i;
			}
		}

		return -1;
	}

	int32_t String::indexOf(String s) const
	{
		return indexOf(s, 0);
	}

	int32_t String::indexOf(String s, int32_t startIndex) const
	{
		if((s.length() + startIndex) > len || s.isEmpty())
		{
			return -1;
		}

		for(int32_t i = startIndex ; i < (len - s.length()); i++)
		{
			bool found = true;

			for(int32_t j = 0 ; j < s.length(); j++)
			{
				if(data[i + j] != s.charAt(j))
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

	bool String::equals(String s) const
	{
		if(s.length() != len)
		{
			return false;
		}

		for(int32_t i = 0; i < len; i++)
		{
			if(data[i] != s.charAt(i))
			{
				return false;
			}
		}

		return true;
	}

	bool String::endsWith(String ending) const
	{
		if(ending.length() > len || ending.isEmpty() || isEmpty())
		{
			return false;
		}

		for(int32_t i = 0; i < ending.length(); i++)
		{
			if(data[i + (len - ending.length())] != ending.charAt(i))
			{
				return false;
			}
		}

		return true;
	}

	bool String::startsWith(String start) const
	{
		if(start.length() > len || start.isEmpty() || isEmpty())
		{
			return false;
		}

		for(int32_t i = 0; i < start.length(); i++)
		{
			if(data[i] != start.charAt(i))
			{
				return false;
			}
		}

		return true;
	}

	bool String::isEmpty() const
	{
		return len == 0;
	}

	String String::toUpperCase() const
	{
		uint8_t *newData = new uint8_t[len];

		for(int32_t i = 0; i < len; i++)
		{
			uint8_t cp = data[i];

			if(cp >= 'a' && cp <= 'z')
			{
				newData[i] = cp - 32; //ASCII offset between upper- and lowercase chars
			}else
			{
				newData[i] = cp;
			}
		}

		return String(newData, len);
	}

	String String::toLowerCase() const
	{
		uint8_t *newData = new uint8_t[len];

		for(int32_t i = 0; i < len; i++)
		{
			uint8_t cp = data[i];

			if(cp >= 'A' && cp <= 'Z')
			{
				newData[i] = cp + 32; //ASCII offset between upper- and lowercase chars
			}else
			{
				newData[i] = cp;
			}
		}

		return String(newData, len);
	}

	String String::concat(String s) const
	{
		int32_t newLen = len + s.length();
		uint8_t *newData = new uint8_t[newLen];

		for(int32_t i = 0; i < newLen; i++)
		{
			if(i < len)
			{
				newData[i] = data[i];
			}else
			{
				newData[i] = s.charAt(i - len);
			}
		}

		return String(newData, newLen);
	}

	String String::substring(int32_t start, int32_t end) const
	{
		int32_t newLen = end-start;
		uint8_t *newData = new uint8_t[newLen];

		for(int32_t i = start; i < end ; i++)
		{
			newData[i-start] = data[i];
		}

		return String(newData, newLen);
	}

	String String::substring(int32_t start) const
	{
		return substring(start, len);
	}


	String String::operator=(String s)
	{
		delete[] data;

		data = new uint8_t[s.length()];
		len = s.length();

		for(int32_t i = 0; i < len; i++)
		{
			data[i] = s.charAt(i);
		}

		return *this;
	}

	String String::operator+=(String s)
	{
		int32_t newLen = len + s.length();
		uint8_t *newData = new uint8_t[newLen];

		for(int32_t i = 0; i < newLen; i++)
		{
			if(i < len)
			{
				newData[i] = data[i];
			}else
			{
				newData[i] = s.charAt(i - len);
			}
		}

		delete[] data;
		data = newData;
		len = newLen;

		return *this;
	}

	String String::operator+=(const char *s)
	{
		//Count bytes until null terminator for string length
		const char *sp = s;

		while(*(sp++))
		{
			if((sp - s) >= 0xFFFF)
			{
				break;
			}
		}

		int32_t newLen = len + (sp - s - 1);
		uint8_t *newData = new uint8_t[newLen];

		for(int32_t i = 0; i < newLen; i++)
		{
			if(i < len)
			{
				newData[i] = data[i];
			}else
			{
				newData[i] = s[i - len];
			}
		}

		delete[] data;
		data = newData;
		len = newLen;

		return *this;
	}

	String String::operator+=(char c)
	{
		int32_t newLen = len + 1;
		uint8_t *newData = new uint8_t[newLen];

		for(int32_t i = 0; i < len; i++)
		{
			newData[i] = data[i];
		}
		newData[len] = c;

		delete[] data;
		data = newData;
		len = newLen;

		return *this;
	}


	std::ostream& operator<<(std::ostream &out, const String &s)
	{

		for(int32_t i = 0; i < s.length(); i++)
		{
			out.put(s.charAt(i));
		}

		return out;
	}

	String operator+(String left, String right)
	{
		return left.concat(right);
	}

	String operator+(String left, const char *right)
	{
		String s = String(right);

		return left.concat(s);
	}

	String operator+(const char *left, String right)
	{
		String s = String(left);

		return s.concat(right);
	}

	String operator+(String left, char right)
	{
		uint8_t *newData = new uint8_t[left.length() + 1];

		for(int32_t i = 0; i < left.length(); i++)
		{
			newData[i] = left.charAt(i);
		}
		newData[left.length()] = right;

		return String(newData, left.length() + 1);
	}

	String operator+(char left, String right)
	{
		uint8_t *newData = new uint8_t[right.length() + 1];

		for(int32_t i = 1; i <= right.length(); i++)
		{
			newData[i] = right.charAt(i);
		}
		newData[0] = left;

		return String(newData, right.length() + 1);
	}

	String operator+(String left, int right)
	{
		//TODO: Make this more efficient. sprintf is not really known as a performance-monster

		char *newData = new char[16]; //11 chars are needed for longest string representation of 32 bit int; 5 chars reserve

		sprintf(newData, "%d", right);

		return left.concat(String(newData));
	}

	String operator+(int left, String right)
	{
		//TODO: Make this more efficient. sprintf is not really known as a performance-monster

		char newData[16]; //11 chars are needed for longest string representation of 32 bit int; 5 chars reserve

		sprintf(newData, "%d", left);

		return String(newData).concat(right);
	}

}
