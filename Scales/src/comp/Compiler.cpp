
#include "comp/Compiler.h"

#include <iostream>

namespace Scales
{

	//TODO: Performance optimization!!! This whole code looks like its your first C++ project (which is kind of true). And try to reduce executable size. We passed 1MB already and that was in v0.2
	//(okay, I have to admit, the 1MB were in Debug configuration. In Release it's just about 200kB. But anyway: Keep it small! 847kB at max!)

	//public class compiler

    Compiler::Compiler(istream &in)
    {
    	lexer = new Lexer(in, KEYWORDS, KEYWORD_COUNT, OPERATORS, OPERATOR_COUNT, true);

    	currentScript = null;

    	lastUID = 0;
    }

    Compiler::~Compiler()
    {
    	delete lexer;
    }

    void Compiler::compile()
    {
    	mainBlock();


    	//ExpressionInfo info = expression();

    	//std::cout << "The type of that expression is " << info.getType().getTypeName() << " and it is " << (info.isConstant() ? "constant" : "not constant") << std::endl;
    }

    void Compiler::mainBlock()
    {
    	String nspace = "";

    	Token t = lexer->readToken();

		while(t.getType() != Token::TT_EOF)
		{
			if(t.is(Token::TT_KEYWORD,"static") || t.is(Token::TT_KEYWORD,"script"))
			{
				bool staticScript = false;

				if(t.is(Token::TT_KEYWORD,"static"))
				{
					staticScript = true;

					t = lexer->readToken();

					if(!t.is(Token::TT_KEYWORD,"script"))
					{
						error("Expected 'script' token after static keyword", t.getLine());
					}
				}

				Token ident = lexer->readToken();

				if(ident.getType() != Token::TT_IDENT)
				{
					error("Expected scriptname after 'script' keyword, but found: " + ident.getLexem(),ident.getLine());
				}

				script(ident.getLexem(), staticScript);

			}else if(t.is(Token::TT_KEYWORD,"namespace"))
			{
				t = lexer->readToken();

				if(t.is(Token::TT_KEYWORD, "default"))
				{
					nspace = String("");

				}else if(t.getType() == Token::TT_IDENT)
				{
					nspace = t.getLexem();

				}else
				{
					error("Expected namespace identifier or 'default' after namespace keyword, but found: " + t.getLexem(), t.getLine());
				}

				t = lexer->readToken();

				if(!t.is(Token::TT_OPERATOR,";"))
				{
					error("Expected semicolon after namespace declaration, but found: " + t.getLexem(), t.getLine());
				}

				std::cout << "Namespace set to: " << nspace << std::endl;

			}else
			{
				error("Expected script header or namespace definition, but found: " + t.getLexem(), t.getLine());
			}

			t = lexer->readToken();
		}
    }

    void Compiler::script(const String &name, bool staticScript)
    {
    	Script scr = Script(name, staticScript);
    	scriptSystem.declareScript(scr);

    	currentScript = scriptSystem.getScript(name);

    	Token t = lexer->peekToken();

    	std::cout << "-------Script " << name << " start " << std::endl;

    	while(!t.is(Token::TT_KEYWORD,"end"))
    	{
    		if(isAccessModifier(t) || t.is(Token::TT_KEYWORD,"native") || t.is(Token::TT_KEYWORD, "func") || t.is(Token::TT_KEYWORD,"event") || t.is(Token::TT_KEYWORD,"init") || isDatatype(t))
    		{
    			lexer->readToken();

    			AccessType accessType = AccessType::PRIVATE;
    			bool explicitType = false;
    			bool native = false;

    			if(isAccessModifier(t))
    			{
    				accessType = AccessType::byName(t.getLexem());

    				explicitType = true;

    				t = lexer->readToken();
    			}

    			if(t.is(Token::TT_KEYWORD, "native"))
    			{
    				native = true;

    				t = lexer->readToken();
    			}


    			if(t.is(Token::TT_KEYWORD,"func"))
    			{
    				t = lexer->readToken();

    				if(isDatatype(t) || t.is(Token::TT_KEYWORD,"void"))
    				{
    					DataType returntype = t.is(Token::TT_KEYWORD,"void") ? DataType::NOTYPE : dataType(t);

    					functionDec(accessType, native, returntype, false);

    				}else
    				{
    					error("Expected data type or 'void' after func keyword, but found: " + t.getLexem(), t.getLine());
    				}

    			}else if(t.is(Token::TT_KEYWORD, "init"))
				{
					if(native)
					{
						//TODO: Native constructors would be quite useful, in fact

						error("Constructors must not be native (This will change. Wait for it)", t.getLine());
					}

					constructorDec(accessType);

				}else if(t.is(Token::TT_KEYWORD,"event"))
    			{
    				if(explicitType && !accessType.equals(AccessType::PUBLIC))
    				{
    					error(String("Events can not be declared ") + accessType.getTypeName() + ". They must be declared public or with implicit access type",t.getLine());
    				}

    				//TODO: Specifiers for object types have to be included in bytecode

    				functionDec(AccessType::PUBLIC,native,DataType::NOTYPE,true);

    				//TODO: Implement events & make this whole function less fuzzy

    			}else if(isDatatype(t))
    			{
    				DataType type = dataType(t);

    				Token ident = lexer->readToken();

					if(ident.getType() != Token::TT_IDENT)
					{
						error("Expected variable name after data type, but found: " + ident.getLexem(), ident.getLine());
					}

					t = lexer->readToken();

					//TODO: write to script constructors bytecode here
					writeASM("DECLAREVAR '" + ident.getLexem() + "'," + ((int32_t)type.getTypeID()) + "," + (int)accessType.getTypeID());

					Variable v = Variable(ident.getLexem(), type, accessType);
					currentScript->declareGlobal(v);

					if(t.is(Token::TT_OPERATOR, ";"))
					{

					}else if(t.is(Token::TT_OPERATOR, "="))
					{
						if(native)
						{
							error("Native variables must not be initialized by the script", t.getLine());
						}

						ExpressionInfo info = expression();

						if(!info.getType().canCastImplicitlyTo(type))
						{
							error("Type mismatch in initialization of variable '" + ident.getLexem() + "': Can not implicitly cast " + info.getType().toString() + " to " + type.toString(), ident.getLine());
						}

						writeASM("POPVAR '" + ident.getLexem() + "'");

						t = lexer->readToken();
						if(!t.is(Token::TT_OPERATOR,";"))
						{
							error("Expected semicolon after variable initialization, but found: " + t.getLexem(), t.getLine());
						}

					}else
					{
						error("Expected semicolon or initialization after variable declaration, but found: " + t.getLexem(), t.getLine());
					}

    			}else
    			{
    				error("Expected 'func', 'event', 'init' or data type after modifiers, but found: " + t.getLexem(), t.getLine());
    			}

    		}else if(t.getType() == Token::TT_IDENT || t.is(Token::TT_KEYWORD, "this") || t.is(Token::TT_KEYWORD, "parent") || t.is(Token::TT_KEYWORD, "new")) //start of expression
			{
				leftEval();

			}else if(t.getType() == Token::TT_EOF)
    		{
    			error("Unexpected end of file in script block", t.getLine());
    		}else
    		{
    			error("Unexpected token in script block: " + t.getLexem(), t.getLine());
    		}

    		t = lexer->peekToken();
    	}

    	lexer->readToken();

    	std::cout << "-------Script " << name << " end" << std::endl;
    }

