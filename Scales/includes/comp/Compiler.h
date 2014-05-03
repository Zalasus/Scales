
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

	class BlockIdent
	{
	public:

		enum BlockType
		{
			BT_MAIN,
			BT_FUNC,
			BT_IF,
			BT_ELSEIF,
			BT_ELSE,
			BT_WHILE,
			BT_SUB
		};

		BlockIdent(BlockType pType, uint32_t pUid);

		uint32_t getUID() const;

		BlockType getBlockType() const;

	private:

		BlockType type;
		uint32_t uid;
	};

	class ExpressionInfo
    {

    public:

    	ExpressionInfo(const DataType &t, bool cons);

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

        ScriptSystem scriptSystem;
        Script *currentScript;

        uint32_t lastUID;

        vector<String> keywords;
        vector<String> datatypes;
        vector<String> operators;

        void mainBlock();
        void script(const String &name, bool staticScript);
        BlockIdent::BlockType functionBlock(const BlockIdent &block, const DataType &returnType);

        void ifStatement(const DataType &returnType);

        void leftEval(const Token &firstIdent);
        DataType rightEval();

        void functionDec(const AccessType &accessType, bool native, const DataType &returnType, bool event);
        void constructorDec(const AccessType &accessType);

        DataType dataType(const Token &type);

        ExpressionInfo expression();
        ExpressionInfo relationalExpression();
        ExpressionInfo arithmeticExpression();
        ExpressionInfo term();
        ExpressionInfo castFactor();
        ExpressionInfo signedFactor();
        ExpressionInfo memberFactor();
        ExpressionInfo factor();

        DataType functionCall(const Token &ident, bool member, const DataType &baseType = DataType::NOTYPE);

        DataType getTypeOfNumberString(const String &numberString);

        String escapeASMChars(const String &s);

        uint32_t getNewUID();

        bool isLogicOp(const Token &t);
        bool isRelationalOp(const Token &t);
        bool isAddOp(const Token &t);
        bool isMultiplyOp(const Token &t);
        bool isAssignmentOperator(const Token &t);

        bool isAccessModifier(const Token &t);

        bool isDatatype(const Token &t);

        void writeASM(const String &line);

        void error(const String &message, int line);
    };

}

#endif
