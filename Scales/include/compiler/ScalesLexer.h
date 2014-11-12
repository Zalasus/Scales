/*
 * ScalesLexer.h
 *
 *  Created on: 08.10.2014
 *      Author: Zalasus
 */

#ifndef SCALESLEXER_H_
#define SCALESLEXER_H_

#include <cstdio>
#include <iostream>
#include <istream>

#include "ScalesString.h"

#define CH_LINEFEED 0x0A
#define I_DECIMAL_POINT '.'
#define I_STRING_START '"'
#define I_STRING_END '"'
#define I_COMMENT_SINGLELINE '#'
#define I_COMMENT_MULTILINE_CONT '*'

#define ALPHAS String("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_")
#define NUMBERS String("0123456789")

namespace Scales
{

	class Token
	{
	public:

		enum tokenType_t
		{
			TT_EOF, //0
			TT_COMMENT_SINGLELINE, //1
			TT_COMMENT_MULTILINE, //2
			TT_KEYWORD, //3
			TT_IDENT, //4
			TT_STRING, //5
			TT_NUMBER, //6
			TT_OPERATOR //7
		};

		Token();

		Token(tokenType_t type, const String &lexem, uint32_t startIndex, uint32_t endIndex, uint32_t line);

		tokenType_t getType() const;

		String getLexem() const;

		uint32_t getStartIndex() const;
		uint32_t getEndIndex() const;
		uint32_t getLine() const;

		bool is(tokenType_t type, const String &lexem) const;
		bool isLexem(const String &lexem) const;
		bool isType(tokenType_t type) const;

	private:

		tokenType_t tokenType;

		String tokenLexem;

		uint32_t tokenStart;
		uint32_t tokenEnd;
		uint32_t tokenLine;
	};


	/**
	 * Stateful lexer
	 */
    class Lexer
    {
    public:

		enum coding_t
		{
			CODING_ASCII
		};

        Lexer(coding_t pCoding, const String *keywords, const uint32_t keywordCount, const String *operators, const uint32_t operatorCount, const bool ignoreComments);
        ~Lexer();

        void setDataSource(std::istream *in);
        void reset();

        Token readToken();

        Token peekToken();

        uint32_t getCurrentLine();

    private:

        void readNext();
        Token readNextToken();
        Token internalTokenRead();

        void skipWhites();

        String readIdent();
        String readNumber();
        String readStringLiteral();

        bool isKeyword(const String &s);
        bool isOperator(const String &s);
    	bool isPartOfOperator(const String &s);

        bool isAlpha(int c);
        bool isNumeric(int c);

        std::istream *input;

        int lchar; //Declared as int, as istream does not use stdint, so that int could mean anything, but we want lchar to be the same type as istream.get()
        Token nextToken;

        bool allowEOF; //Set to true by scanner if an occuring EOF should be counted as an error, like while reading a string literal

        uint32_t currentLine;
        uint32_t currentIndex;

        const String *keywords;
        const uint32_t keywordCount;
        const String *operators;
        const uint32_t operatorCount;

        const bool ignoreComments;
    };

}

#endif /* SCALESLEXER_H_ */
