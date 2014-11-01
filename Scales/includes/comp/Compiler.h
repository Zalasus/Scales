
namespace Scales
{
	class ExpressionInfo;
	class Compiler;
}

#ifndef COMPILER_H_
#define COMPILER_H_

#include "ScalesSystem.h"
#include "ScalesLibrary.h"
#include "ScalesUtil.h"
#include "ScalesType.h"
#include "ScalesVariable.h"
#include "Lexer.h"
#include "ScalesException.h"
#include "ASMStream.h"

#include <istream>

namespace Scales
{

	enum BlockType
	{
		BT_FUNC,
		BT_IF,
		BT_ELSEIF,
		BT_ELSE,
		BT_WHILE,
		BT_SUB
	};

	class ExpressionInfo
    {
    public:

		enum FactorType
		{
			FT_VARIABLE_REF,
			FT_FUNCTION_RETURN,
			FT_VOID,
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

        Compiler();
        ~Compiler();

        Library compile(std::istream *in, ScalesSystem *ss);

    private:

        ScalesSystem *scalesSystem;
        ClassSketch *currentClassProto;
        FunctionSketch *currentFunctionProto;

        uint32_t lastUID;

        Lexer lexer;


        void mainBlock(Library &lib);
        void classDef(const String &nspace);
        void functionDec(uint32_t funcFlags);
        /**
         * @returns The type of block that should start right after conclusion of this block (used for nested if block handling)
         */
        BlockType block(BlockType blockType, const Scope &scope);

        /**
         * @param blockType This is the BlockType of the the block the if is encountered in. NOT of the if block itself.
         */
        void ifStatement(const BlockType &blockType, const Scope &scope, uint32_t &blocksInThisBlock);

        void usingStatement();

        void variableDec(uint32_t varFlags, const Scope &currentScope);

        void leftEval();

        ExpressionInfo expression(const bool mustYieldResult);
        ExpressionInfo logicExpression();
        ExpressionInfo relationalExpression();
        ExpressionInfo arithmeticExpression();
        ExpressionInfo term();
        ExpressionInfo castFactor();
        ExpressionInfo signedFactor();
        ExpressionInfo memberFactor();
        ExpressionInfo indexFactor();
        ExpressionInfo factor();

        ExpressionInfo staticExpression();

        DataType functionCall(const String &funcName, bool member, const DataType &baseType = DataType(DataType::DTB_NOTYPE));

        DataType dataType();
        ClassId classId();
        Token name();

        DataType getTypeOfNumberString(const String &numberString);

        String escapeASMChars(const String &s);

        uint32_t getNewUID();

        bool namespaceExists(const String &nspace);
        /**
         * Searches the library beeing built right now and the system for a specific prototype.
         */
        ClassPrototype *lookupClassPrototype(const ClassId &cid);
        /**
         * Searches for a specific classname in the following namespaces: usage list -> current namespace -> default namespace
         */
        ClassPrototype *lookupClassPrototypeByName(const String &name);
        /**
         * Searches for a specific variable in the following scopes: local -> global -> static
         */
        VariableSketch *lookupVariablePrototype(const String &name);

        void writeDatatypeToBytecode(const DataType &t);

        bool isLogicOp(const Token &t);
        bool isRelationalOp(const Token &t);
        bool isAddOp(const Token &t);
        bool isMultiplyOp(const Token &t);
        bool isAssignmentOperator(const Token &t);

        bool isModifier(const Token &t);
        bool isAccessModifier(const Token &t);

        bool isPrimitive(const Token &t);

        void error(const String &message, int line);


        int32_t parseInt(const String &s);
        int64_t parseLong(const String &s);
        float parseFloat(const String &s);
        double parseDouble(const String &s);

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
