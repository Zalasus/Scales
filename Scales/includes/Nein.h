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

/*
 * Nein
 * Knifto conform C++ coding
 *
 * -Don't use initializer lists. They are ugly, hard to read, confusing and they have almost no advantages at runtime when sticking to the next rule.
 *
 * -Don't make anything constant that is not assigned at compile time. It just doesn't fit a "constant" if it is still assigned at runtime, even if it happens only once. Same goes
 *  for parameters. It has almost no use to make them constant, as "call by reference" should also not be used (see next rule) and therefore, changing them has no effect anyway.
 *
 * -Try to avoid "call by reference". Most of the time, there are better ways to do stuff. It is confusing if a void function still "returns" something by altering it's parameters.
 *
 * -Overriding operators is a no-no. There are reasons the default operators are doing things how they are doing them. Define methods with meaningful names instead, so one can easily understand
 *  your program/API and assume the operators are working as they would do with any other object. Otherwise you get crap like string concatenation using the dot operator and shit (Official insult towards PHP users).
 *
 * -Always start classnames and namespaces with an uppercase letter, variable and function names with an lowercase letter and name constants in full uppercase.
 *  Call me a java idiot, but you can not deny that it has some advantages to be able to differ between typedefs for primitive types (uint8_t etc.), functions, variables and actual classes.
 *
 * -Use CamelCase (No underscores) for variable-,function- and classnames. Use underscores exclusively for constants and defines.
 *
 * -Where ever you don't need compatibility with existing libraries that do not the following, use the data types in stdint.h for integer values. long has at least 3 different meanings
 *  depending on the compiler used. Using stdint you can clearly see what you have. And by the way, what dou you think is more convenient to type, "unsigned long long" or "uint64_t"?
 *
 * -Don't use "using namespace [..]". You may use it if you have a REALLY big namespace, and you are really using ALL of the stuff inside it, but as this should be not that usual, you'd better
 *  stick to use "using namespace::[..]" for each class by its own. It has no really performance advantages, but one can clearly see what stuff of what namespace your code needs without looking
 *  through the whole file.
 *
 * -Create a new header-/sourcefile for each "public" class. You may put class declarations that are only used by one specific class in the same file as the one of that class, but
 *  try to maintain a structure where the filename somewhat resembles the main class defined/declared in it. It is very frustrating if you want to look at a class definition, and you have to look through
 *  the whole include list as none of the filenames listed there has anything in common with the name of the class you are looking for.
 *
 * -And for god's sake, code in english! Keywords are english, naming conventions are english, so don't mix them with your mother tongue, even if it may be easier for you. If you want
 *  to publish some of your code sometime, things like "getBuchungsschluessel()"(Real example. Saw that once.) are just plain ridicoulous. Same goes for comments, no matter how bad your
 *  english is. It's better to write comments in bad english which can still be understood by everyone than writing poetical masterpieces anyone but your nation understands.
 *
 */

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

}


#endif /* STRING_H_ */
