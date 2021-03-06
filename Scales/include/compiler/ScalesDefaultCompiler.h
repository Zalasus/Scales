
#ifndef COMPILER_H_
#define COMPILER_H_

#include "ScalesRoot.h"
#include "ScalesString.h"
#include "ScalesType.h"
#include "compiler/ScalesLexer.h"
#include "compiler/ScalesCompiler.h"
#include "compiler/ScalesASMStream.h"

#include <istream>

namespace Scales
{

	//Helper classes for the compiler

	class BlockInfo
	{
	public:

		enum blockType_t
		{
			BT_FUNC,
			BT_IF,
			BT_ELSEIF,
			BT_ELSE,
			BT_WHILE,
			BT_DOWHILE,
			BT_SUB
		};

		BlockInfo(uint32_t pLocalCount, uint32_t pStackSize, blockType_t pFollowingBlock);

		uint32_t getLocalCount() const;
		uint32_t getStackSize() const;
		blockType_t getFollowingBlock() const;

	private:

		uint32_t localCount;
		uint32_t stackSize;
		blockType_t followingBlock;

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

	class Scope
	{
	public:
		Scope(uint32_t pNestId, uint32_t pRowId, uint32_t pUniqueId);

		uint32_t getNestId() const;
		uint32_t getRowId() const;
		uint32_t getUniqueId() const;

		bool isVisibleIn(const Scope &s) const;
		bool isGlobal() const;

		static const Scope GLOBAL;

	private:

		uint32_t nestId;
		uint32_t rowId;
		uint32_t uniqueId;
	};

	class Local
	{
	public:

		Local(const String &pName, const DataType &pType, const Scope &pScope, uint32_t pIndex);

		String getName() const;
		DataType getType() const;
		Scope getScope() const;
		uint32_t getIndex() const;

	private:

		String name;
		DataType type;
		Scope scope;
		uint32_t index;
	};


	//END helper classes for the compiler

    class DefaultCompiler : public Compiler
    {
    public:

		DefaultCompiler(Root *pRoot, compilerFlags_t flags);
        ~DefaultCompiler();

        size_t compile(std::istream *in);
        std::vector<const Class*> listGeneratedClasses();

    private:

        Root *root;

        Class *currentClass;
        Function *currentFunction;

        uint32_t lastUID;
        uint32_t currentGlobalStackMax;
        uint32_t currentFunctionStackMax;

        Lexer lexer;

        std::vector<const Class*> classList; // each time a compilation is started, all generated classes are stored here
        std::vector<Local> locals; // This is only used for compile time type checking and counting. During runtime, locals are created by the bytecode and only their count is needed.
        std::vector<const Class*> importList;

        ASMStream asmout;


        void mainBlock();
        void classDef(const String &nspace);
        void functionDec(uint32_t funcFlags);
        /**
         * @returns The type of block that should start right after conclusion of this block (used for nested if block handling)
         */
        BlockInfo block(BlockInfo::blockType_t blockType, const Scope &scope);

        /**
         * @param blockType This is the BlockType of the the block the if is encountered in. NOT of the if block itself.
         */
        BlockInfo ifStatement(const BlockInfo::blockType_t &blockType, const Scope &scope, uint32_t &blocksInThisBlock);

        void usingStatement();

        void variableDec(uint32_t varFlags, const Scope &currentScope);

        void leftEval(const Scope &scope);

        ExpressionInfo expression(const bool valueContext, const Scope &expressionScope);
        ExpressionInfo logicExpression(const Scope &expressionScope);
        ExpressionInfo relationalExpression(const Scope &expressionScope);
        ExpressionInfo arithmeticExpression(const Scope &expressionScope);
        ExpressionInfo term(const Scope &expressionScope);
        ExpressionInfo castFactor(const Scope &expressionScope);
        ExpressionInfo signedFactor(const Scope &expressionScope);
        ExpressionInfo memberFactor(const Scope &expressionScope);
        ExpressionInfo indexFactor(const Scope &expressionScope);
        ExpressionInfo factor(const Scope &expressionScope);

        TypeList parameterList(const Scope &scope);
        DataType functionCall(const String &funcName, bool member, const Scope &scope, const DataType &baseType = DataType(DataType::DTB_VOID));

        DataType dataType();
        ClassID classID();
        Token name();

        DataType getTypeOfNumberString(const String &numberString);

        String escapeASMChars(const String &s);

        uint32_t getNewUID();
        void incrementStackSize(uint32_t i = 1);
        void decrementStackSize(uint32_t i = 1);

        bool namespaceExists(const String &nspace);

        /**
         * Searches the local memory for a local with given name and returns it's pointer if it is visible in the given scope.
         * Otherwise, nullptr is returned.
         *
         * \returns Found local or nullptr
         */
        Local *lookupLocal(const String &pName, const Scope &pScope);

        /**
		 * Searches for a specific class. If the namespace is empty, the following namespaces are tried: usage list -> current namespace -> default namespace
		 */
		const Class *lookupClass(const ClassID &name);

        /**
         * Searches for a class with a specific classname in the following namespaces: usage list -> current namespace -> default namespace
         */
        const Class *lookupClassByName(const String &name);

        void writeDatatypeToBytecode(const DataType &t);

        bool isLogicOp(const Token &t);
        bool isRelationalOp(const Token &t);
        bool isAddOp(const Token &t);
        bool isMultiplyOp(const Token &t);
        bool isAssignmentOperator(const Token &t);

        bool isModifier(const Token &t);
        bool isAccessModifier(const Token &t);

        bool isPrimitive(const Token &t);

        bool isStartOfExpression(const Token &t);

        void error(const String &message, int line);


        static const String KEYWORDS[];
        static const uint32_t KEYWORD_COUNT;

        static const String DATATYPES[];
        static const uint32_t DATATYPE_COUNT;

        static const String OPERATORS[];
        static const uint32_t OPERATOR_COUNT;
    };

}

#endif
