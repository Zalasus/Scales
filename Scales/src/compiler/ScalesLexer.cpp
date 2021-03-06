
#include "compiler/ScalesLexer.h"
#include "ScalesException.h"

namespace Scales
{

	//public class lexer

    Lexer::Lexer(coding_t pCoding, const String *pKeywords, const uint32_t pKeywordCount, const String *pOperators, const uint32_t pOperatorCount, const bool pIgnoreComments)
    :
    		input(nullptr),
    		lchar(EOF),
    		keywords(pKeywords),
    		keywordCount(pKeywordCount),
    		operators(pOperators),
    		operatorCount(pOperatorCount),
    		ignoreComments(pIgnoreComments)
    {
        currentIndex = 0;
        currentLine = 1;

        allowEOF = true;
    }

    Lexer::~Lexer()
    {

    }

    //public

    void Lexer::setDataSource(std::istream *in)
    {
    	input = in;

    	reset();

    	readNext(); //Prefetch for pseudo-one-token lookahead
    }

    void Lexer::reset()
    {
    	currentIndex = 0;
		currentLine = 1;

		allowEOF = true;
    }

    Token Lexer::readToken()
    {
    	if(nextToken.getType() == Token::TT_EOF)
    	{
    		nextToken = readNextToken();
    	}

    	Token currentToken = nextToken;

    	nextToken = readNextToken();

        return currentToken;
    }

    Token Lexer::peekToken()
    {
        if(nextToken.getType() == Token::TT_EOF)
        {
        	nextToken = readNextToken();
        }

        return nextToken;
    }

    uint32_t Lexer::getCurrentLine()
    {
    	return currentLine;
    }

    //private

    Token Lexer::readNextToken()
    {
    	Token t = internalTokenRead();

    	while(ignoreComments && (t.getType() == Token::TT_COMMENT_SINGLELINE || t.getType() == Token::TT_COMMENT_MULTILINE))
    	{
    		t = internalTokenRead();
    	}

    	return t;
    }

    Token Lexer::internalTokenRead()
    {
    	allowEOF = true;

    	skipWhites();

    	if(lchar == EOF)
    	{
    		return Token(Token::TT_EOF, "end of stream", currentIndex, currentIndex, currentLine);
    	}

    	int startLine = currentLine;
    	int startIndex = currentIndex;

    	if(isAlpha(lchar))
    	{
    		String s = readIdent();

    		if(isKeyword(s))
    		{
    			return Token(Token::TT_KEYWORD, s, startIndex, currentIndex, startLine);
    		}else
    		{
    			return Token(Token::TT_IDENT, s, startIndex, currentIndex, startLine);
    		}
    	}else if(isNumeric(lchar))
		{
			String s = readNumber();

			return Token(Token::TT_NUMBER, s, startIndex, currentIndex, startLine);

		}else if(lchar == I_STRING_START)
		{
			String s = readStringLiteral();

			return Token(Token::TT_STRING, s, startIndex, currentIndex, startLine);

		}else if(lchar == I_COMMENT_SINGLELINE)
		{
			bool multiline = false;
			String s = String("");

			readNext();
			if(lchar == I_COMMENT_MULTILINE_CONT)
			{
				multiline = true;

			}

			while(lchar != EOF)
			{
				if(multiline)
				{
					String end = String("") + I_COMMENT_MULTILINE_CONT;

					if(lchar == I_COMMENT_SINGLELINE && StringUtils::endsWith(s, end))
					{
						//Found end token for multiline comment
						readNext(); //consume it
						break;
					}

				}else
				{
					if(lchar == CH_LINEFEED)
					{
						break;
					}
				}

				s += (char)lchar;
				readNext();
			}

			if(multiline)
			{
				//If multiline, cut away multiline indicators (check if there really is one at the end; comment might be closed by EOF, too)

				return Token(Token::TT_COMMENT_MULTILINE, StringUtils::substring(s, 1, StringUtils::endsWith(s,"*") ? s.length()-1 : s.length()), startIndex, currentIndex, startLine);

			}else
			{
				return Token(Token::TT_COMMENT_SINGLELINE, s, startIndex, currentIndex, startLine);
			}

		}else if(isPartOfOperator(String("") + (char)lchar))
		{
			String op = String("") + (char)lchar;

			while(isPartOfOperator(op))
			{
				readNext();
				op += (char)lchar;
			}

			String s = StringUtils::substring(op, 0, op.length()-1);

			if(!isOperator(s))
			{
				SCALES_EXCEPT(Exception::ET_COMPILER, "Unexpected character in operator");
			}

			return Token(Token::TT_OPERATOR, s, startIndex, currentIndex, startLine);
		}else
		{
			SCALES_EXCEPT(Exception::ET_COMPILER, "Unexpected character");
		}

    	return Token(Token::TT_EOF, "end of stream", currentIndex, currentIndex, currentLine);
    }


