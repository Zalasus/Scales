/*
 * Nein.h
 *
 *  Created on: 20.04.2014
 *      Author: Niklas Weissner
 */

#ifndef NEIN_H_
#define NEIN_H_

#include <stdint.h>

#include <ostream>

#define null NULL

namespace Scales
{

	/**
	 * Convenient string class implementing behaviour like it would be expected from a java string.
	 * Also, this one is written with a capital letter, as every class should be!!!
	 */
	class String
	{
	public:
		String();
		~String();

		String(const String &s);

		String(const char *s);

		String(uint8_t *s, int32_t newLength);

		bool equals(String s) const;

		bool endsWith(String ending) const;
		bool startsWith(String begin) const;
		bool isEmpty() const;

		String toUpperCase() const;
		String toLowerCase() const;

		int32_t length() const;

		uint8_t charAt(int32_t index) const;
		uint8_t *getBytes() const;

		int32_t indexOf(uint8_t c) const;
		int32_t indexOf(uint8_t c, int32_t startIndex) const;
		int32_t indexOf(String s) const;
		int32_t indexOf(String s, int32_t startIndex) const;

		String concat(String s) const;

		String substring(int32_t start) const;
		String substring(int32_t start, int32_t end) const;

		String operator=(String s);

		String operator+=(String s);
		String operator+=(const char *s);
		String operator+=(char c);

		/*String operator+(String &left, String &right);

		String operator+(String &left, const char* right);
		String operator+(const char* left, String &right);

		String operator+(String &left, char right);
		String operator+(char left, String &right);*/

	private:

		uint8_t *data;

		int32_t len;
	};

	std::ostream& operator<<(std::ostream &out, const String &s);

	String operator+(String left, String right);

	String operator+(String left, const char *right);
	String operator+(const char *left, String right);

	String operator+(String left, char right);
	String operator+(char left, String right);

	String operator+(String left, int right);
	String operator+(int left, String right);

}


#endif /* STRING_H_ */
