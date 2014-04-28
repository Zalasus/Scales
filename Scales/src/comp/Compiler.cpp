
#include "comp/Compiler.h"

#include <iostream>

namespace Scales
{

	//public class compiler

    Compiler::Compiler(istream &in)
    {
    	lexer = new Lexer(in);

    	keywords = vector<String>();
    	keywords.push_back("begin");
    	keywords.push_back("end");
    	keywords.push_back("public");
    	keywords.push_back("private");
    	keywords.push_back("universal");
    	keywords.push_back("func");
    	keywords.push_back("native");
    	keywords.push_back("void");
    	keywords.push_back("script");
    	keywords.push_back("static");
    	keywords.push_back("links");
    	keywords.push_back("extends");
    	keywords.push_back("uses");
    	keywords.push_back("return");
    	keywords.push_back("while");
    	keywords.push_back("if");
    	keywords.push_back("elseif");
    	keywords.push_back("else");
    	keywords.push_back("break");
    	keywords.push_back("null");
    	keywords.push_back("this");
    	keywords.push_back("goto");
    	keywords.push_back("remark");
    	keywords.push_back("new");

    	datatypes = vector<String>();
    	datatypes.push_back("int");
    	datatypes.push_back("long");
    	datatypes.push_back("float");
    	datatypes.push_back("double");
    	datatypes.push_back("string");
    	datatypes.push_back("object");

    	operators = vector<String>();
    	operators.push_back("+");
    	operators.push_back("-");
    	operators.push_back("*");
    	operators.push_back("/");
    	operators.push_back("=");
    	operators.push_back("+=");
    	operators.push_back("-=");
    	operators.push_back("*=");
    	operators.push_back("/=");
    	operators.push_back("(");
    	operators.push_back(")");
    	operators.push_back("->");
    	operators.push_back("<");
    	operators.push_back(">");
    	operators.push_back("<=");
    	operators.push_back(">=");
    	operators.push_back("!=");
    	operators.push_back("==");
    	operators.push_back("&");
    	operators.push_back("|");
    	operators.push_back("!");
    	operators.push_back(".");
    	operators.push_back(",");
    	operators.push_back(";");


    	for(uint16_t i = 0; i < keywords.size(); i++)
    	{
    		lexer->declareKeyword(keywords[i]);
    	}

    	for(uint16_t i = 0; i < datatypes.size(); i++)
		{
			lexer->declareKeyword(datatypes[i]);
		}

    	for(uint16_t i = 0; i < operators.size(); i++)
		{
			lexer->declareOperator(operators[i]);
		}

    	lexer->setIgnoreComments(true);
    }

    Compiler::~Compiler()
    {
    	delete lexer;
    }

    void Compiler::compile()
    {
    	ExpressionInfo info = expression();

    	std::cout << "The type of that expression is " << info.getType().getTypeName() << " and it is " << (info.isConstant() ? "constant" : "not constant") << std::endl;
    }

    ExpressionInfo Compiler::functionCall(Token ident, bool member)
    {
    	String funcName = ident.getLexem();

		Token t = lexer->readToken();
		if(!t.is(TT_OPERATOR,"("))
		{
			error("Expected opening parentheses after function name in function call", t.getLine());
		}

		int paramCount = 0;

		t = lexer->peekToken();
		if(!t.is(TT_OPERATOR,")"))
		{

			do
			{
				expression();

				paramCount++;
				t = lexer->readToken();

			}while(t.is(TT_OPERATOR,","));

			if(!t.is(TT_OPERATOR,")"))
			{
				error("Expected closing parentheses after parameter list in function call", t.getLine());
			}

		}else
		{
			lexer->readToken();
		}

		if(member)
		{
			writeASM("CALLMEMBER '" + funcName + "'," + paramCount);
		}else
		{
			writeASM("CALL '" + funcName + "'," + paramCount);
		}

		//TODO: Somehow retrieve correct data type

    	return ExpressionInfo(DataType::NOTYPE, false); //Function returns are never considered as constant
    }