    BlockIdent::BlockType Compiler::functionBlock(const BlockIdent &block, const DataType &returnType)
	{
		Token t = lexer->peekToken();

		while(!t.is(Token::TT_KEYWORD,"end"))
		{
			if(t.is(Token::TT_KEYWORD, "return"))
			{
				lexer->readToken();

				t = lexer->peekToken();

				if(t.is(Token::TT_OPERATOR, ";"))
				{
					lexer->readToken();

				}else
				{
					if(returnType.equals(DataType::NOTYPE))
					{
						error("Can not return a value in a void function/event. Expected semicolon after 'return' keyword", t.getLine());
					}

					ExpressionInfo info = expression();

					if(!info.getType().canCastImplicitlyTo(returnType))
					{
						error("Type mismatch in return statement: Can not implicitly cast " + info.getType().toString() + " to " + returnType.toString(), t.getLine());
					}

					t = lexer->readToken();
					if(!t.is(Token::TT_OPERATOR, ";"))
					{
						error("Expected semicolon after return expression, but found: " + t.getLexem(), t.getLine());
					}
				}

				writeASM("RETURN");

			}else if(t.is(Token::TT_KEYWORD, "while"))
			{
				lexer->readToken();

				int32_t currentWhileUID = getNewUID();

				writeASM(String("while_uid") + currentWhileUID + "_start:");

				ExpressionInfo info = expression();
				if(!info.getType().isNumeric())
				{
					error("Loop condition in while statement must be numeric, but non-numeric type " + info.getType().toString() + " was given", t.getLine());
				}

				writeASM(String("JUMPFALSE while_uid") + currentWhileUID + "_end");

				writeASM("BEGIN");

				functionBlock(BlockIdent(BlockIdent::BT_WHILE, currentWhileUID), returnType);

				writeASM("END");

				writeASM(String("JUMP while_uid") + currentWhileUID + "_start");

				writeASM(String("while_uid") + currentWhileUID + "_end:");

			}else if(t.is(Token::TT_KEYWORD, "if"))
			{
				lexer->readToken();

				ifStatement(returnType);

			}else if(t.is(Token::TT_KEYWORD, "else"))
			{
				lexer->readToken();

				if(block.getBlockType() == BlockIdent::BT_IF)
				{

					return BlockIdent::BT_ELSE; //Returns to ifStatement(), further processing happens there

				}else if(block.getBlockType() == BlockIdent::BT_ELSEIF)
				{

					//Currently unsupported, should never occur anyhow

				}else
				{
					error("Else-block must be part of if- or elseif-block", t.getLine());
				}

			}else if(t.is(Token::TT_KEYWORD, "elseif"))
			{
				lexer->readToken();

				if(block.getBlockType() == BlockIdent::BT_IF)
				{

					return BlockIdent::BT_ELSEIF; //Returns to ifStatement(), further processing happens there

				}else if(block.getBlockType() == BlockIdent::BT_ELSEIF)
				{

					//Currently unsupported, should never occur anyhow

				}else if(block.getBlockType() == BlockIdent::BT_ELSE)
				{

					error("Else-block must be defined before else-block", t.getLine());

				}else
				{
					error("Elseif-block must be part of if-block", t.getLine());
				}

			}else if(t.getType() == Token::TT_IDENT || t.is(Token::TT_KEYWORD, "this") || t.is(Token::TT_KEYWORD, "parent") || t.is(Token::TT_KEYWORD, "new")) //start of expression
			{

				leftEval();

			}else if(t.getType() == Token::TT_EOF)
			{
				error("Unexpected end of file in function block", t.getLine());

			}else
			{
				error("Unexpected token in function block: " + t.getLexem(), t.getLine());
			}

			t = lexer->peekToken();
		}

		lexer->readToken();

		return BlockIdent::BT_MAIN; //Regular processing
	}