    String Lexer::readIdent()
    {
    	String result = String("");

		if(isAlpha(lchar))
		{

			do
			{
				result += (char) lchar;
				readNext();

			}while(isAlpha(lchar) || isNumeric(lchar));

		}

		return result;
    }

    String Lexer::readNumber()
	{
		String result = String("");
		bool dot = false;

		while(isNumeric(lchar) || (lchar == I_DECIMAL_POINT && !dot))
		{
			if(lchar == I_DECIMAL_POINT)
			{
				dot = true;
			}

			result += (char)lchar;
			readNext();
		}

		return result;
	}

    String Lexer::readStringLiteral()
	{
		readNext();
		String s = "";
		while(lchar != I_STRING_END && lchar != EOF)
		{
			if(lchar == '\\')
			{
				s += (char)lchar;
				readNext();
			}

			s += (char)lchar;
			readNext();
		}
		readNext();

		return s;
	}

    /**
     * Skips all non-printable characters
     */
    void Lexer::skipWhites()
	{
    	while((lchar < 0x21) && (lchar != EOF))
		{
			readNext();
		}
	}

    void Lexer::readNext()
	{
    	if(input == nullptr)
    	{
    		SCALES_EXCEPT(Exception::ET_COMPILER, "Missing or invalid data source");
    	}

    	if(!input->good())
    	{
    		SCALES_EXCEPT(Exception::ET_COMPILER, "Missing or invalid data source");
    	}

		lchar = input->get();

		if(lchar == EOF && !allowEOF)
		{
			SCALES_EXCEPT(Exception::ET_COMPILER, "Unexpected end of stream while scanning!");
		}

		if(lchar == CH_LINEFEED)
		{
			currentLine++;
		}

		currentIndex++;
	}

    bool Lexer::isKeyword(const String &s)
    {
    	for(uint32_t i = 0; i < keywordCount ; i++)
    	{
    		if(keywords[i] == s)
			{
				return true;
			}
    	}

    	return false;
    }

    bool Lexer::isOperator(const String &s)
	{
    	for(uint32_t i = 0; i < operatorCount ; i++)
		{
			if(operators[i] == s)
			{
				return true;
			}
		}

		return false;
	}

    bool Lexer::isPartOfOperator(const String &op)
	{
    	for(uint32_t i = 0; i < operatorCount; i++)
    	{
    		String so = operators[i];

    		if(StringUtils::startsWith(so, op))
    		{
    			return true;
    		}
    	}

		return false;
	}

    bool Lexer::isAlpha(int c)
    {
    	return StringUtils::indexOf(ALPHAS, (char)c) != -1;
    }

    bool Lexer::isNumeric(int c)
    {
    	return StringUtils::indexOf(NUMBERS, (char)c) != -1;
    }


    //private class token

    Token::Token()
	{
		tokenType = Token::TT_EOF;
		tokenLexem = String("");
		tokenStart = 0;
		tokenEnd = 0;
		tokenLine = 0;
	}

	Token::Token(Token::tokenType_t type, const String &lexem, uint32_t startIndex, uint32_t endIndex, uint32_t line)
	{
		tokenType = type;
		tokenLexem = lexem;
		tokenStart = startIndex;
		tokenEnd = endIndex;
		tokenLine = line;
	}

	Token::tokenType_t Token::getType() const
	{
		return tokenType;
	}

	String Token::getLexem() const
	{
		return tokenLexem;
	}

	uint32_t Token::getStartIndex() const
	{
		return tokenStart;
	}

	uint32_t Token::getEndIndex() const
	{
		return tokenEnd;
	}

	uint32_t Token::getLine() const
	{
		return tokenLine;
	}

	bool Token::is(Token::tokenType_t type, const String &lexem) const
	{
		return (tokenType == type && tokenLexem ==lexem);
	}

	bool Token::isLexem(const String &s) const
	{
		return !tokenLexem.compare(s);
	}

	bool Token::isType(Token::tokenType_t t) const
	{
		return tokenType == t;
	}
}
