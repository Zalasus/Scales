
#ifndef LEXER_H_
#define LEXER_H_

#include "Nein.h"
#include <istream>
#include <vector>

#define CH_LINEFEED 0x0A
#define I_DECIMAL_POINT '.'
#define I_STRING_START '"'
#define I_STRING_END '"'
#define I_COMMENT_SINGLELINE '#'
#define I_COMMENT_MULTILINE_CONT '*'

#define ALPHAS String("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_")
#define NUMBERS String("0123456789")

using std::istream;
using std::vector;

namespace Scales
{

	/*enum TokenType
	{
		TT_EOF, //0
		TT_COMMENT_SINGLELINE, //1
		TT_COMMENT_MULTILINE, //2
		TT_KEYWORD, //3
		TT_IDENT, //4
		TT_STRING, //5
		TT_NUMBER, //6
		TT_OPERATOR //7
	};*/


	class Token
	{
	public:

		enum TokenType
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

		Token(TokenType type, const String &lexem, uint32_t startIndex, uint32_t endIndex, uint32_t line);

		TokenType getType() const;

		String getLexem() const;

		uint32_t getStartIndex() const;
		uint32_t getEndIndex() const;
		uint32_t getLine() const;

		bool is(TokenType type, const String &lexem) const;
		bool is(TokenType type, const char* lexem) const;

	private:

		TokenType tokenType;

		String tokenLexem;

		uint32_t tokenStart;
		uint32_t tokenEnd;
		uint32_t tokenLine;
	};



    class Lexer
    {
    public:

        Lexer(istream &in);
        ~Lexer();

        Token readToken();

        Token peekToken();

        void declareKeyword(const String &s);
        void declareOperator(const String &s);

        void setIgnoreComments(bool ic);

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

        void lexerError(const String &message);
        void lexerError(const char* message);


        istream &input;

        int lchar; //Declared as int, as istream does not use stdint, so that int could mean anything, but we want lchar to be the same type as istream.get()
        Token nextToken;

        bool allowEOF; //Set to true by scanner if an occuring EOF should be counted as an error, like while reading a string literal
        bool ignoreComments;

        uint32_t currentLine;
        uint32_t currentIndex;

        vector<String> keywords;
        vector<String> operators;

    };

}

#endif