    ExpressionInfo Compiler::expression()
    {
        ExpressionInfo info = relationalExpression();

        Token t = lexer->peekToken();
        while(isLogicOp(t))
        {
            lexer->readToken();

            ExpressionInfo rightInfo = relationalExpression();

            if(!info.getType().isNumeric())
			{
				error(String("Left hand side of operator ") + t.getLexem() + " is of non-numeric type(" + info.getType().getTypeName() + "), but " + t.getLexem() + " requires numeric types (representing booleans)", t.getLine());
			}
			if(!rightInfo.getType().isNumeric())
			{
				error(String("Right hand side of operator ") + t.getLexem() + " is of non-numeric type(" + rightInfo.getType().getTypeName() +"), but " + t.getLexem() + " requires numeric types (representing booleans)", t.getLine());
			}

            info = ExpressionInfo(DataType::INT, rightInfo.isConstant() && info.isConstant()); //Type is always int after logic operation

            if(t.is(TT_OPERATOR,"|"))
            {
                writeASM("LOGICOR");

            }else if(t.is(TT_OPERATOR,"&"))
            {
                writeASM("LOGICAND");
            }

            t = lexer->peekToken();
        }

        return info;
    }

    ExpressionInfo Compiler::relationalExpression()
    {
        ExpressionInfo info = arithmeticExpression();

        Token t = lexer->peekToken();
        while(isRelationalOp(t))
        {
            lexer->readToken();

            ExpressionInfo rightInfo = arithmeticExpression();
            info = ExpressionInfo(DataType::INT, rightInfo.isConstant() && info.isConstant()); //Type is always int after relation

            if(t.is(TT_OPERATOR,"=="))
            {
                writeASM("COMPARE");

            }else if(t.is(TT_OPERATOR,"<"))
            {
                writeASM("LESS");

            }else if(t.is(TT_OPERATOR,">"))
            {
                writeASM("GREATER");

            }else if(t.is(TT_OPERATOR,"<="))
            {
                writeASM("LESSEQUAL");

            }else if(t.is(TT_OPERATOR,">="))
            {
                writeASM("GREATEREQUAL");
            }else if(t.is(TT_OPERATOR,"!="))
            {
                writeASM("COMPARE");
                writeASM("INVERT");
            }

            t = lexer->peekToken();
        }

        return info;
    }

    ExpressionInfo Compiler::arithmeticExpression()
    {
    	ExpressionInfo info = term();

        Token t = lexer->peekToken();
        while(isAddOp(t))
        {
            lexer->readToken();

            ExpressionInfo rightInfo = term();

            if(t.is(TT_OPERATOR,"+"))
            {
            	if(info.getType().equals(DataType::STRING) || rightInfo.getType().equals(DataType::STRING))
            	{
            		info = ExpressionInfo(DataType::STRING, info.isConstant() && rightInfo.isConstant());

            	}else
            	{
            		if(!info.getType().isNumeric())
					{
						error(String("Left hand side of operator + is of non-numeric type(") + info.getType().getTypeName() + "), but + requires numeric types on both sides or at least one string type", t.getLine());
					}
					if(!rightInfo.getType().isNumeric())
					{
						error(String("Right hand side of operator + is of non-numeric type(") + rightInfo.getType().getTypeName() + "), but + requires numeric types on both sides or at least one string type", t.getLine());
					}

					info = ExpressionInfo(DataType::mathCast(info.getType(),rightInfo.getType()), info.isConstant() && rightInfo.isConstant());
            	}

                writeASM("ADD");

            }else if(t.is(TT_OPERATOR,"-"))
            {

            	if(!info.getType().isNumeric())
				{
					error(String("Left hand side of operator - is of non-numeric type(") + info.getType().getTypeName() +"), but - requires numeric types on both sides", t.getLine());
				}
				if(!rightInfo.getType().isNumeric())
				{
					error(String("Right hand side of operator - is of non-numeric type(") + rightInfo.getType().getTypeName() + "), but - requires numeric types on both sides", t.getLine());
				}

            	info = ExpressionInfo(DataType::mathCast(info.getType(),rightInfo.getType()), info.isConstant() && rightInfo.isConstant());

                writeASM("SUBSTRACT");
            }

            t = lexer->peekToken();
        }

        return info;
    }