    void Compiler::ifStatement(const DataType &returnType)
    {
    	ExpressionInfo info = expression();

		if(!info.getType().isNumeric())
		{
			error("Condition in if statement must be numeric, but non-numeric type " + info.getType().toString() + " was given", lexer->getCurrentLine());
		}

		int currentIfUID = getNewUID();

		writeASM(String("JUMPFALSE if_uid") + currentIfUID + "_end");
		writeASM(String("# if_uid") + currentIfUID + "_start:");
		writeASM("BEGIN");

		BlockIdent::BlockType bt = functionBlock(BlockIdent(BlockIdent::BT_IF, currentIfUID), returnType);

		writeASM("END");

		if(bt == BlockIdent::BT_ELSE || bt == BlockIdent::BT_ELSEIF)
		{
			writeASM(String("JUMP if_else_uid") + currentIfUID + "_end");
		}

		writeASM(String("if_uid") + currentIfUID + "_end:");

		if(bt == BlockIdent::BT_ELSE || bt == BlockIdent::BT_ELSEIF)
		{
			writeASM(String("# if_else_uid") + currentIfUID + "_start:");

			if(bt == BlockIdent::BT_ELSE)
			{
				writeASM("BEGIN");

				functionBlock(BlockIdent(BlockIdent::BT_ELSE, currentIfUID), returnType);

				writeASM("END");
			}else
			{
				ifStatement(returnType);
			}

			writeASM(String("if_else_uid") + currentIfUID + "_end:");
		}
    }

    void Compiler::leftEval()
	{
    	ExpressionInfo left = expression(true);

    	Token t = lexer->peekToken();

    	if(t.is(Token::TT_OPERATOR, ";"))
    	{
    		if(left.getFactorType() != ExpressionInfo::FT_FUNCTION_RETURN)
    		{
    			error("Only function call expressions can be closed without an assigment", t.getLine());
    		}

    		if(!left.getType().equals(DataType::NOTYPE))
    		{
    			writeASM("DISCARD");
    		}

    		lexer->readToken();

    	}else if(isAssignmentOperator(t))
    	{
    		if(left.getFactorType() != ExpressionInfo::FT_VARIABLE_REF)
    		{
    			error("Can only assign to variable references", t.getLine());
    		}

    		DataType rightType = rightEval();

			if(!rightType.canCastImplicitlyTo(left.getType()))
			{
				error("Type mismatch in assignment: Can not implicitly cast " + rightType.toString() + " to " + left.getType().toString(), t.getLine());
			}

    	}else
    	{
    		error("Expected assignment or semicolon after left eval, but found: " + t.getLexem(), t.getLine());
    	}

	}

	DataType Compiler::rightEval()
	{
		Token t = lexer->readToken();
		DataType type = DataType::NOTYPE;

		if(t.is(Token::TT_OPERATOR, "="))
		{
			type = expression().getType();

		}else
		{
			writeASM("COPY");
			type = expression().getType();

			if(t.is(Token::TT_OPERATOR, "+="))
			{
				writeASM("ADD");

			}else if(t.is(Token::TT_OPERATOR, "-="))
			{
				writeASM("SUBTRACT");

			}else if(t.is(Token::TT_OPERATOR, "*="))
			{
				writeASM("MULTIPLY");

			}else if(t.is(Token::TT_OPERATOR, "/="))
			{
				writeASM("DIVIDE");

			}else
			{
				error("Unknown assign operator: " + t.getLexem() + ". This is probably not your fault.", t.getLine());
			}
		}

		t = lexer->readToken();
		if(!t.is(Token::TT_OPERATOR, ";"))
		{
			error("Expected semicolon after assignment, but found: " + t.getLexem(), t.getLine());
		}

		writeASM("POPREF");

		return type;
	}

    void Compiler::functionDec(const AccessType &accessType, bool native, const DataType &returnType, bool event)
    {
    	//TODO: Clean up this function

    	Token ident = lexer->readToken();

		if(ident.getType() != Token::TT_IDENT)
		{
			if(event)
			{
				error("Expected event name after 'event' keyword, but found: " + ident.getLexem(), ident.getLine());
			}else
			{
				error("Expected function name after function return type, but found: " + ident.getLexem(), ident.getLine());
			}
		}

		if(accessType.equals(AccessType::UNIVERSAL) && !event)
		{
			error("universal access type is not applicable for functions", ident.getLine());

		}else if(!accessType.equals(AccessType::PUBLIC) && event)
		{
			error("Only public access type is applicable for events", ident.getLine());
		}

		Token t = lexer->readToken();

		if(!t.is(Token::TT_OPERATOR,"("))
		{
			error("Expected parameter list after function name, but found: " + t.getLexem(), t.getLine());
		}

		vector<DataType> paramTypes = vector<DataType>();
		vector<String> paramNames = vector<String>();

		if(!lexer->peekToken().is(Token::TT_OPERATOR,")"))
		{
			do
			{
				t = lexer->readToken();

				if(isDatatype(t))
				{
					DataType type = dataType(t);

					paramTypes.push_back(type);

					t = lexer->readToken();
					if(t.getType() != Token::TT_IDENT)
					{
						error("Expected variable name in parameter list, but found: " + t.getLexem(), t.getLine());
					}
					paramNames.push_back(t.getLexem());

					t = lexer->readToken();

					if(!(t.is(Token::TT_OPERATOR,",") || (t.is(Token::TT_OPERATOR,")"))))
					{
						error("Expected , or closing parentheses in parameter list, but found: " + t.getLexem(), t.getLine());
					}

				}else
				{
					error("Unexpected token in parameter list. Found: " + t.getLexem(), t.getLine());
				}

			}while(t.is(Token::TT_OPERATOR,","));

		}else
		{
			t = lexer->readToken();
		}

		if(!t.is(Token::TT_OPERATOR,")"))
		{
			error("Expected closing parentheses after parameter list, but found: " + t.getLexem(), t.getLine());
		}

		Function func = Function(ident.getLexem(), paramTypes, returnType, accessType, native, event, 0); //TODO: Change adress during assembly
		currentScript->declareFunction(func);

		if(native)
		{
			t = lexer->readToken();
			if(!t.is(Token::TT_OPERATOR, ";"))
			{
				error(String("Native ") + (event ? "events" : "functions") + " must not have a body. Expected semicolon after parameter list, but found: " + t.getLexem(),t.getLine());
			}

			return;
		}

		String defInstr;

		if(event)
		{
			defInstr = String("DECLAREEVENT '") + ident.getLexem() + "'," + (String("") + (int32_t)paramNames.size()) + ",";

		}else
		{
			defInstr = String("DECLAREFUNC '") + ident.getLexem() + "'," + returnType.getTypeID() + "," + (int)accessType.getTypeID() + "," + (String("") + (int32_t)paramNames.size()) + ",";
		}

		for(uint32_t i = 0; i < paramTypes.size(); i++)
		{
			defInstr = defInstr + (int32_t)paramTypes[i].getTypeID();

			if(i < paramTypes.size()-1)
			{
				defInstr += ";";
			}
		}

		int currentFunctionUID = getNewUID();

		defInstr += ",func_" + ident.getLexem() + "_uid" + currentFunctionUID +  "_start";

		writeASM(defInstr);

		writeASM("JUMP func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_end");

		writeASM("func_" + ident.getLexem() + "_uid" + currentFunctionUID +  "_start:");

		writeASM("BEGIN");

		for(int i = paramTypes.size()-1; i >= 0 ; i--)
		{
			DataType paraType = paramTypes[i];
			String paraName = paramNames[i];

			writeASM(String("DECLAREVAR ") + paraType.getTypeID() + ",'" + paraName + "'," + (int)AccessType::PRIVATE.getTypeID());

			writeASM("POPVAR '" + paraName + "'");
		}

		functionBlock(BlockIdent(BlockIdent::BT_FUNC,currentFunctionUID), returnType);

		writeASM("END");

		writeASM("func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_end:");
    }

