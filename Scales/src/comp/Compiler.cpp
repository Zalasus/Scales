
#include "comp/Compiler.h"
#include "DataType.h"
#include "ScriptSystem.h"

#include <iostream>

namespace Scales
{

    Compiler::Compiler()
    {


    }

    char* Compiler::compile(istream in)
    {
        lexer = new Lexer(in);

        char *bytecode = new char[10]; //Put stuff in here
        //start parsing here

        //Free resources after parsing
        delete lexer;

        return bytecode;
    }

    DataType Compiler::functionCall(Token ident, bool member)
    {
    	//TODO: Do usefull stuff here

    	return DataType::NULL_TYPE;
    }

    DataType Compiler::expression()
    {
        DataType leftType = relationalExpression();
        DataType rightType;

        Token t = lexer->peekToken();
        while(isLogicOp(t))
        {
            if(!leftType.isNumeric())
            {
                //TODO: Add informatation on operator and type using string concat here
                error("Logical operator" + t.getLexem() + " is not applicable with type " + leftType.getTypeName(),t.getLine());
            }

            lexer->readToken();

            rightType = relationalExpression();

            if(t.is(TT_OPERATOR,"|"))
            {
                writeASM("LOGICOR");

            }else if(t.is(TT_OPERATOR,"&"))
            {
                writeASM("LOGICAND");
            }

            leftType = DataType::INT;

            t = lexer->peekToken();
        }

        if(rightType.equals(DataType::NULL_TYPE)) //If there was no compare operation, the expression has the type of the left statement
        {
            return leftType;

        }else //If there was a compare operation, the type is always INT
        {
            return DataType::INT;
        }
    }

    DataType Compiler::relationalExpression()
    {
        DataType leftType = arithmeticExpression();
        DataType rightType;

        Token t = lexer->peekToken();
        while(isRelationalOp(t))
        {
            lexer->readToken();

            rightType = arithmeticExpression();

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

        if(rightType.equals(DataType::NULL_TYPE)) //If there was no compare operation, the expression has the type of the left statement
        {
            return leftType;

        }else //If there was a compare operation, the type is always INT
        {
            return DataType::INT;
        }
    }

    DataType Compiler::arithmeticExpression()
    {
        DataType type = term();

        Token t = lexer->peekToken();
        while(isAddOp(t))
        {
            lexer->readToken();

            type = DataType::mathCast(type, term());

            if(t.is(TT_OPERATOR,"+"))
            {
                writeASM("ADD");
            }else if(t.is(TT_OPERATOR,"-"))
            {
                writeASM("SUBSTRACT");
            }

            t = lexer->peekToken();
        }

        return type;
    }

    DataType Compiler::term()
    {
        DataType type = signedFactor();

        Token t = lexer->peekToken();
        while(isMultiplyOp(t))
        {
            lexer->readToken();

            type = DataType::mathCast(type, memberFactor());

            if(t.is(TT_OPERATOR,"*"))
            {
                writeASM("MULTIPLY");

            }else if(t.is(TT_OPERATOR,"/"))
            {
                writeASM("DIVIDE");
            }

            t = lexer->peekToken();
        }

        return type;
    }

    DataType Compiler::signedFactor()
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

        DataType type = castFactor();

        if(negate)
        {
            if(!type.isNumeric())
            {
                error("Negation operator is applicable on numeric types only!",t.getLine());
            }

            writeASM("NEGATE");
        }

        if(invert)
        {
            if(!type.isNumeric())
            {
                error("Inversion operator is applicable on numeric types only!",t.getLine());
            }

            writeASM("INVERT");

            type = DataType::INT;
        }

        return type;
    }

    DataType Compiler::castFactor()
    {
    	DataType type = memberFactor();

    	Token t = lexer->peekToken();
    	if(t.is(TT_OPERATOR,"->"))
    	{
    		lexer->readToken();

    		t = lexer->readToken();

    		if(!isDatatype(t))
    		{
    			error("Expected data type after cast operator. Found " + t.getLexem(), t.getLine());
    		}

    		writeASM(String("TO") + t.getLexem().toUpperCase());

    		return DataType::byName(t.getLexem());
    	}

    	return type;
    }

    DataType Compiler::memberFactor()
    {
        DataType type = factor();

        Token t = lexer->peekToken();
        while(t.is(TT_OPERATOR,"."))
        {
            lexer->readToken();
            t = lexer->readToken();
            if(t.getType() != TT_IDENT)
            {
                error(String("Expected identifier after member directive. Found: ") + t.getLexem(), t.getLine());
            }

            Token member = t;

            t = lexer->peekToken();
            if(t.is(TT_OPERATOR,"("))
            {

                type = functionCall(member, true);

            }else
            {
                type = DataType::OBJECT; //TODO: Get correct type of member

                writeASM("GETMEMBER '" + member.getLexem() + "'");
            }
        }

        return type;
    }

    DataType Compiler::factor()
    {
        DataType type;

        Token t = lexer->readToken();
        if(t.getType() == TT_NUMBER)
        {
            DataType numberType = getTypeOfNumberString(t.getLexem());

            if(numberType.equals(DataType::NULL_TYPE))
            {
                error("Number literal '" + t.getLexem() + "' does not fit in any numerical data type.",t.getLine());
            }

            writeASM("PUSH" + numberType.getTypeName() + " " + t.getLexem());

            type = numberType;

        }else if(t.getType() == TT_STRING)
        {
            writeASM("PUSHSTRING '" + escapeASMChars(t.getLexem()) + "'");

            type = DataType::STRING;

        }else if(t.getType() == TT_IDENT)
        {
            Token t2 = lexer->peekToken();
            if(t2.is(TT_OPERATOR,"("))
            {
                type = functionCall(t,false);

            }else
            {
                /*Variable *v = scriptSystem.getVariable(t.getLexem());

                if(v == NULL)
                {
                    error("Variable '" + t.getLexem + "' was not declared in this scope!",t.getLine());
                }

                writeASM("PUSHVAR '" + t.getLexem() + "'");

                type = &v->getType();*/

            	//TODO: get variable and stuff
            }

        }else if(t.is(TT_OPERATOR,"("))
        {
            type = expression();

            Token t2 = lexer->readToken();
            if(!t.is(TT_OPERATOR,")"))
            {
                error("Missing closing parentheses after expression!",t.getLine());
            }

        }else if(t.is(TT_KEYWORD,"NEW"))
        {
            //TODO: implement NEW here
        }else if(t.is(TT_KEYWORD,"NULL"))
        {
            writeASM("PUSHNULL");

            type = DataType::OBJECT; //TODO: Re-think if NULL is really an object

        }else
        {
            error("Unexpected token in expression factor: " + t.getLexem(), t.getLine());
        }

        return type;
    }

    DataType Compiler::getTypeOfNumberString(String s)
    {
    	//TODO: Do usefull stuff here

    	return DataType::NULL_TYPE;
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

    	return t.getLexem().equals("int") || t.getLexem().equals("long") || t.getLexem().equals("double") || t.getLexem().equals("float") ||
    			t.getLexem().equals("string") || t.getLexem().equals("object");
    }

    String Compiler::escapeASMChars(String s)
    {
    	//TODO: Do usefull stuff here

    	return s;
    }

    void Compiler::writeASM(String s)
    {
    	std::cout << s << std::endl;
    }

    void Compiler::error(String s, int line)
    {
    	//TODO: Do stuff with the error message and throw exception
    }

}