    ExpressionInfo Compiler::term()
    {
    	ExpressionInfo info = castFactor();

        Token t = lexer->peekToken();
        while(isMultiplyOp(t))
        {
            lexer->readToken();

            ExpressionInfo rightInfo = castFactor();

            if(!info.getType().isNumeric())
            {
            	error(String("Left hand side of operator ") + t.getLexem() + " is of non-numeric type(" + rightInfo.getType().getTypeName() + "), but " + t.getLexem() + " requires numeric types", t.getLine());
            }
            if(!rightInfo.getType().isNumeric())
			{
				error(String("Right hand side of operator ") + t.getLexem() + " is of non-numeric type(" + rightInfo.getType().getTypeName() + "), but " + t.getLexem() + " requires numeric types", t.getLine());
			}

            info = ExpressionInfo(DataType::mathCast(info.getType(),rightInfo.getType()), info.isConstant() && rightInfo.isConstant());

            if(t.is(TT_OPERATOR,"*"))
            {
                writeASM("MULTIPLY");

            }else if(t.is(TT_OPERATOR,"/"))
            {
                writeASM("DIVIDE");
            }

            t = lexer->peekToken();
        }

        return info;
    }

    ExpressionInfo Compiler::castFactor()
	{
		ExpressionInfo info = signedFactor();

		Token t = lexer->peekToken();
		if(t.is(TT_OPERATOR,"->"))
		{
			lexer->readToken();

			t = lexer->readToken();

			if(!isDatatype(t))
			{
				error("Expected data type after cast operator, but found: " + t.getLexem(), t.getLine());
			}

			writeASM(String("TO") + t.getLexem().toUpperCase());

			return ExpressionInfo(DataType::byName(t.getLexem()), false);
		}

		return info;
	}

    ExpressionInfo Compiler::signedFactor()
	{
		bool negate = false;
		bool invert = false;

		Token t = lexer->peekToken();
		if(t.is(TT_OPERATOR,"-"))
		{
			negate = true;
			lexer->readToken();

		}else if(t.is(TT_OPERATOR,"!"))
		{
			invert = true;
			lexer->readToken();
		}

		ExpressionInfo info = memberFactor();

		if(negate)
		{
			if(!info.getType().isNumeric())
			{
				error(String("Negation operator is applicable on numeric types only. Given type is: ") + info.getType().getTypeName() ,t.getLine());
			}

			writeASM("NEGATE");
		}

		if(invert)
		{
			if(!info.getType().isNumeric())
			{
				error(String("Inversion operator is applicable on numeric types only. Given type is: ") + info.getType().getTypeName() ,t.getLine());
			}

			writeASM("INVERT");

			info = ExpressionInfo(DataType::INT, info.isConstant());
		}

		return info;
	}

    ExpressionInfo Compiler::memberFactor()
    {
        ExpressionInfo info = factor();

        Token t = lexer->peekToken();
        while(t.is(TT_OPERATOR,"."))
        {
            lexer->readToken();
            t = lexer->readToken();
            if(t.getType() != TT_IDENT)
            {
                error(String("Expected identifier after member directive, but found: ") + t.getLexem(), t.getLine());
            }

            Token member = t;

            t = lexer->peekToken();
            if(t.is(TT_OPERATOR,"("))
            {

                info = functionCall(member, true);

            }else
            {
                info = ExpressionInfo(DataType::OBJECT, false); //TODO: Get correct type of member

                writeASM("GETMEMBER '" + member.getLexem() + "'");
            }
        }

        return info;
    }