    void Compiler::constructorDec(const AccessType &accessType)
	{

		if(accessType.equals(AccessType::UNIVERSAL))
		{
			error("universal access type is not applicable for constructors", lexer->getCurrentLine());
		}

		Token t = lexer->readToken();

		if(!t.is(Token::TT_OPERATOR,"("))
		{
			error("Expected parameter list after constructor header, but found: " + t.getLexem(), t.getLine());
		}

		vector<DataType> paramTypes = vector<DataType>();
		vector<String> paramNames = vector<String>();

		if(!lexer->peekToken().is(Token::TT_OPERATOR,")"))
		{
			do
			{
				t = lexer->readToken();

				if(isDatatype(t))
				{
					DataType type = dataType(t);

					paramTypes.push_back(type);

					t = lexer->readToken();
					if(t.getType() != Token::TT_IDENT)
					{
						error("Expected variable name in parameter list, but found: " + t.getLexem(), t.getLine());
					}
					paramNames.push_back(t.getLexem());

					t = lexer->readToken();

					if(!(t.is(Token::TT_OPERATOR,",") || (t.is(Token::TT_OPERATOR,")"))))
					{
						error("Expected , or closing parentheses in parameter list, but found: " + t.getLexem(), t.getLine());
					}

				}else
				{
					error("Unexpected token in parameter list. Found: " + t.getLexem(), t.getLine());
				}

			}while(t.is(Token::TT_OPERATOR,","));

		}else
		{
			t = lexer->readToken();
		}

		if(!t.is(Token::TT_OPERATOR,")"))
		{
			error("Expected closing parentheses after parameter list, but found: " + t.getLexem(), t.getLine());
		}

		String defInstr = String("DECLAREFUNC 'init',") + DataType::NOTYPE.getTypeID() + "," + (int)accessType.getTypeID() + "," + (String("") + (int32_t)paramNames.size()) + ",";

		for(uint32_t i = 0; i < paramTypes.size(); i++)
		{
			defInstr = defInstr + (int32_t)paramTypes[i].getTypeID();

			if(i < paramTypes.size()-1)
			{
				defInstr += ";";
			}
		}

		int currentFunctionUID = getNewUID();

		Function func = Function(String("init"), paramTypes, DataType::NOTYPE, accessType, false, false, 0); //TODO: Change adress during assembly
		currentScript->declareFunction(func);

		defInstr += String(", constructor_uid") + currentFunctionUID +  "_start";

		writeASM(defInstr);

		writeASM(String("JUMP constructor_uid") + currentFunctionUID + "_end");

		writeASM(String("constructor_uid") + currentFunctionUID +  "_start:");

		writeASM("BEGIN");

		for(int i = paramTypes.size()-1; i >= 0 ; i--)
		{
			DataType paraType = paramTypes[i];
			String paraName = paramNames[i];

			writeASM(String("DECLAREVAR ") + paraType.getTypeID() + ",'" + paraName + "'," + (int)AccessType::PRIVATE.getTypeID());

			writeASM("POPVAR '" + paraName + "'");
		}

		functionBlock(BlockIdent(BlockIdent::BT_FUNC,currentFunctionUID), DataType::NOTYPE);

		writeASM("END");

		writeASM(String("constructor_uid") + currentFunctionUID + "_end:");
	}

