
#include "comp/Compiler.h"

#include <iostream>

namespace Scales
{

	//TODO: Find out if "const Object &o" is really more efficient than the stuff that has to be copied all the time

	//public class compiler

    Compiler::Compiler(istream &in)
    {
    	lexer = new Lexer(in);

    	keywords = vector<String>(30);
    	keywords.push_back("namespace");
    	keywords.push_back("default");
    	keywords.push_back("begin");
    	keywords.push_back("end");
    	keywords.push_back("public");
    	keywords.push_back("private");
    	keywords.push_back("universal");
    	keywords.push_back("func");
    	keywords.push_back("event");
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
    	keywords.push_back("parent");
    	keywords.push_back("goto");
    	keywords.push_back("new");

    	datatypes = vector<String>(8);
    	datatypes.push_back("int");
    	datatypes.push_back("long");
    	datatypes.push_back("float");
    	datatypes.push_back("double");
    	datatypes.push_back("string");
    	datatypes.push_back("object");

    	operators = vector<String>(25);
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

    	currentScript = null;

    	lexer->setIgnoreComments(true);

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

    	Token t = lexer->readToken();

    	std::cout << "-------Script " << name << " - " << (staticScript ? "static" : "") << "" << std::endl;

    	while(!t.is(Token::TT_KEYWORD,"end"))
    	{
    		if(isAccessModifier(t) || t.is(Token::TT_KEYWORD,"native") || t.is(Token::TT_KEYWORD, "func") || t.is(Token::TT_KEYWORD,"event") || isDatatype(t))
    		{
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

    				if(!isDatatype(t) && !t.is(Token::TT_KEYWORD,"void"))
    				{
    					error("Expected data type or 'void' after func keyword, but found: " + t.getLexem(), t.getLine());
    				}

    				DataType returntype = t.is(Token::TT_KEYWORD,"void") ? DataType::NOTYPE : dataType(t);

    				functionDec(accessType, native, returntype, false);

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
    				error("Expected 'func' or data type after modifiers, but found: " + t.getLexem(), t.getLine());
    			}

    		}else if(t.getType() == Token::TT_IDENT)
			{
				leftEval(t);

			}else if(t.getType() == Token::TT_EOF)
    		{
    			error("Unexpected end of file in script block", t.getLine());
    		}else
    		{
    			error("Unexpected token in script block: " + t.getLexem(), t.getLine());
    		}

    		t = lexer->readToken();
    	}

