
#include "comp/Lexer.h"
#include "Nein.h"
#include <iostream>
#include <cstdio>

namespace Scales
{

	Token::Token()
	{
		tokenType = TT_EOF;
		tokenLexem = String("");
		tokenStart = 0;
		tokenEnd = 0;
		tokenLine = 0;
	}

	Token::Token(TokenType type, String lexem, uint32_t startIndex, uint32_t endIndex, uint32_t line)
	{
		tokenType = type;
		tokenLexem = lexem;
		tokenStart = startIndex;
		tokenEnd = endIndex;
		tokenLine = line;
	}

	TokenType Token::getType()
	{
		return tokenType;
	}

	String Token::getLexem()
	{
		return tokenLexem;
	}

	uint32_t Token::getStartIndex()
	{
		return tokenStart;
	}

	uint32_t Token::getEndIndex()
	{
		return tokenEnd;
	}

	uint32_t Token::getLine()
	{
		return tokenLine;
	}

	bool Token::is(TokenType type, String lexem)
	{
		return (tokenType == type && tokenLexem.equals(lexem));
	}

	bool Token::is(TokenType type, const char *lexem)
	{
		return is(type, String(lexem));
	}

    Lexer::Lexer(istream &in) : input(in)
    {
        currentIndex = 0;
        currentLine = 1;

        //input = in;

        allowEOF = true;

        lchar = input.get();

        keywords = vector<String>();
        operators = vector<String>();
    }

    Lexer::~Lexer()
    {

    }

    //public

    Token Lexer::readToken()
    {
    	if(nextToken.getType() == TT_EOF)
    	{
    		nextToken = readNextToken();
    	}

    	Token currentToken = nextToken;

    	nextToken = readNextToken();

        return currentToken;
    }

    Token Lexer::peekToken()
    {
        if(nextToken.getType() == TT_EOF)
        {
        	nextToken = readNextToken();
        }

        return nextToken;
    }

    void Lexer::declareKeyword(String s)
    {
    	keywords.push_back(s);
    }

    void Lexer::declareOperator(String s)
	{
		operators.push_back(s);
	}

    //private

    Token Lexer::readNextToken()
    {
    	allowEOF = true;

    	skipWhites();

    	if(lchar == EOF)
    	{
    		return Token(TT_EOF, "", currentIndex, currentIndex, currentLine);
    	}

    	int startLine = currentLine;
    	int startIndex = currentIndex;

    	if(isAlpha(lchar))
    	{
    		String s = readIdent();

    		if(isKeyword(s))
    		{
    			return Token(TT_KEYWORD, s, startIndex, currentIndex, startLine);
    		}else
    		{
    			return Token(TT_IDENT, s, startIndex, currentIndex, startLine);
    		}
    	}else if(isNumeric(lchar))
		{
			String s = readNumber();

			return Token(TT_NUMBER, s, startIndex, currentIndex, startLine);

		}else if(lchar == I_STRING_START)
		{
			String s = readStringLiteral();

			return Token(TT_STRING, s, startIndex, currentIndex, startLine);

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
					//TODO: Strange way to do that. Better review it later
					String end = String("");
					end += I_COMMENT_MULTILINE_CONT;

					if(lchar == I_COMMENT_SINGLELINE && s.endsWith(end))
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

			return Token(multiline ? TT_COMMENT_MULTILINE : TT_COMMENT_SINGLELINE, s, startIndex, currentIndex, startLine);

		}else if(isPartOfOperator(String("") + (char)lchar))
		{
			String op = String("") + (char)lchar;

			while(isPartOfOperator(op))
			{
				readNext();
				op += (char)lchar;
			}

			String s = op.substring(0, op.length()-1);

			if(!isOperator(s))
			{
				lexerError("Lexer: Unexpected character in operator");
			}

			return Token(TT_OPERATOR, s, startIndex, currentIndex, startLine);
		}else
		{
			lexerError("Unexpected character");
		}

    	return Token(TT_EOF, "", currentIndex, currentIndex, currentLine);
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
		while(lchar != I_STRING_END || lchar != EOF)
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
		lchar = input.get();

		if(lchar == EOF && !allowEOF)
		{
			lexerError("Unexpected end of stream while scanning!");
		}

		if(lchar == CH_LINEFEED)
		{
			currentLine++;
		}

		currentIndex++;
	}

    bool Lexer::isKeyword(String s)
    {
    	for(uint32_t i = 0; i < keywords.size() ; i++)
    	{
    		if(keywords[i].equals(s))
			{
				return true;
			}
    	}

    	return false;
    }

    bool Lexer::isOperator(String s)
	{
    	for(uint32_t i = 0; i < operators.size() ; i++)
		{
			if(operators[i].equals(s))
			{
				return true;
			}
		}

		return false;
	}

    bool Lexer::isPartOfOperator(String op)
	{
    	for(uint32_t i = 0; i < operators.size(); i++)
    	{
    		String so = operators[i];

    		if(so.startsWith(op))
    		{
    			return true;
    		}
    	}

		return false;
	}

    bool Lexer::isAlpha(int c)
    {
    	return ALPHAS.indexOf((char)c) != -1;
    }

    bool Lexer::isNumeric(int c)
    {
    	return NUMBERS.indexOf((char)c) != -1;
    }

    void Lexer::lexerError(String msg)
    {
    	throw msg;
    }

    void Lexer::lexerError(const char* msg)
    {
    	lexerError(String(msg));
    }
}