    DataType Compiler::functionCall(const Token &ident, bool member, const DataType &baseType)
    {
    	String funcName = ident.getLexem();

		Token t = lexer->readToken();
		if(!t.is(Token::TT_OPERATOR,"("))
		{
			error("Expected opening parentheses after function name in function call", t.getLine());
		}

		int paramCount = 0;

		vector<DataType> paramTypes = vector<DataType>();

		t = lexer->peekToken();
		if(!t.is(Token::TT_OPERATOR,")"))
		{

			do
			{
				ExpressionInfo info = expression();

				paramTypes.push_back(info.getType());

				paramCount++;
				t = lexer->readToken();

			}while(t.is(Token::TT_OPERATOR,","));

			if(!t.is(Token::TT_OPERATOR,")"))
			{
				error("Expected closing parentheses after parameter list in function call", t.getLine());
			}

		}else
		{
			lexer->readToken();
		}

		if(member)
		{
			if(!baseType.equals(DataType::OBJECT))
			{
				error("Only object types have callable member functions, but given type was: " + baseType.toString(), t.getLine());
			}

			Script *script = scriptSystem.getScript(baseType.getSpecifier());

			if(script == null)
			{
				error("Specifier of given base type for member function call points to non-existent script: " + baseType.getSpecifier(), t.getLine());
			}

			Function *function = script->getFunction(ident.getLexem(), paramTypes);

			if(function == null)
			{
				error("Function '" + Function::createInfoString(ident.getLexem(), paramTypes) + "' was not defined in given object type script '" + baseType.getSpecifier() + "'", t.getLine());
			}
			
			if(function->getAccessType().equals(AccessType::PRIVATE))
			{
				error("Function '" + Function::createInfoString(ident.getLexem(), paramTypes) + "' in given object type script '" + baseType.getSpecifier() + "' is private", t.getLine());
			}

			writeASM("CALLMEMBER '" + funcName + "'," + paramCount);

			return function->getReturnType();

		}else
		{
			Function *function = currentScript->getFunction(ident.getLexem(), paramTypes);

			if(function == null)
			{
				error("Function '" + Function::createInfoString(ident.getLexem(), paramTypes) + "' was not defined in current script", t.getLine());
			}

			//No need to check for private, as we are calling from the same script
			
			writeASM("CALL '" + funcName + "'," + paramCount);

			return function->getReturnType();
		}
    }

    DataType Compiler::dataType(const Token &type)
    {
    	if(!isDatatype(type))
    	{
    		error("Expected data type, but received: " + type.getLexem(), type.getLine());
    	}

    	DataType dataType = DataType::byName(type.getLexem());

    	if(type.getLexem().equals("object"))
    	{
    		Token t = lexer->readToken();

    		if(!t.is(Token::TT_OPERATOR, "<"))
    		{
    			error("Expected script specifier after object datatype, but found: " + t.getLexem(), t.getLine());
    		}

    		//TODO: Maybe check if given script specifier is really existent
    		Token specifier = lexer->readToken();

    		if(specifier.getType() != Token::TT_IDENT)
    		{
    			error("Expected script specifier after object datatype, but found: " + specifier.getLexem(), specifier.getLine());
    		}

    		t = lexer->readToken();

    		if(!t.is(Token::TT_OPERATOR, ">"))
			{
				error("Expected closing chevron after script specifier, but found: " + t.getLexem(), t.getLine());
			}

    		dataType.initSpecifier(specifier.getLexem());
    	}

    	return dataType;
    }

    ExpressionInfo Compiler::expression(const bool leftEval)
    {
        ExpressionInfo info = relationalExpression(leftEval);

        Token t = lexer->peekToken();
        while(isLogicOp(t))
        {
            lexer->readToken();

            ExpressionInfo rightInfo = relationalExpression(leftEval);

            if(!info.getType().isNumeric())
			{
				error(String("Left hand side of operator ") + t.getLexem() + " is of non-numeric type(" + info.getType().toString() + "), but " + t.getLexem() + " requires numeric types (representing booleans)", t.getLine());
			}
			if(!rightInfo.getType().isNumeric())
			{
				error(String("Right hand side of operator ") + t.getLexem() + " is of non-numeric type(" + rightInfo.getType().toString() +"), but " + t.getLexem() + " requires numeric types (representing booleans)", t.getLine());
			}

            info = ExpressionInfo(DataType::INT, rightInfo.isConstant() && info.isConstant(), ExpressionInfo::FT_MATH_EXPR); //Type is always int after logic operation

            if(t.is(Token::TT_OPERATOR,"|"))
            {
                writeASM("LOGICOR");

            }else if(t.is(Token::TT_OPERATOR,"&"))
            {
                writeASM("LOGICAND");
            }

            t = lexer->peekToken();
        }

        return info;
    }

    ExpressionInfo Compiler::relationalExpression(const bool leftEval)
    {
        ExpressionInfo info = arithmeticExpression(leftEval);

        Token t = lexer->peekToken();
        while(isRelationalOp(t))
        {
            lexer->readToken();

            ExpressionInfo rightInfo = arithmeticExpression(leftEval);
            info = ExpressionInfo(DataType::INT, rightInfo.isConstant() && info.isConstant(), ExpressionInfo::FT_MATH_EXPR); //Type is always int after relation

            if(t.is(Token::TT_OPERATOR,"=="))
            {
                writeASM("COMPARE");

            }else if(t.is(Token::TT_OPERATOR,"<"))
            {
                writeASM("LESS");

            }else if(t.is(Token::TT_OPERATOR,">"))
            {
                writeASM("GREATER");

            }else if(t.is(Token::TT_OPERATOR,"<="))
            {
                writeASM("LESSEQUAL");

            }else if(t.is(Token::TT_OPERATOR,">="))
            {
                writeASM("GREATEREQUAL");
            }else if(t.is(Token::TT_OPERATOR,"!="))
            {
                writeASM("COMPARE");
                writeASM("INVERT");
            }

            t = lexer->peekToken();
        }

        return info;
    }

