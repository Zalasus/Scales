
#ifndef COMPILER_H_
#define COMPILER_H_

#include <istream>

#include "Nein.h"

#include "DataType.h"
#include "Compiler.h"
#include "ScriptSystem.h"
#include "Lexer.h"
#include "Exception.h"

using std::istream;

namespace Scales
{

	class ExpressionInfo
    {

    public:

    	ExpressionInfo(DataType t, bool cons);

    	bool isConstant();

    	DataType getType();


    private:

    	DataType type;

    	bool constant;

    };


    class Compiler
    {
    public:

        Compiler(istream &in);

        ~Compiler();

        void compile();

    private:

        Lexer *lexer;
        PrototypeScriptSystem scriptSystem;

        vector<String> keywords;
        vector<String> datatypes;
        vector<String> operators;

        ExpressionInfo expression();
        ExpressionInfo relationalExpression();
        ExpressionInfo arithmeticExpression();
        ExpressionInfo term();
        ExpressionInfo castFactor();
        ExpressionInfo signedFactor();
        ExpressionInfo memberFactor();
        ExpressionInfo factor();

        ExpressionInfo functionCall(Token ident, bool member);

        DataType getTypeOfNumberString(String numberString);

        String escapeASMChars(String s);

        bool isLogicOp(Token t);
        bool isRelationalOp(Token t);
        bool isAddOp(Token t);
        bool isMultiplyOp(Token t);

        bool isDatatype(Token t);

        void writeASM(String line);

        void error(String message, int line);
    };

}

#endif
