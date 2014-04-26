
#ifndef COMPILER_H_
#define COMPILER_H_

#include <istream>

#include "Nein.h"

#include "Compiler.h"
#include "ScriptSystem.h"
#include "Lexer.h"

using std::istream;

namespace Scales
{

    class Compiler
    {
    public:

        Compiler();

        char *compile(istream in);

    private:

        Lexer *lexer;
        PrototypeScriptSystem scriptSystem;

        DataType expression();
        DataType relationalExpression();
        DataType arithmeticExpression();
        DataType term();
        DataType signedFactor();
        DataType castFactor();
        DataType memberFactor();
        DataType factor();

        DataType functionCall(Token ident, bool member);

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