    ExpressionInfo Compiler::arithmeticExpression(const bool leftEval)
    {
    	ExpressionInfo info = term(leftEval);

        Token t = lexer->peekToken();
        while(isAddOp(t))
        {
            lexer->readToken();

            ExpressionInfo rightInfo = term(leftEval);

            if(t.is(Token::TT_OPERATOR,"+"))
            {
            	if(info.getType().equals(DataType::STRING) || rightInfo.getType().equals(DataType::STRING))
            	{
            		info = ExpressionInfo(DataType::STRING, info.isConstant() && rightInfo.isConstant(), ExpressionInfo::FT_MATH_EXPR);

            	}else
            	{
            		if(!info.getType().isNumeric())
					{
						error(String("Left hand side of operator + is of non-numeric type(") + info.getType().toString() + "), but + requires numeric types on both sides or at least one string type", t.getLine());
					}
					if(!rightInfo.getType().isNumeric())
					{
						error(String("Right hand side of operator + is of non-numeric type(") + rightInfo.getType().toString() + "), but + requires numeric types on both sides or at least one string type", t.getLine());
					}

					info = ExpressionInfo(DataType::mathCast(info.getType(),rightInfo.getType()), info.isConstant() && rightInfo.isConstant(), ExpressionInfo::FT_MATH_EXPR);
            	}

                writeASM("ADD");

            }else if(t.is(Token::TT_OPERATOR,"-"))
            {

            	if(!info.getType().isNumeric())
				{
					error(String("Left hand side of operator - is of non-numeric type(") + info.getType().toString() +"), but - requires numeric types on both sides", t.getLine());
				}
				if(!rightInfo.getType().isNumeric())
				{
					error(String("Right hand side of operator - is of non-numeric type(") + rightInfo.getType().toString() + "), but - requires numeric types on both sides", t.getLine());
				}

            	info = ExpressionInfo(DataType::mathCast(info.getType(),rightInfo.getType()), info.isConstant() && rightInfo.isConstant(), ExpressionInfo::FT_MATH_EXPR);

                writeASM("SUBSTRACT");
            }

            t = lexer->peekToken();
        }

        return info;
    }

    ExpressionInfo Compiler::term(const bool leftEval)
    {
    	ExpressionInfo info = castFactor(leftEval);

        Token t = lexer->peekToken();
        while(isMultiplyOp(t))
        {
            lexer->readToken();

            ExpressionInfo rightInfo = castFactor(leftEval);

            if(!info.getType().isNumeric())
            {
            	error(String("Left hand side of operator ") + t.getLexem() + " is of non-numeric type(" + rightInfo.getType().toString() + "), but " + t.getLexem() + " requires numeric types", t.getLine());
            }
            if(!rightInfo.getType().isNumeric())
			{
				error(String("Right hand side of operator ") + t.getLexem() + " is of non-numeric type(" + rightInfo.getType().toString() + "), but " + t.getLexem() + " requires numeric types", t.getLine());
			}

            info = ExpressionInfo(DataType::mathCast(info.getType(),rightInfo.getType()), info.isConstant() && rightInfo.isConstant(), ExpressionInfo::FT_MATH_EXPR);

            if(t.is(Token::TT_OPERATOR,"*"))
            {
                writeASM("MULTIPLY");

            }else if(t.is(Token::TT_OPERATOR,"/"))
            {
                writeASM("DIVIDE");
            }

            t = lexer->peekToken();
        }

        return info;
    }

    ExpressionInfo Compiler::castFactor(const bool leftEval)
	{
		ExpressionInfo info = signedFactor(leftEval);

		Token t = lexer->peekToken();
		if(t.is(Token::TT_OPERATOR,"->"))
		{
			if(info.getType().equals(DataType::NOTYPE))
			{
				error("Can not cast nulltype to anything", t.getLine());
			}

			lexer->readToken();

			t = lexer->readToken();

			if(!isDatatype(t))
			{
				error("Expected data type after cast operator, but found: " + t.getLexem(), t.getLine());
			}

			DataType type = dataType(t);

			if(type.equals(DataType::OBJECT))
			{
				writeASM(String("TOOBJECT '") + type.getSpecifier() + "'");

			}else
			{
				writeASM(String("TO") + type.getTypeName().toUpperCase());
			}

			return ExpressionInfo(type, false, ExpressionInfo::FT_MATH_EXPR);
		}

		return info;
	}

    ExpressionInfo Compiler::signedFactor(const bool leftEval)
	{
		bool negate = false;
		bool invert = false;

		Token t = lexer->peekToken();
		if(t.is(Token::TT_OPERATOR,"-"))
		{
			negate = true;
			lexer->readToken();

		}else if(t.is(Token::TT_OPERATOR,"!"))
		{
			invert = true;
			lexer->readToken();
		}

		ExpressionInfo info = memberFactor(leftEval);

		if(negate)
		{
			if(!info.getType().isNumeric())
			{
				error(String("Negation operator is applicable on numeric types only. Given type is: ") + info.getType().toString() ,t.getLine());
			}

			writeASM("NEGATE");
		}

		if(invert)
		{
			if(!info.getType().isNumeric())
			{
				error(String("Inversion operator is applicable on numeric types only. Given type is: ") + info.getType().toString() ,t.getLine());
			}

			writeASM("INVERT");

			info = ExpressionInfo(DataType::INT, info.isConstant(), ExpressionInfo::FT_MATH_EXPR);
		}

		return info;
	}

