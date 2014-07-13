
#ifndef COMPILER_H_
#define COMPILER_H_

#include <istream>

#include "ScalesSystem.h"
#include "ScalesUtil.h"

#include "DataType.h"
#include "Compiler.h"
#include "Lexer.h"
#include "Exception.h"
#include "ASMStream.h"

using std::istream;

namespace Scales
{

	class BlockInfo
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

		BlockInfo(BlockType pType, uint32_t pUid, uint32_t pNid, uint32_t pRid);

		uint32_t getUID() const;
		uint32_t getNestID() const;
		uint32_t getRowID() const;

		BlockType getBlockType() const;

	private:

		BlockType type;
		uint32_t uid;
		uint32_t nid;
		uint32_t rid;
	};

	class ExpressionInfo
    {
    public:

		enum FactorType
		{
			FT_VARIABLE_REF,
			FT_FUNCTION_RETURN,
			FT_LITERAL,
			FT_MATH_EXPR

		};

    	ExpressionInfo(const DataType &dtype, const bool cons, const FactorType ftype);

    	bool isConstant();

    	DataType getType();

    	FactorType getFactorType();


    private:

    	DataType dataType;

    	bool constant;

    	FactorType factorType;

    };


    class Compiler
    {
    public:

        Compiler(istream &in, ScalesSystem *ss);

        ~Compiler();

        void compile();

    private:

        Lexer lexer;

        ScalesSystem *scalesSystem;
        ClassPrototype *currentClass;
        FunctionPrototype *currentFunction;

        uint32_t lastUID;

        void mainBlock();
        void classDef(const String &nspace);
        void functionDec(bool priv, bool native);
        BlockInfo::BlockType block(const BlockInfo &blockInfo);

        void ifStatement(const BlockInfo &blockInfo, uint32_t &blocksInThisBlock);

        void usingStatement();

        void variableDec(bool priv, bool native, bool local);

        void leftEval();
        DataType rightEval();

        ExpressionInfo expression(const bool leftEval = false);
        ExpressionInfo relationalExpression(const bool leftEval = false);
        ExpressionInfo arithmeticExpression(const bool leftEval = false);
        ExpressionInfo term(const bool leftEval = false);
        ExpressionInfo castFactor(const bool leftEval = false);
        ExpressionInfo signedFactor(const bool leftEval = false);
        ExpressionInfo memberFactor(const bool leftEval = false);
        ExpressionInfo factor(const bool leftEval = false);

        DataType functionCall(const String &funcName, bool member, const DataType &baseType = DataType::NOTYPE);

        DataType dataType();
        ClassId classId();

        DataType getTypeOfNumberString(const String &numberString);

        String escapeASMChars(const String &s);

        uint32_t getNewUID();

        VariablePrototype *getVariableInClass(Class *s, const String &name);
        void writeDatatypeToBytecode(const DataType &t);

        bool isLogicOp(const Token &t);
        bool isRelationalOp(const Token &t);
        bool isAddOp(const Token &t);
        bool isMultiplyOp(const Token &t);
        bool isAssignmentOperator(const Token &t);

        bool isAccessModifier(const Token &t);

        bool isPrimitive(const Token &t);
        bool isTypeOrNamespace(const String &t);

        void error(const String &message, int line);


        ASMStream asmout;

        static const String KEYWORDS[];
        static const uint32_t KEYWORD_COUNT;

        static const String DATATYPES[];
        static const uint32_t DATATYPE_COUNT;

        static const String OPERATORS[];
        static const uint32_t OPERATOR_COUNT;
    };

}

#endif
