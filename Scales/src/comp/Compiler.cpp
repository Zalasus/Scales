
#include "comp/Compiler.h"

//TODO: I want this to work from anywhere, so this file has to be compiled before anything that might use this macro
#define SCALES_COMPILER_WRITEASM

#ifdef SCALES_COMPILER_WRITEASM

	#include <iostream>

	#define writeASM(s) std::cout << s << std::endl;

#else

	#define writeASM(s)

#endif


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
			if(t.is(Token::TT_KEYWORD,"script"))
			{
				Token ident = lexer->readToken();

				if(ident.getType() != Token::TT_IDENT)
				{
					error("Expected scriptname after 'script' keyword, but found: " + ident.getLexem(),ident.getLine());
				}

				script(ScriptIdent(nspace, ident.getLexem()));

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

				writeASM("#Namespace set to: " + nspace);

				//asmout.write(0xCB); //Namespace tag
				//asmout.writeSString(nspace);
				//asmout.write(0xFE); //End tag

			}else
			{
				error("Expected script header or namespace definition, but found: " + t.getLexem(), t.getLine());
			}

			t = lexer->readToken();
		}
    }

    void Compiler::script(const ScriptIdent &scriptident)
    {
    	//uint32_t startSize = asmout.getSize();

    	//asmout.write(0xCA); //Class start tag
    	//asmout.writeSString(scriptident.getScriptname()); //Classname
    	//asmout.writeMarker("sizeof_script" + scriptident.getScriptname()); //Class bytecode size

    	Script scr = Script(scriptident);
    	scriptSystem.declareScript(scr);

    	//The Script object is copied by the vector in the scriptsystem. We need a pointer to the version in the vector, so re-fetch the script
    	currentScript = scriptSystem.getScript(scriptident);

    	Token t = lexer->peekToken();

    	writeASM("#-------Script " + scriptident.getScriptname() + " start ");

    	while(!t.is(Token::TT_KEYWORD,"end"))
    	{
    		if(t.getType() == Token::TT_IDENT) // private variable / left eval
    		{

    			if(scriptSystem.isNamespaceDeclared(t.getLexem()) || (scriptSystem.getScript(ScriptIdent("",t.getLexem())) != null)) //private variable
				{

    				variableDec(AccessType::PRIVATE, false, false);

				}else
				{
					leftEval();
				}

    		}else if(isPrimitive(t)) //private variable
    		{
    			variableDec(AccessType::PRIVATE, false, false);

    		}else if(t.is(Token::TT_KEYWORD, "func") || t.is(Token::TT_KEYWORD, "event") || t.is(Token::TT_KEYWORD, "init")) //private function / private event /private constructor
    		{

    			functionDec(AccessType::PRIVATE,false);

    		}else if(t.is(Token::TT_KEYWORD, "native")) //private native function / private native variable
    		{
    			lexer->readToken();
    			t = lexer->peekToken();

    			if(t.getType() == Token::TT_IDENT || isPrimitive(t)) // variable
				{

					variableDec(AccessType::PRIVATE, true, false);

				}else if(t.is(Token::TT_KEYWORD, "func") || t.is(Token::TT_KEYWORD, "event")) //function / event
				{

					functionDec(AccessType::PRIVATE,true);

				}else if(t.is(Token::TT_KEYWORD, "init")) //constructor
				{
					error("Constructors must not be native (This will change. Wait for it)", t.getLine());

				}else
				{
					error("Unexpected token after 'native'-keyword: " + t.getLexem(), t.getLine());
				}

    		}else if(isAccessModifier(t)) //variable / function / constructor
    		{
    			AccessType access = AccessType::byName(t.getLexem());
    			bool native = false;

    			lexer->readToken();
    			t = lexer->peekToken();

    			if(t.is(Token::TT_KEYWORD, "native"))
    			{
    				native = true;

    				lexer->readToken();
    				t = lexer->peekToken();
    			}

    			if(t.getType() == Token::TT_IDENT || isPrimitive(t)) // variable (since there already were keywords typical for a declaration, we don't need to check if the ident really is a type or the beginning of a left eval)
				{

					variableDec(access, native, false);

				}else if(t.is(Token::TT_KEYWORD, "func") || t.is(Token::TT_KEYWORD, "event")) //function / event
				{

					functionDec(access, native);

				}else if(t.is(Token::TT_KEYWORD, "init")) //constructor
				{
					if(native)
					{
						error("Constructors must not be native (This will change. Wait for it)", t.getLine()); //TODO: Allow constructors beeing native
					}

					functionDec(access, false);

				}else
				{
					error("Unexpected token after access modifier: " + t.getLexem(), t.getLine());
				}

    		}else
    		{
    			error("Unexpected token in script block: " + t.getLexem(), t.getLine());
    		}

    		t = lexer->peekToken();
    	}

    	lexer->readToken();

    	writeASM("#-------Script " + scriptident.getScriptname() + " end");

    	//asmout.write(0xFE); //Script block end
    	//asmout.defineMarker("sizeof_script" + scriptident.getScriptname(), asmout.getSize() - startSize); //Write script size to marker defined at start
    }

    BlockIdent::BlockType Compiler::functionBlock(const BlockIdent &block, const DataType &returnType)
	{
    	currentScript->enterLocalScope();

		Token t = lexer->peekToken();

		while(!t.is(Token::TT_KEYWORD,"end"))
		{
			if(t.getType() == Token::TT_IDENT) // private variable / left eval
			{

				if(scriptSystem.isNamespaceDeclared(t.getLexem()) || (scriptSystem.getScript(ScriptIdent("",t.getLexem())) != null)) //private variable
				{

					variableDec(AccessType::PRIVATE, false, true);

				}else
				{
					leftEval();
				}

			}else if(isPrimitive(t)) //private variable
			{
				variableDec(AccessType::PRIVATE, false, true);

			}else if(isAccessModifier(t) || t.is(Token::TT_KEYWORD, "native") || t.is(Token::TT_KEYWORD, "static"))
			{
				error("No variable modifiers allowed in function block", t.getLine());

			}else if(t.is(Token::TT_KEYWORD, "begin"))
			{
				lexer->readToken();

				uint32_t currentSubBlockUID = getNewUID();

				writeASM("BEGIN");

				functionBlock(BlockIdent(BlockIdent::BT_SUB, currentSubBlockUID), returnType);

				writeASM("END");

			}else if(t.is(Token::TT_KEYWORD, "return"))
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

					//Currently unsupported, should never occur anyhow TODO: We should at least do some error processing here

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

		currentScript->leaveLocalScope();

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

	void Compiler::variableDec(const AccessType &accessType, bool native, bool local)
	{
		DataType type = dataType();

		Token ident = lexer->readToken();

		if(ident.getType() != Token::TT_IDENT)
		{
			error("Expected variable name after data type, but found: " + ident.getLexem(), ident.getLine());
		}

		if(scriptSystem.isNamespaceDeclared(ident.getLexem()) || (scriptSystem.getScript(ScriptIdent("",ident.getLexem())) != null))
		{
			error("Variable name '" + ident.getLexem() + "' is already occupied by a script/namespace", ident.getLine());
		}

		if(!local && (currentScript->getGlobal(ident.getLexem()) != null))
		{
			//Globals must not override globals
			error("Double declaration of variable " + ident.getLexem() + " in global scope", ident.getLine());

		}else if(local && (currentScript->getLocal(ident.getLexem()) != null))
		{
			//Locals must not override locals
			error("Double declaration of variable " + ident.getLexem() + " in local scope", ident.getLine());
		}


		Token t = lexer->readToken();

		VariablePrototype v = VariablePrototype(ident.getLexem(), type, accessType);

		if(local)
		{
			currentScript->declareLocal(v);
		}else
		{
			currentScript->declareGlobal(v);
		}

		//Scope is determined by callstack during runtime
		writeASM("DECLAREVAR '" + ident.getLexem() + "'," + (int)type.getTypeID() + "," + (int)accessType.getTypeID()); //TODO: Object specifier has to be included, oh and also the native flag

		if(t.is(Token::TT_OPERATOR, "="))
		{
			ExpressionInfo info = expression();

			if(!info.getType().canCastImplicitlyTo(type))
			{
				error("Type mismatch in variable initialization: Can not implicitly cast " + info.getType().toString() + " to " + type.toString(), t.getLine());
			}

			t = lexer->readToken();

			if(!t.is(Token::TT_OPERATOR, ";"))
			{
				error("Expected semicolon after variable initialization, but found: " + t.getLexem(), t.getLine());
			}

			writeASM("POPVAR '" + ident.getLexem() + "'");

		}else if(!t.is(Token::TT_OPERATOR, ";"))
		{
			error("Expected semicolon or assignment after variable declaration, but found: " + t.getLexem(), t.getLine());
		}
	}

    void Compiler::functionDec(const AccessType &accessType, bool native)
    {
    	//TODO: Clean up this function; see comment in constructorDec about redundant code
    	Function::FunctionType type;

    	Token t = lexer->readToken();

    	if(t.is(Token::TT_KEYWORD, "event"))
    	{
    		type = Function::FT_EVENT;

    	}else if(t.is(Token::TT_KEYWORD, "init"))
    	{
    		type = Function::FT_CONSTRUCTOR;

    	}else if(t.is(Token::TT_KEYWORD, "func"))
    	{
    		type = Function::FT_NORMAL;

    	}else
    	{
    		error("Called functionDec for invalid position in token stream (expected recognizer func, init or event)", t.getLine());
    	}

    	DataType returnType = DataType::NOTYPE;
    	if(type == Function::FT_NORMAL)
    	{
    		t = lexer->peekToken();

    		if(!t.is(Token::TT_KEYWORD, "void"))
    		{
    			returnType = dataType();

    		}else
    		{
    			lexer->readToken();
    		}
    	}


		Token ident = (type == Function::FT_CONSTRUCTOR) ? t : lexer->readToken(); //The ident of a constructor is it's recognizer (init)

		if(type != Function::FT_CONSTRUCTOR)
		{
			if(ident.getType() != Token::TT_IDENT)
			{
				if(type == Function::FT_EVENT)
				{
					error("Expected event name after 'event' keyword, but found: " + ident.getLexem(), ident.getLine());
				}else
				{
					error("Expected function name after function return type, but found: " + ident.getLexem(), ident.getLine());
				}
			}

			//TODO: We could replace this thing (also the checking for TT_IDENT) by a new processor function name();
			if(scriptSystem.isNamespaceDeclared(ident.getLexem()) || (scriptSystem.getScript(ScriptIdent("",ident.getLexem())) != null))
			{
				error("Function name '" + ident.getLexem() + "' is already occupied by a script/namespace", ident.getLine());
			}
		}

		if(type == Function::FT_EVENT && !accessType.equals(AccessType::PUBLIC))
		{
			error("Only public access type is applicable for events", ident.getLine());
		}

		t = lexer->readToken();

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
				DataType type = dataType();

				paramTypes.push_back(type);

				t = lexer->readToken();
				if(t.getType() != Token::TT_IDENT)
				{
					error("Expected variable name in parameter list, but found: " + t.getLexem(), t.getLine());
				}

				if(scriptSystem.isNamespaceDeclared(t.getLexem()) || (scriptSystem.getScript(ScriptIdent("",t.getLexem())) != null))
				{
					error("Parameter name '" + t.getLexem() + "' is already occupied by a script/namespace", t.getLine());
				}

				paramNames.push_back(t.getLexem());

				t = lexer->readToken();

				if(!(t.is(Token::TT_OPERATOR,",") || (t.is(Token::TT_OPERATOR,")"))))
				{
					error("Expected , or closing parentheses in parameter list, but found: " + t.getLexem(), t.getLine());
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

		Function func = Function(ident.getLexem(), paramTypes, returnType, accessType, native, type, 0); //TODO: Change adress during assembly
		currentScript->declareFunction(func);

		if(native)
		{
			t = lexer->readToken();
			if(!t.is(Token::TT_OPERATOR, ";"))
			{
				error(String("Native functions must not have a body. Expected semicolon after parameter list") ,t.getLine());
			}

			return;

			//TODO: Natives also have to be included in bytecode!
		}

		String defInstr;

		if(type == Function::FT_EVENT)
		{
			defInstr = String("REGISTEREVENT '") + ident.getLexem() + "'," + (String("") + (int32_t)paramNames.size()) + ",";

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

			//Declare locals prototypes
			VariablePrototype v = VariablePrototype(paraName, paraType, AccessType::PRIVATE); //Parameters are always private
			currentScript->declareLocal(v);

			//Declare parameter variables...
			writeASM(String("DECLAREVAR ") + paraType.getTypeID() + ",'" + paraName + "'," + (int)AccessType::PRIVATE.getTypeID());
			//...and get their values from the stack
			writeASM("POPVAR '" + paraName + "'");
		}

		functionBlock(BlockIdent(BlockIdent::BT_FUNC,currentFunctionUID), returnType);

		//All locals now have lost their validity, and since we are on a single-pass compiler, we are never going to see that function again, so we can delete all the prototypes
		currentScript->destroyAllLocals();

		writeASM("END");

		writeASM("func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_end:");
    }

    /*void Compiler::constructorDec(const AccessType &accessType)
	{
    	//TODO: Much, much redundant code with functionDec. Find way to merge both funcs

    	lexer->readToken(); //consume constructor recognizer
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

				DataType type = dataType();

				paramTypes.push_back(type);

				t = lexer->readToken();
				if(t.getType() != Token::TT_IDENT)
				{
					error("Expected variable name in parameter list, but found: " + t.getLexem(), t.getLine());
				}

				if(scriptSystem.isNamespaceDeclared(t.getLexem()) || (scriptSystem.getScript(ScriptIdent("",t.getLexem())) != null))
				{
					error("Parameter name '" + t.getLexem() + "' is already occupied by a script/namespace", t.getLine());
				}

				paramNames.push_back(t.getLexem());

				t = lexer->readToken();

				if(!(t.is(Token::TT_OPERATOR,",") || (t.is(Token::TT_OPERATOR,")"))))
				{
					error("Expected , or closing parentheses in parameter list, but found: " + t.getLexem(), t.getLine());
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

			VariablePrototype v = VariablePrototype(paraName, paraType, AccessType::PRIVATE); //Parameters are always private
			currentScript->declareLocal(v);

			writeASM(String("DECLAREVAR ") + paraType.getTypeID() + ",'" + paraName + "'," + (int)AccessType::PRIVATE.getTypeID());

			writeASM("POPVAR '" + paraName + "'");
		}

		functionBlock(BlockIdent(BlockIdent::BT_FUNC,currentFunctionUID), DataType::NOTYPE);

		currentScript->destroyAllLocals();

		writeASM("END");

		writeASM(String("constructor_uid") + currentFunctionUID + "_end:");
	}*/

    DataType Compiler::functionCall(const String &funcName, bool member, const DataType &baseType)
    {
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
			if(baseType.getTypeID() != DataType::OBJECT.getTypeID())
			{
				error("Only object types have callable member functions, but given type was: " + baseType.toString(), t.getLine());
			}

			Script *script = scriptSystem.getScript(baseType.getObjectType());

			if(script == null)
			{
				error("Specifier of given base type for member function call points to non-existent script: " + baseType.toString(), t.getLine());
			}

			Function *function = script->getFunction(funcName, paramTypes);

			if(function == null)
			{
				error("Function '" + Function::createInfoString(funcName, paramTypes) + "' was not defined in given object type script '" + baseType.toString() + "'", t.getLine());
			}
			
			if(function->getAccessType().equals(AccessType::PRIVATE))
			{
				error("Function '" + Function::createInfoString(funcName, paramTypes) + "' in given object type script '" + baseType.toString() + "' is private", t.getLine());
			}

			writeASM("CALLMEMBER '" + funcName + "'," + paramCount);

			return function->getReturnType();

		}else
		{
			Function *function = currentScript->getFunction(funcName, paramTypes);

			if(function == null)
			{
				error("Function '" + Function::createInfoString(funcName, paramTypes) + "' was not defined in current script", t.getLine());
			}

			//No need to check for private, as we are calling from the same script
			
			writeASM("CALL '" + funcName + "'," + paramCount);

			return function->getReturnType();
		}
    }

    DataType Compiler::dataType()
    {
    	Token t = lexer->readToken();

    	if(isPrimitive(t))
    	{

    		return DataType::byName(t.getLexem());

    	}else if(t.getType() == Token::TT_IDENT)
    	{
    		Token t2 = lexer->peekToken();

    		if(t2.is(Token::TT_OPERATOR, ":"))
    		{
    			if(!scriptSystem.isNamespaceDeclared(t.getLexem()))
    			{
    				error("Identifier '" + t.getLexem() + "' does not name a namespace", t.getLine());
    			}

    			lexer->readToken();
    			t2 = lexer->readToken();

    			if(t2.getType() != Token::TT_IDENT)
    			{
    				error("Expected script identifier after namespace operator, but found: " + t2.getLexem(), t2.getLine());
    			}

    			if(scriptSystem.getScript(ScriptIdent(t.getLexem(), t2.getLexem())) == null)
    			{
    				error("Identifier '" + t2.getLexem() + "' does not name a script in namespace '" + t.getLexem() + "'", t2.getLine());
    			}

    			DataType type = DataType::OBJECT;
    			type.initObjectType(ScriptIdent(t.getLexem(), t2.getLexem()));

    			return type;

    		}else
    		{
    			if(scriptSystem.getScript(ScriptIdent("", t.getLexem())) == null)
    			{
    				error("Identifier '" + t.getLexem() + "' does not name a script in default namespace", t.getLine());
    			}

    			DataType type = DataType::OBJECT;
    			type.initObjectType(ScriptIdent("", t.getLexem()));

    			return type;
    		}

    	}else
    	{
    		error("Expected primitve data type or script identifier, but found: " + t.getLexem(), t.getLine());
    	}

    	return DataType::NOTYPE; //should never happen

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

			DataType type = dataType();

			if(type.equals(DataType::OBJECT))
			{
				writeASM(String("TOOBJECT '") + type.getObjectType().getNamespace() + "','" + type.getObjectType().getScriptname() + "'");

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
		bool toVal = false;

		Token t = lexer->peekToken();
		if(t.is(Token::TT_OPERATOR,"-"))
		{
			negate = true;
			lexer->readToken();

		}else if(t.is(Token::TT_OPERATOR,"!"))
		{
			invert = true;
			lexer->readToken();

		}else if(t.is(Token::TT_OPERATOR,"$"))
		{
			toVal = true;
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

		if(toVal)
		{
			writeASM("DEREFER");

			info = ExpressionInfo(info.getType(), info.isConstant(), ExpressionInfo::FT_MATH_EXPR); //TODO: I have a bad feeling about this. Maybe re-think if all the values are correct
		}

		return info;
	}

    ExpressionInfo Compiler::memberFactor(const bool leftEval)
    {
        ExpressionInfo info = factor(leftEval);

        Token t = lexer->peekToken();
        while(t.is(Token::TT_OPERATOR,"."))
        {
        	if(info.getType().getTypeID() != DataType::OBJECT.getTypeID())
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
                DataType ftype = functionCall(member.getLexem(), true, info.getType());

                if(ftype.equals(DataType::NOTYPE) && !leftEval)
                {
                	error("Function '" + member.getLexem() + "' of script '" + info.getType().toString()  + "' is of type void, but it was used in a value-context", member.getLine());
                }

                info = ExpressionInfo(ftype, false, ExpressionInfo::FT_FUNCTION_RETURN);

            }else
            {
            	Script *s = scriptSystem.getScript(info.getType().getObjectType());
				if(s == null)
				{
					error("Specifier of given object type points to non-existent script '" + info.getType().toString() + "'", member.getLine());
				}

				VariablePrototype *v = s->getGlobal(member.getLexem()); //No need to access locals
				if(v == null)
				{
					//No need to check for universals here, as they can not be referenced through objects

					error("Referenced script '" + info.getType().toString() + "' has no member '" + member.getLexem() + "'", member.getLine());
				}

				if(v->getAccessType().equals(AccessType::PRIVATE) && !s->getIdent().equals(currentScript->getIdent()))
				{
					//If a private object member is accessed from a script of the same type as the object, the access modifier is ignored (like all script methods are declared as friend)

					error("Variable '" + member.getLexem() + "' in referenced script '" + info.getType().toString() + "' is private", member.getLine());
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
                DataType type = functionCall(t.getLexem(), false);

                if(type.equals(DataType::NOTYPE) && !leftEval)
                {
                	error("Function '" + t.getLexem() + "' is of type void, but it was used in a value-context", t.getLine());
                }

                return ExpressionInfo(type, false, ExpressionInfo::FT_FUNCTION_RETURN);

            }else if(t2.is(Token::TT_OPERATOR, "["))
            {
            	VariablePrototype *v = getVariableInScript(currentScript, t.getLexem());

            	lexer->readToken();

            	expression();

            	writeASM("GETINDEX");

            	t2 = lexer->readToken();
				if(!t2.is(Token::TT_OPERATOR, "]"))
				{
					error("Missing closing square brackets after array index expression",t.getLine());
				}

				return ExpressionInfo(v->getType(), false, ExpressionInfo::FT_VARIABLE_REF);

            }else
            {
            	VariablePrototype *v = getVariableInScript(currentScript, t.getLexem());

				if(v == null)
				{
					error("Variable/type '" + t.getLexem() + "' was not declared in this scope", t.getLine());
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

        }/*else if(t.is(Token::TT_OPERATOR,"["))
        {
            DataType type = arrayLiteral();

            Token t2 = lexer->readToken();
            if(!t2.is(Token::TT_OPERATOR,")"))
            {
                error("Missing closing square brackets after array literal expression",t.getLine());
            }

            return ExpressionInfo(type, false, ExpressionInfo::FT_LITERAL);

        }*/else if(t.is(Token::TT_KEYWORD,"new"))
        {
            DataType scripttype = dataType();

            if(scripttype.getTypeID() != DataType::OBJECT.getTypeID())
            {
            	error("Expected object type identifier after new keyword, but found: " + scripttype.getTypeName(), t.getLine());
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
			
			Script *script = scriptSystem.getScript(scripttype.getObjectType());
			if(script == null)
			{
				error("Given identifier '" + scripttype.toString() + "' in new statement does not name a type", t2.getLine());
			}
			
			Function *constr = script->getFunction("init", paramTypes);
			
			if(constr == null)
			{
				error("Constructor " + Function::createInfoString(scripttype.toString(), paramTypes) + " was not declared", t2.getLine());
				
			}else
			{
				if(constr->getAccessType().equals(AccessType::PRIVATE))
				{
					error("Constructor " + Function::createInfoString(scripttype.toString(), paramTypes) + " is private", t2.getLine());
				}
			
				writeASM("NEW '" + scripttype.getObjectType().getNamespace() + "','" + scripttype.getObjectType().getScriptname() + "'," + paramCount);
			}

			return ExpressionInfo(scripttype, false, ExpressionInfo::FT_FUNCTION_RETURN); //Constructors are considered as functions for now

        }else if(t.is(Token::TT_KEYWORD,"null"))
        {
            writeASM("PUSHNULL");

            return ExpressionInfo(DataType::OBJECT, true, ExpressionInfo::FT_LITERAL); //TODO: Re-think if NULL is really an object

        }else if(t.is(Token::TT_KEYWORD,"this"))
        {
        	//TODO: Private variables in "this" are currently not accessible, this has to be changed
        	//Note: the public API for scripts should not give any access to locals. Same goes for the one used for variable type stuff during compile time
        	//so this.xyz won't let the program access locals. (Functions where locals have same name as globals which should still be accessed can be by using 'this')

        	t = lexer->peekToken();

        	if(t.is(Token::TT_OPERATOR, "("))
        	{
        		functionCall("init",false); //Calling a constructor of the current script

        		return ExpressionInfo(DataType::NOTYPE, true, ExpressionInfo::FT_FUNCTION_RETURN); //Constructors do not return anything

        	}else
        	{

        		writeASM("PUSHTHIS");

        		DataType type = DataType::OBJECT;
        		type.initObjectType(currentScript->getIdent());

        		return ExpressionInfo(type, false, ExpressionInfo::FT_LITERAL); //Although "PUSHTHIS" actually creates a reference, returning FT_VARIABLE_REF would allow the program to assign to it
        	}

        }else if(t.getType() == Token::TT_EOF)
        {
            error("Unexpected end of file while parsing expression", t.getLine());

        }else
        {
            error("Unexpected token in expression factor: " + t.getLexem(), t.getLine());
        }

        return ExpressionInfo(DataType::NOTYPE, false, ExpressionInfo::FT_LITERAL); //Should never happen
    }

    /*ArrayType Compiler::arrayLiteral()
    {
    	Token t = lexer->peekToken();
    	DataType *type = null;
    	uint32_t elementCount = 0;

    	while(!t.is(Token::TT_OPERATOR, "]"))
    	{
    		ExpressionInfo info = expression();

    		if(elementCount == 0)
    		{
    			type = info.getType();

    		}else
    		{
    			if(!type->equals(info.getType()))
    			{
    				error("Type mismatch in array literal: First element in list was of type " + type->toString() + ", but element #" + (int)elementCount + " was of type " + info.getType().toString() + " (implicit casts in array literals are not yet allowed)", lexer->getCurrentLine());
    			}
    		}

    		t = lexer->readToken();
    		if(!t.is(Token::TT_OPERATOR, ",") && !t.is(Token::TT_OPERATOR, "]"))
    		{
    			error("Expected ',' after array literal element #" + (int)elementCount + ", but found: " + t.getLexem(), t.getLine());
    		}

    		elementCount++;
    	}

    	return ArrayType(&type, elementCount);
    }*/

    VariablePrototype *Compiler::getVariableInScript(Script *s, const String &name)
    {
    	VariablePrototype *v = s->getLocal(name);

		if(v == null)
		{
			v = s->getGlobal(name);
		}

		return v;

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
    	return t.is(Token::TT_KEYWORD, "public") || t.is(Token::TT_KEYWORD, "private");
    }

    bool Compiler::isPrimitive(const Token &t)
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
			"string"
	};
	const uint32_t Compiler::KEYWORD_COUNT = sizeof(KEYWORDS)/sizeof(String);

	const String Compiler::DATATYPES[] =
	{
			"int",
			"long",
			"float",
			"double",
			"string"
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
			";",
			":",
			"$"
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