    ExpressionInfo Compiler::memberFactor(const bool leftEval)
    {
        ExpressionInfo info = factor(leftEval);

        Token t = lexer->peekToken();
        while(t.is(Token::TT_OPERATOR,"."))
        {
        	if(!info.getType().equals(DataType::OBJECT))
			{
				error("Only object types have accessible members, but given type is: " + info.getType().toString(), t.getLine());
			}

            lexer->readToken();
            t = lexer->readToken();
            if(t.getType() != Token::TT_IDENT)
            {
                error(String("Expected identifier after member directive, but found: ") + t.getLexem(), t.getLine());
            }

            Token member = t;

            t = lexer->peekToken();
            if(t.is(Token::TT_OPERATOR,"("))
            {
                DataType ftype = functionCall(member, true, info.getType());

                if(ftype.equals(DataType::NOTYPE) && !leftEval)
                {
                	error("Function '" + member.getLexem() + "' of script '" + info.getType().getSpecifier()  + "' is of type void, but it was used in a value-context", member.getLine());
                }

                info = ExpressionInfo(ftype, false, ExpressionInfo::FT_FUNCTION_RETURN);

            }else
            {
            	Script *s = scriptSystem.getScript(info.getType().getSpecifier());
				if(s == null)
				{
					error("Specifier of given object type points to non-existent script '" + info.getType().getSpecifier() + "'", member.getLine());
				}

				Variable *v = s->getVariable(member.getLexem());
				if(v == null)
				{
					//No need to check for universals here, as they can not be referenced through objects

					error("Referenced script '" + info.getType().getSpecifier() + "' has no member '" + member.getLexem() + "'", member.getLine());
				}

				if(v->getAccessType().equals(AccessType::PRIVATE))
				{
					error("Variable '" + member.getLexem() + "' in referenced script '" + info.getType().getSpecifier() + "' is private", member.getLine());
				}

                info = ExpressionInfo(v->getType(), false, ExpressionInfo::FT_VARIABLE_REF);

                writeASM("GETMEMBER '" + member.getLexem() + "'");
            }
        }

        return info;
    }

    ExpressionInfo Compiler::factor(const bool leftEval)
    {
        Token t = lexer->readToken();

        if(t.getType() == Token::TT_NUMBER)
        {
            DataType numberType = getTypeOfNumberString(t.getLexem());

            if(numberType.equals(DataType::NOTYPE))
            {
                error("Number literal '" + t.getLexem() + "' does not fit in any numerical data type",t.getLine());
            }

            writeASM("PUSH" + numberType.getTypeName().toUpperCase() + " " + t.getLexem());

            return ExpressionInfo(numberType, true, ExpressionInfo::FT_LITERAL);

        }else if(t.getType() == Token::TT_STRING)
        {
            writeASM("PUSHSTRING '" + escapeASMChars(t.getLexem()) + "'");

           return ExpressionInfo(DataType::STRING, true, ExpressionInfo::FT_LITERAL);

        }else if(t.getType() == Token::TT_IDENT)
        {
            Token t2 = lexer->peekToken();
            if(t2.is(Token::TT_OPERATOR,"("))
            {
                DataType type = functionCall(t,false);

                if(type.equals(DataType::NOTYPE) && !leftEval)
                {
                	error("Function '" + t.getLexem() + "' is of type void, but it was used in a value-context", t.getLine());
                }

                return ExpressionInfo(type, false, ExpressionInfo::FT_FUNCTION_RETURN);

            }else
            {
                Variable *v = currentScript->getVariable(t.getLexem());

                if(v == null)
                {
                	//Variable is not declared in script. Check if there is an universal instead
                	v = scriptSystem.getUniversal(t.getLexem());

					if(v == null)
					{
						error("Variable '" + t.getLexem() + "' was not declared in this script", t.getLine());
					}
                }

                writeASM("PUSHVAR '" + t.getLexem() + "'");

                return ExpressionInfo(v->getType(), false, ExpressionInfo::FT_VARIABLE_REF);
            }

        }else if(t.is(Token::TT_OPERATOR,"("))
        {
            ExpressionInfo info = expression();

            Token t2 = lexer->readToken();
            if(!t2.is(Token::TT_OPERATOR,")"))
            {
                error("Missing closing parentheses after expression",t.getLine());
            }

            return info;

        }else if(t.is(Token::TT_KEYWORD,"new"))
        {
            Token scriptname = lexer->readToken();
			
			if(scriptname.getType() != Token::TT_IDENT)
			{
				error("Expected scriptname after 'new' keyword, but found: " + scriptname.getLexem(), scriptname.getLine());
			}
			
			Token t2 = lexer->readToken();
			if(!t2.is(Token::TT_OPERATOR,"("))
			{
				error("Expected parameter list after scriptname in constructor call, but found: " + t2.getLexem(), t2.getLine());
			}

			int paramCount = 0;

			vector<DataType> paramTypes = vector<DataType>();

			t2 = lexer->peekToken();
			if(!t2.is(Token::TT_OPERATOR,")"))
			{

				do
				{
					ExpressionInfo info = expression();

					paramTypes.push_back(info.getType());

					paramCount++;
					t2 = lexer->readToken();

				}while(t2.is(Token::TT_OPERATOR,","));

				if(!t2.is(Token::TT_OPERATOR,")"))
				{
					error("Expected closing parentheses after parameter list in constructor call, but found: " + t2.getLexem(), t2.getLine());
				}

			}else
			{
				lexer->readToken();
			}
			
			Script *script = scriptSystem.getScript(scriptname.getLexem());
			if(script == null)
			{
				error("Given script '" + scriptname.getLexem() + "' in new statement was not declared",t2.getLine());
			}
			
			writeASM("NEW '" + scriptname.getLexem() + "'");
			
			Function *constr = script->getFunction("init", paramTypes);
			
			if(constr == null)
			{
				if(paramCount > 0)
				{
					error("Constructor " + Function::createInfoString(scriptname.getLexem(), paramTypes) + " was not declared", t2.getLine());
				}
				
			}else
			{
				if(constr->getAccessType().equals(AccessType::PRIVATE))
				{
					error("Constructor " + Function::createInfoString(scriptname.getLexem(), paramTypes) + " is private", t2.getLine());
				}
			
				writeASM("COPY");
				writeASM(String("CALLMEMBER 'init',") + paramCount);
			}
			
			DataType dt = DataType::OBJECT;
			dt.initSpecifier(scriptname.getLexem());

			return ExpressionInfo(dt, false, ExpressionInfo::FT_FUNCTION_RETURN); //Constructors are considered as functions for now

        }else if(t.is(Token::TT_KEYWORD,"null"))
        {
            writeASM("PUSHNULL");

            return ExpressionInfo(DataType::OBJECT, true, ExpressionInfo::FT_LITERAL); //TODO: Re-think if NULL is really an object

        }else if(t.is(Token::TT_KEYWORD,"this"))
        {
        	writeASM("PUSHTHIS");

        	DataType type = DataType::OBJECT;
        	type.initSpecifier(currentScript->getName());

        	return ExpressionInfo(type, false, ExpressionInfo::FT_LITERAL); //Although "PUSHTHIS" actually creates reference, returning FT_VARIABLE_REF would allow the program to assign to it

        }else if(t.getType() == Token::TT_EOF)
        {
            error("Unexpected end of file while parsing expression", t.getLine());

        }else
        {
            error("Unexpected token in expression factor: " + t.getLexem(), t.getLine());
        }

        return ExpressionInfo(DataType::NOTYPE, false, ExpressionInfo::FT_LITERAL); //Should never happen
    }