    ExpressionInfo Compiler::factor()
    {
        Token t = lexer->readToken();

        if(t.getType() == TT_NUMBER)
        {
            DataType numberType = getTypeOfNumberString(t.getLexem());

            if(numberType.equals(DataType::NOTYPE))
            {
                error("Number literal '" + t.getLexem() + "' does not fit in any numerical data type",t.getLine());
            }

            writeASM("PUSH" + numberType.getTypeName().toUpperCase() + " " + t.getLexem());

            return ExpressionInfo(numberType, true);

        }else if(t.getType() == TT_STRING)
        {
            writeASM("PUSHSTRING '" + escapeASMChars(t.getLexem()) + "'");

           return ExpressionInfo(DataType::STRING, true);

        }else if(t.getType() == TT_IDENT)
        {
            Token t2 = lexer->peekToken();
            if(t2.is(TT_OPERATOR,"("))
            {
                return functionCall(t,false);

            }else
            {
                /*Variable *v = scriptSystem.getVariable(t.getLexem());

                if(v == NULL)
                {
                    error("Variable '" + t.getLexem + "' was not declared in this scope!",t.getLine());
                }*/

                writeASM("PUSHVAR '" + t.getLexem() + "'");

                return ExpressionInfo(DataType::INT, false);

            	//TODO: get variable and stuff
            }

        }else if(t.is(TT_OPERATOR,"("))
        {
            ExpressionInfo info = expression();

            Token t2 = lexer->readToken();
            if(!t2.is(TT_OPERATOR,")"))
            {
                error("Missing closing parentheses after expression",t.getLine());
            }

            return info;

        }else if(t.is(TT_KEYWORD,"new"))
        {
            //TODO: implement NEW here
        }else if(t.is(TT_KEYWORD,"null"))
        {
            writeASM("PUSHNULL");

            return ExpressionInfo(DataType::OBJECT, true); //TODO: Re-think if NULL is really an object

        }else if(t.getType() == TT_EOF)
        {
            error("Unexpected end of file while parsing expression", t.getLine());

        }else
        {
            error("Unexpected token in expression factor: " + t.getLexem(), t.getLine());
        }

        return ExpressionInfo(DataType::NOTYPE, false);
    }

    DataType Compiler::getTypeOfNumberString(String s)
    {
    	bool fp = (s.indexOf('.') != -1);

    	//TODO: Consider big types here
    	return fp ? DataType::FLOAT : DataType::INT;
    }

    bool Compiler::isAddOp(Token t)
    {
    	return t.is(TT_OPERATOR, "+") || t.is(TT_OPERATOR, "-");
    }

    bool Compiler::isMultiplyOp(Token t)
    {
    	return t.is(TT_OPERATOR, "*") || t.is(TT_OPERATOR, "/");
    }

    bool Compiler::isLogicOp(Token t)
    {
    	return t.is(TT_OPERATOR, "|") || t.is(TT_OPERATOR, "&");
    }

    bool Compiler::isRelationalOp(Token t)
    {
    	return t.is(TT_OPERATOR, "==") || t.is(TT_OPERATOR, "!=") || t.is(TT_OPERATOR, ">") || t.is(TT_OPERATOR, "<") || t.is(TT_OPERATOR, ">=") || t.is(TT_OPERATOR, "<=");
    }

    bool Compiler::isDatatype(Token t)
    {
    	if(t.getType() != TT_KEYWORD)
    	{
    		return false;
    	}

    	for(uint16_t i = 0; i < datatypes.size(); i++)
    	{

    		if(t.getLexem().equals(datatypes[i]))
    		{
    			return true;
    		}
    	}

    	return false;
    }

    String Compiler::escapeASMChars(String s)
    {
    	//TODO: Do useful stuff here

    	return s;
    }

    void Compiler::writeASM(String s)
    {
    	std::cout << s << std::endl;
    }

    void Compiler::error(String s, int line)
    {
    	//TODO: Do stuff with the error message and throw exception
    	throw Exception(String("Error in line ") + line + ": " + s);
    }


    //private class ExpressionInfo

    ExpressionInfo::ExpressionInfo(DataType t, bool cons) : type(t), constant(cons)
    {
    	//this->type = type;
    	//this->constant = constant;
    }

    bool ExpressionInfo::isConstant()
    {
    	return constant;
    }

    DataType ExpressionInfo::getType()
    {
    	return type;
    }
}
