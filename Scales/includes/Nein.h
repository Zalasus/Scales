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

#include <functional>

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

		bool equals(const String &s) const;

		bool endsWith(const String &ending) const;
		bool startsWith(const String &begin) const;
		bool isEmpty() const;

		String toUpperCase() const;
		String toLowerCase() const;

		size_t hashCode() const;

		int32_t length() const;

		uint8_t charAt(int32_t index) const;
		uint8_t *getBytes() const;

		int32_t indexOf(uint8_t c) const;
		int32_t indexOf(uint8_t c, int32_t startIndex) const;
		int32_t indexOf(const String &s) const;
		int32_t indexOf(const String &s, int32_t startIndex) const;

		String concat(const String &s) const;

		String substring(int32_t start) const;
		String substring(int32_t start, int32_t end) const;

		bool operator==(const String &s) const;

		String operator=(const String &s);

		String operator+=(const String &s);
		String operator+=(const char *s);
		String operator+=(char c);
		String operator+=(int i);

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

	String operator+(const String &left, const String &right);

	String operator+(const String &left, const char *right);
	String operator+(const char *left, const String &right);

	String operator+(const String &left, char right);
	String operator+(char left, const String &right);

	String operator+(const String &left, int right);
	String operator+(int left, const String &right);


	class ByteArrayOutputStream
	{
	public:
		ByteArrayOutputStream();
		~ByteArrayOutputStream();

		void write(uint8_t c);

		uint32_t size();

		void reset();

		uint8_t *toNewArray();
		uint8_t *newBufferCopy();
		uint8_t *getBuffer();

	private:
		uint8_t *buffer;
		uint32_t bufferSize;
		uint32_t count;

	};
}


namespace std
{
	template<>
	class hash<Scales::String>
	{
	public:
		size_t operator()(const Scales::String &s) const
		{
			return s.hashCode();
		}
	};
}

#endif /* STRING_H_ */