    DataType Compiler::getTypeOfNumberString(const String &s)
    {
    	bool fp = (s.indexOf('.') != -1);

    	//TODO: Consider big types here
    	return fp ? DataType::FLOAT : DataType::INT;
    }

    bool Compiler::isAddOp(const Token &t)
    {
    	return t.is(Token::TT_OPERATOR, "+") || t.is(Token::TT_OPERATOR, "-");
    }

    bool Compiler::isMultiplyOp(const Token &t)
    {
    	return t.is(Token::TT_OPERATOR, "*") || t.is(Token::TT_OPERATOR, "/");
    }

    bool Compiler::isLogicOp(const Token &t)
    {
    	return t.is(Token::TT_OPERATOR, "|") || t.is(Token::TT_OPERATOR, "&");
    }

    bool Compiler::isRelationalOp(const Token &t)
    {
    	return t.is(Token::TT_OPERATOR, "==") || t.is(Token::TT_OPERATOR, "!=") || t.is(Token::TT_OPERATOR, ">") || t.is(Token::TT_OPERATOR, "<") || t.is(Token::TT_OPERATOR, ">=") || t.is(Token::TT_OPERATOR, "<=");
    }

    bool Compiler::isAssignmentOperator(const Token &t)
    {
    	return t.is(Token::TT_OPERATOR,"=") || t.is(Token::TT_OPERATOR,"+=") || t.is(Token::TT_OPERATOR,"-=") || t.is(Token::TT_OPERATOR,"*=") || t.is(Token::TT_OPERATOR,"/=");
    }

    bool Compiler::isAccessModifier(const Token &t)
    {
    	return t.is(Token::TT_KEYWORD, "public") || t.is(Token::TT_KEYWORD, "private") || t.is(Token::TT_KEYWORD, "universal");
    }

    bool Compiler::isDatatype(const Token &t)
    {
    	if(t.getType() != Token::TT_KEYWORD)
    	{
    		return false;
    	}

    	for(uint16_t i = 0; i < DATATYPE_COUNT; i++)
    	{

    		if(t.getLexem().equals(DATATYPES[i]))
    		{
    			return true;
    		}
    	}

    	return false;
    }

    String Compiler::escapeASMChars(const String &s)
    {
    	//TODO: Do useful stuff here

    	return s;
    }

    uint32_t Compiler::getNewUID()
    {
    	return ++lastUID;
    }

    void Compiler::writeASM(const String &s)
    {
    	std::cout << s << std::endl;
    }

    void Compiler::error(const String &s, int line)
    {
    	//TODO: Do more stuff with the error message than just throwing an exception

    	throw Exception(String("Error in line ") + line + ": " + s);
    }

    const String Compiler::KEYWORDS[] =
	{
			"namespace",
			"default",
			"begin",
			"end",
			"public",
			"private",
			"universal",
			"func",
			"event",
			"native",
			"void",
			"script",
			"static",
			"links",
			"extends",
			"uses",
			"return",
			"while",
			"if",
			"elseif",
			"else",
			"break",
			"null",
			"this",
			"parent",
			"goto",
			"new",
			"init",

			"int",
			"long",
			"float",
			"double",
			"string",
			"object"
	};
	const uint32_t Compiler::KEYWORD_COUNT = sizeof(KEYWORDS)/sizeof(String);

	const String Compiler::DATATYPES[] =
	{
			"int",
			"long",
			"float",
			"double",
			"string",
			"object"
	};
	const uint32_t Compiler::DATATYPE_COUNT = sizeof(DATATYPES)/sizeof(String);

	const String Compiler::OPERATORS[] =
	{
			"+",
			"-",
			"*",
			"/",
			"=",
			"+=",
			"-=",
			"*=",
			"/=",
			"(",
			")",
			"->",
			"<",
			">",
			"<=",
			">=",
			"!=",
			"==",
			"&",
			"|",
			"!",
			".",
			",",
			";"
	};
	const uint32_t Compiler::OPERATOR_COUNT = sizeof(OPERATORS)/sizeof(String);


    //private class ExpressionInfo

    ExpressionInfo::ExpressionInfo(const DataType &dtype, const bool cons, const FactorType ftype) : dataType(dtype), constant(cons), factorType(ftype)
    {

    }

    bool ExpressionInfo::isConstant()
    {
    	return constant;
    }

    ExpressionInfo::FactorType ExpressionInfo::getFactorType()
    {
    	return factorType;
    }

    DataType ExpressionInfo::getType()
    {
    	return dataType;
    }


    //private class BlockIdent

    BlockIdent::BlockIdent(BlockIdent::BlockType pType, uint32_t pUid)
    :
    		type(pType),
    		uid(pUid)
    {

    }

    BlockIdent::BlockType BlockIdent::getBlockType() const
    {
    	return type;
    }

    uint32_t BlockIdent::getUID() const
    {
    	return uid;
    }
}