    	std::cout << "Script " << name << " end--------" << std::endl;
    }

    BlockIdent::BlockType Compiler::functionBlock(const BlockIdent &block, const DataType &returnType)
	{
		Token t = lexer->readToken();

		while(!t.is(Token::TT_KEYWORD,"end"))
		{
			if(t.is(Token::TT_KEYWORD, "return"))
			{
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

				ifStatement(returnType);

			}else if(t.is(Token::TT_KEYWORD, "else"))
			{

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

			}else if(t.getType() == Token::TT_IDENT)
			{
				leftEval(t);

			}else if(t.getType() == Token::TT_EOF)
			{
				error("Unexpected end of file in function block", t.getLine());

			}else
			{
				error("Unexpected token in function block: " + t.getLexem(), t.getLine());
			}

			t = lexer->readToken();
		}

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

    void Compiler::leftEval(const Token &firstIdent)
	{
    	//Note: This function was directly ported from the java version of KniftoScript 2

		Token t = lexer->peekToken();
		DataType type = DataType::NOTYPE;

		if(t.is(Token::TT_OPERATOR,"("))
		{
			type = functionCall(firstIdent, false);

			t = lexer->readToken();

			if(t.is(Token::TT_OPERATOR,";"))
			{
				if(!type.equals(DataType::NOTYPE))
				{
					//Function type != void -> remove unused return value from stack

					writeASM("POPNULL");
				}

				return;

			}else if(t.is(Token::TT_OPERATOR,"."))
			{
				if(!type.equals(DataType::OBJECT))
				{
					error("Only object types have accessible members, but given type was: " + type.toString(), t.getLine());
				}

				//goto loop

			}else if(isAssignmentOperator(t))
			{
				error("Cannot assign to the return value of a function", t.getLine());

			}else
			{
				error("Expected semicolon or member directive after function call, but found: " + t.getLexem(), t.getLine());
			}

		}else if(t.is(Token::TT_OPERATOR,"."))
		{
			writeASM("PUSHVAR '" + firstIdent.getLexem() + "'");

			Variable *v = currentScript->getVariable(firstIdent.getLexem());
			if(v == null)
			{
				v = scriptSystem.getUniversal(firstIdent.getLexem());

				if(v == null)
				{
					error("Variable '" + firstIdent.getLexem() + "' was not declared in this script", firstIdent.getLine());
				}
			}

			type = v->getType();

			if(!type.equals(DataType::OBJECT))
			{
				error("Only object types have accessible members, but given type was: " + type.toString(), t.getLine());
			}

			lexer->readToken();

			//goto loop

		}else if(isAssignmentOperator(t))
		{
			Variable *v =  currentScript->getVariable(firstIdent.getLexem());

			if(v == null)
			{
				v = scriptSystem.getUniversal(firstIdent.getLexem());

				if(v == null)
				{
					error("Variable '" + firstIdent.getLexem() + "' was not declared in this script", firstIdent.getLine());
				}
			}

			type = v->getType();

			writeASM("PUSHVAR '" + firstIdent.getLexem() + "'");

			DataType rightType = rightEval();

			if(!rightType.canCastImplicitlyTo(type))
			{
				error("Type mismatch in assignment: Can not implicitly cast " + rightType.toString() + " to " + type.toString(), t.getLine());
			}

			return;

		}else
		{
			error("Expected member directive, parameter list or assignment after identifier, but found: " + t.getLexem(), t.getLine());
		}

		do
		{

			t = lexer->readToken();

			if(t.getType() != Token::TT_IDENT)
			{
				error("Expected identifier after member directive, but found: " + t.getLexem(), t.getLine());
			}

			Token ident = t;

			t = lexer->peekToken();

			if(t.is(Token::TT_OPERATOR,"("))
			{
				type = functionCall(ident, true, type);

				//Type is now return type of function

				t = lexer->readToken();

				if(t.is(Token::TT_OPERATOR, ";"))
				{
					if(!type.equals(DataType::NOTYPE))
					{
						//Function type != void -> remove unused return value from stack

						writeASM("POPNULL");
					}

					return;

				}else if(t.is(Token::TT_OPERATOR, "."))
				{
					if(!type.equals(DataType::OBJECT))
					{
						error("Only object types have accessible members, but given return type of function is: " + type.toString(), t.getLine());
					}

					//goto loop

				}else if(isAssignmentOperator(t))
				{
					error("Cannot assign to the return value of a function", t.getLine());

				}else
				{
					error("Expected semicolon or member directive after function call, but found: " + t.getLexem(), t.getLine());
				}

			}else if(t.is(Token::TT_OPERATOR, "."))
			{
				//type = the data type of the element left of the dot operator

				if(!type.equals(DataType::OBJECT))
				{
					error("Only object types have accessible members, but given type was: " + type.toString(), t.getLine());
				}

				Script *s = scriptSystem.getScript(type.getSpecifier());
				if(s == null)
				{
					error("Specifier of given object type points to non-existent script '" + type.getSpecifier() + "'",t.getLine());
				}

				Variable *v = s->getVariable(ident.getLexem());
				if(v == null)
				{
					error("Referenced script '" + type.getSpecifier() + "' has no member '" + ident.getLexem() + "'", t.getLine());
				}

				if(v->getAccessType().equals(AccessType::PRIVATE))
				{
					error("Variable '" + ident.getLexem() + "' in referenced script '" + type.getSpecifier() + "' is private", t.getLine());
				}

				type = v->getType();

				writeASM("GETMEMBER '" + ident.getLexem() + "'");

				lexer->readToken();

			}else if(isAssignmentOperator(t))
			{
				if(!type.equals(DataType::OBJECT))
				{
					error("Only object types have accessible members, but given type was: " + type.toString(), t.getLine());
				}

				Script *s = scriptSystem.getScript(type.getSpecifier());
				if(s == null)
				{
					error("Specifier of given object type points to non-existent script '" + type.getSpecifier() + "'",t.getLine());
				}

				Variable *v = s->getVariable(ident.getLexem());
				if(v == null)
				{
					//No need to check for universals here, as they can not be referenced through objects

					error("Referenced script '" + type.getSpecifier() + "' has no member '" + ident.getLexem() + "'", t.getLine());
				}

				if(v->getAccessType().equals(AccessType::PRIVATE))
				{
					error("Variable '" + ident.getLexem() + "' in referenced script '" + type.getSpecifier() + "' is private", t.getLine());
				}

				type = v->getType();

				writeASM("GETMEMBER '" + ident.getLexem() + "'");

				DataType rightType = rightEval();

				if(!rightType.canCastImplicitlyTo(type))
				{
					error("Type mismatch in assignment: Can not implicitly cast " + rightType.toString() + " to " + type.toString(), t.getLine());
				}

				return;
			}else
			{
				error("Expected member directive, function call or assignment after identifier, but found: " + t.getLexem(), t.getLine());
			}

		}while(t.is(Token::TT_OPERATOR, "."));

		error("Expected .-token, but found: " + t.getLexem() + ". This should really never happen", t.getLine());
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

		Function func = Function(ident.getLexem(), paramTypes, returnType, accessType, native, event, 0); //TODO: Change adress during assembly
		currentScript->declareFunction(func);

		defInstr += ",func_" + ident.getLexem() + "_uid" + currentFunctionUID +  "_start";

		writeASM(defInstr);

		writeASM("JUMP func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_end");

		writeASM("func_" + ident.getLexem() + "_uid" + currentFunctionUID +  "_start:");

		writeASM("BEGIN");

		for(int i = paramTypes.size()-1; i >= 0 ; i--)
		{
			DataType paraType = paramTypes[i];
			String paraName = paramNames[i];

			writeASM(String("DECLAREVAR ") + paraType.getTypeID() + ",'" + paraName + "', private");

			writeASM("POPVAR '" + paraName + "'");
		}

		functionBlock(BlockIdent(BlockIdent::BT_FUNC,currentFunctionUID), returnType);

		writeASM("END");

		writeASM("func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_end:");
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

			writeASM("CALLMEMBER '" + funcName + "'," + paramCount);

			return function->getReturnType();

		}else
		{
			Function *function = currentScript->getFunction(ident.getLexem(), paramTypes);

			if(function == null)
			{
				//TODO: Add parameter information to this error message
				error("Function '" + Function::createInfoString(ident.getLexem(), paramTypes) + "' was not defined in current script", t.getLine());
			}

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
				error(String("Left hand side of operator ") + t.getLexem() + " is of non-numeric type(" + info.getType().toString() + "), but " + t.getLexem() + " requires numeric types (representing booleans)", t.getLine());
			}
			if(!rightInfo.getType().isNumeric())
			{
				error(String("Right hand side of operator ") + t.getLexem() + " is of non-numeric type(" + rightInfo.getType().toString() +"), but " + t.getLexem() + " requires numeric types (representing booleans)", t.getLine());
			}

            info = ExpressionInfo(DataType::INT, rightInfo.isConstant() && info.isConstant()); //Type is always int after logic operation

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

    ExpressionInfo Compiler::relationalExpression()
    {
        ExpressionInfo info = arithmeticExpression();

        Token t = lexer->peekToken();
        while(isRelationalOp(t))
        {
            lexer->readToken();

            ExpressionInfo rightInfo = arithmeticExpression();
            info = ExpressionInfo(DataType::INT, rightInfo.isConstant() && info.isConstant()); //Type is always int after relation

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

    ExpressionInfo Compiler::arithmeticExpression()
    {
    	ExpressionInfo info = term();

        Token t = lexer->peekToken();
        while(isAddOp(t))
        {
            lexer->readToken();

            ExpressionInfo rightInfo = term();

            if(t.is(Token::TT_OPERATOR,"+"))
            {
            	if(info.getType().equals(DataType::STRING) || rightInfo.getType().equals(DataType::STRING))
            	{
            		info = ExpressionInfo(DataType::STRING, info.isConstant() && rightInfo.isConstant());

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

					info = ExpressionInfo(DataType::mathCast(info.getType(),rightInfo.getType()), info.isConstant() && rightInfo.isConstant());
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
            	error(String("Left hand side of operator ") + t.getLexem() + " is of non-numeric type(" + rightInfo.getType().toString() + "), but " + t.getLexem() + " requires numeric types", t.getLine());
            }
            if(!rightInfo.getType().isNumeric())
			{
				error(String("Right hand side of operator ") + t.getLexem() + " is of non-numeric type(" + rightInfo.getType().toString() + "), but " + t.getLexem() + " requires numeric types", t.getLine());
			}

            info = ExpressionInfo(DataType::mathCast(info.getType(),rightInfo.getType()), info.isConstant() && rightInfo.isConstant());

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

    ExpressionInfo Compiler::castFactor()
	{
		ExpressionInfo info = signedFactor();

		Token t = lexer->peekToken();
		if(t.is(Token::TT_OPERATOR,"->"))
		{
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

			return ExpressionInfo(type, false);
		}

		return info;
	}

    ExpressionInfo Compiler::signedFactor()
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

		ExpressionInfo info = memberFactor();

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

			info = ExpressionInfo(DataType::INT, info.isConstant());
		}

		return info;
	}

    ExpressionInfo Compiler::memberFactor()
    {
        ExpressionInfo info = factor();

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

                info = ExpressionInfo(functionCall(member, true, info.getType()), false);

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

                info = ExpressionInfo(v->getType(), false);

                writeASM("GETMEMBER '" + member.getLexem() + "'");
            }
        }

        return info;
    }

    ExpressionInfo Compiler::factor()
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

            return ExpressionInfo(numberType, true);

        }else if(t.getType() == Token::TT_STRING)
        {
            writeASM("PUSHSTRING '" + escapeASMChars(t.getLexem()) + "'");

           return ExpressionInfo(DataType::STRING, true);

        }else if(t.getType() == Token::TT_IDENT)
        {
            Token t2 = lexer->peekToken();
            if(t2.is(Token::TT_OPERATOR,"("))
            {
                DataType type = functionCall(t,false);

                if(type.equals(DataType::NOTYPE))
                {
                	error("Function '" + t.getLexem() + "' is of type void, but it was used in an expression factor", t.getLine());
                }

                return ExpressionInfo(type, false);

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

                return ExpressionInfo(v->getType(), false);
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
            //TODO: implement NEW here
        }else if(t.is(Token::TT_KEYWORD,"null"))
        {
            writeASM("PUSHNULL");

            return ExpressionInfo(DataType::OBJECT, true); //TODO: Re-think if NULL is really an object

        }else if(t.getType() == Token::TT_EOF)
        {
            error("Unexpected end of file while parsing expression", t.getLine());

        }else
        {
            error("Unexpected token in expression factor: " + t.getLexem(), t.getLine());
        }

        return ExpressionInfo(DataType::NOTYPE, false);
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

    	for(uint16_t i = 0; i < datatypes.size(); i++)
    	{

    		if(t.getLexem().equals(datatypes[i]))
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
    	//TODO: Do stuff with the error message and throw exception
    	throw Exception(String("Error in line ") + line + ": " + s);
    }


    //private class ExpressionInfo

    ExpressionInfo::ExpressionInfo(const DataType &t, bool cons) : type(t), constant(cons)
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
