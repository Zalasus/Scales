
#include "comp/Compiler.h"


#ifdef SCALES_COMPILER_WRITEASM

	#include <iostream>

	#define writeASM(s) std::cout << s << std::endl;

#else

	#define writeASM(s)

#endif


#define VF_LOCAL 1
#define VF_PRIVATE 2
#define VF_NATIVE 4
#define VF_STATIC 8
#define VF_CONSTRUCTOR 16
#define VF_PUBLIC 32 //this is not used for identifying public variables, just for error detection

namespace Scales
{

	//TODO: Performance optimization!!! This whole code looks like its your first C++ project (which is kind of true). And try to reduce executable size. We passed 1MB already and that was in v0.2
	//(okay, I have to admit, the 1MB were in Debug configuration. In Release it's just about 200kB. But anyway: Keep it small! 847kB at max!)

	//public class compiler

	//TODO: check for double declarations (not data type double but... ah, you know what I mean)
	//TODO: currently every single variable declaration takes up one pointer in the varmem forever, even if it leaves scope somewehere. this might be optimized.
	
    Compiler::Compiler()
    :
    		scalesSystem(null),
    		currentClassProto(null),
    		currentFunctionProto(null),
    		lastUID(0),
    		lexer(KEYWORDS, KEYWORD_COUNT, OPERATORS, OPERATOR_COUNT, true)
    {
    }

    Compiler::~Compiler()
    {
    	delete currentClassProto; //This one is copied; we can delete it for safety
    }

    Library Compiler::compile(std::istream *in, ScalesSystem *ss)
    {
    	Library lib;

    	lexer.setDataSource(in);
    	scalesSystem = ss;

    	mainBlock(lib);

    	return lib;
    }

    void Compiler::mainBlock(Library &lib)
    {
    	String nspace = "";

    	Token t = lexer.readToken();

		while(!t.isType(Token::TT_EOF))
		{
			if(t.is(Token::TT_KEYWORD,"class"))
			{
				classDef(nspace);

				lib.add(*currentClassProto); //Add the newly created class prototype to the library (it is copied) TODO: If this is null, shit gets real
				delete currentClassProto; //We need to delete it before doing another run TODO: This is really bad style (RAII and so on). Try to improve this later (see comment above)
				currentClassProto = null; //Set it to null so the destructor doesn't delete non-existent instances

			}else if(t.is(Token::TT_KEYWORD,"namespace"))
			{
				t = lexer.readToken();

				if(t.is(Token::TT_KEYWORD, "default"))
				{
					nspace = String("");

				}else if(t.isType(Token::TT_IDENT))
				{
					nspace = t.getLexem();

				}else
				{
					error("Expected namespace identifier or 'default' after namespace keyword, but found: " + t.getLexem(), t.getLine());
				}

				t = lexer.readToken();

				if(!t.is(Token::TT_OPERATOR,";"))
				{
					error("Expected semicolon after namespace declaration, but found: " + t.getLexem(), t.getLine());
				}

				writeASM("#Namespace set to: " + nspace);

			}else
			{
				error("Expected script header or namespace definition, but found: " + t.getLexem(), t.getLine());
			}

			t = lexer.readToken();
		}
    }

    void Compiler::classDef(const String &nspace)
    {
    	Token t = lexer.readToken();
		if(!t.isType(Token::TT_IDENT))
		{
			error("Expected classname after 'class' keyword, but found: " + t.getLexem(), t.getLine());
		}

		ClassId id = ClassId(nspace, t.getLexem());
		String nativeLink;
		ClassPrototype *superclass = null;

		t = lexer.peekToken();
		if(t.is(Token::TT_KEYWORD, "extends"))
		{
			lexer.readToken(); //Consume the keyword

			t = lexer.peekToken();
			if(!t.isType(Token::TT_IDENT))
			{
				error("Expected class identifier after extends keyword, but found: " + t.getLexem(), t.getLine());
			}

			ClassId cid = classId();

			superclass = lookupClassPrototype(cid);

			t = lexer.peekToken();
		}

		if(t.is(Token::TT_KEYWORD, "links"))
		{
			lexer.readToken(); //Consume the "links" keyword

			t = lexer.readToken();
			if(t.getType() != Token::TT_IDENT)
			{
				error("Expected native link unit name after links keyword, but found: " + t.getLexem(), t.getLine());
			}

			nativeLink = t.getLexem();

			t = lexer.peekToken();
		}

		currentClassProto = new ClassSketch(id, superclass, nativeLink); //TODO: RAII!!! This is not Java!!!

    	writeASM("#-------Class " + id.toString() + " start ");

    	while(!t.is(Token::TT_KEYWORD,"end"))
    	{
    		if(t.getType() == Token::TT_IDENT) // private variable / left eval
    		{

    			if((namespaceExists(t.getLexem()) || lookupClassPrototypeByName(t.getLexem()) != null)) //private variable
				{

    				variableDec(VF_PRIVATE, Scope::GLOBAL);

				}else
				{
					leftEval();
				}

    		}else if(isPrimitive(t)) //private variable
    		{
    			variableDec(VF_PRIVATE, Scope::GLOBAL);

    		}else if(t.is(Token::TT_KEYWORD, "func")) //private function
    		{

    			functionDec(VF_PRIVATE);

    		}else if(t.is(Token::TT_KEYWORD, "init")) //private constructor
    		{
    			functionDec(VF_PRIVATE | VF_CONSTRUCTOR);

    		}else if(isModifier(t)) //variable / function with modifiers
    		{
    			uint32_t flags = 0;

    			do
    			{
    				uint32_t flagToBeSet = 0;

    				if(t.is(Token::TT_KEYWORD, "private"))
    				{

    					flagToBeSet = VF_PRIVATE;

    				}else if(t.is(Token::TT_KEYWORD, "public"))
    				{

    					flagToBeSet = VF_PUBLIC;

    				}else if(t.is(Token::TT_KEYWORD, "static"))
    				{

						flagToBeSet = VF_STATIC;

    				}else if(t.is(Token::TT_KEYWORD, "native"))
    				{

    					flagToBeSet = VF_NATIVE;

    				}else
    				{
    					error("[COMPILER BUG] Unknown modifier " + t.getLexem(), t.getLine());
    				}


    				if(flags & flagToBeSet)
    				{
    					error("Each modifier only allowed once per element.", t.getLine());
    				}

    				flags |= flagToBeSet;

    			}while(isModifier(t));


    			if((flags & VF_PUBLIC) && (flags & VF_PRIVATE))
				{
					error("Conflicting access modifiers. Element was declared both public and private.", t.getLine());
				}

    			if((flags & VF_STATIC) && !(flags & VF_NATIVE))
				{
					error("Statics are only valid when native.", t.getLine());
				}

    			//Now that we have collected the flags we need to check what king of element we are declaring/defining
    			if(t.getType() == Token::TT_IDENT || isPrimitive(t))
				{
    				//After modifier keywords, no expressions are allowed. Therefore we assume a declaration here (otherwise the ident could be a leftEval)

					variableDec(flags, Scope::GLOBAL);

				}else if(t.is(Token::TT_KEYWORD, "func")) //function
				{

					functionDec(flags);

				}else if(t.is(Token::TT_KEYWORD, "init")) //constructor
				{
					functionDec(flags | VF_CONSTRUCTOR);

				}else
				{
					error("Expected function/variable or constructor declaration after modifiers, but found: " + t.getLexem(), t.getLine());
				}

    		}else if(t.is(Token::TT_KEYWORD, "using"))
    		{
    			lexer.readToken();

    			usingStatement();

    		}else
    		{
    			error("Unexpected token in class block: " + t.getLexem(), t.getLine());
    		}

    		t = lexer.peekToken();
    	}

    	lexer.readToken();

    	writeASM("#-------Class " + id.getClassname() + " end");

    	if(asmout.hasUndefinedMarkers())
		{
			error("Fatal error: Compiler generated invalid bytecode. Markers were requested but not defined. This is most likely a compiler bug.", 0);
		}

    	asmout.reset();
    }

    BlockType Compiler::block(BlockType blockType, const Scope &scope)
	{
    	uint32_t blocksInThisBlock = 0;


		Token t = lexer.peekToken();

		while(!t.is(Token::TT_KEYWORD,"end"))
		{
			if(t.getType() == Token::TT_IDENT) // private variable / left eval
			{

				if((namespaceExists(t.getLexem()) || lookupClassPrototypeByName(t.getLexem()) != null)) //private variable
				{

					variableDec(VF_LOCAL | VF_PRIVATE, scope);

				}else
				{
					leftEval(); //If the ident is no type, it is the beginning of an expression or an error
				}

			}else if(isPrimitive(t)) //private variable
			{
				variableDec(VF_LOCAL | VF_PRIVATE, scope);

			}else if(isAccessModifier(t) || t.is(Token::TT_KEYWORD, "native") || t.is(Token::TT_KEYWORD, "static"))
			{
				error("No variable modifiers allowed in function block", t.getLine());

			}else if(t.is(Token::TT_KEYWORD, "begin"))
			{
				lexer.readToken();

				writeASM("BEGIN");
				asmout << OP_BEGIN;

				block(BT_SUB, Scope(scope.getNestId() + 1, blocksInThisBlock++, getNewUID()));

				writeASM("END");
				asmout << OP_END;

			}else if(t.is(Token::TT_KEYWORD, "return"))
			{
				lexer.readToken();

				t = lexer.peekToken();

				if(t.is(Token::TT_OPERATOR, ";"))
				{
					lexer.readToken();

				}else
				{
					if(currentFunctionProto->getReturnType() == DataType::DTB_NOTYPE)
					{
						error("Can not return a value in a void function/event. Expected semicolon after 'return' keyword", t.getLine());
					}

					ExpressionInfo info = expression(true);

					if(!DataType::canCastImplicitly(info.getType(), currentFunctionProto->getReturnType()))
					{
						error("Type mismatch in return statement: Can not implicitly cast " + info.getType().toString() + " to " + currentFunctionProto->getReturnType().toString(), t.getLine());
					}

					t = lexer.readToken();
					if(!t.is(Token::TT_OPERATOR, ";"))
					{
						error("Expected semicolon after return expression, but found: " + t.getLexem(), t.getLine());
					}
				}

				writeASM("RETURN");
				asmout << OP_RETURN;

			}else if(t.is(Token::TT_KEYWORD, "while"))
			{
				lexer.readToken();

				int32_t currentWhileUID = getNewUID();

				writeASM(String("while_uid") + currentWhileUID + "_start:");
				asmout.defineMarker(String("while_uid") + currentWhileUID + "_start");

				ExpressionInfo info = expression(true);
				if(!info.getType().isNumeric())
				{
					error("Loop condition in while statement must be numeric, but non-numeric type " + info.getType().toString() + " was given", t.getLine());
				}

				writeASM(StringUtils::append("JUMPFALSE while_uid", currentWhileUID) + "_end");
				asmout << OP_JUMPFALSE;
				asmout.writeMarker(StringUtils::append("while_uid", currentWhileUID) + "_end");

				writeASM("BEGIN");
				asmout << OP_BEGIN;

				block(BT_WHILE, Scope(scope.getNestId() + 1, blocksInThisBlock++, currentWhileUID));

				writeASM("END");
				asmout << OP_END;

				writeASM(StringUtils::append("JUMP while_uid", currentWhileUID) + "_start");
				asmout << OP_JUMP;
				asmout.writeMarker(StringUtils::append("while_uid", currentWhileUID) + "_start");

				writeASM(StringUtils::append("while_uid", currentWhileUID) + "_end:");
				asmout.defineMarker(StringUtils::append("while_uid", currentWhileUID) + "_end");

			}else if(t.is(Token::TT_KEYWORD, "if"))
			{
				lexer.readToken();

				ifStatement(blockType, scope, blocksInThisBlock);

			}else if(t.is(Token::TT_KEYWORD, "else"))
			{
				lexer.readToken();

				if(blockType == BT_IF)
				{

					return BT_ELSE; //Returns to ifStatement(), further processing happens there

				}else if(blockType == BT_ELSEIF)
				{

					//Should never occur. Don't take it seriously
					error("[Compiler bug] That isn't going to do you any good, Flynn.", t.getLine());

				}else
				{
					error("Else-block must be part of if- or elseif-block", t.getLine());
				}

			}else if(t.is(Token::TT_KEYWORD, "elseif"))
			{
				lexer.readToken();

				if(blockType == BT_IF)
				{

					return BT_ELSEIF; //Returns to ifStatement(), further processing happens there

				}else if(blockType == BT_ELSEIF)
				{

					//Should never occur. Don't take it seriously
					error("[Compiler bug] I'm sorry, Dave. I'm afraid I can't do that.", t.getLine());

				}else if(blockType == BT_ELSE)
				{

					error("Else-block must be defined before else-block", t.getLine());

				}else
				{
					error("Elseif-block must be part of if-block", t.getLine());
				}

			}else if(t.is(Token::TT_KEYWORD, "this") || t.is(Token::TT_KEYWORD, "new") || t.is(Token::TT_OPERATOR, "(")) //start of expression
			{
				//this, new and openening parentheses may open expressions. If not, they are errors and detected as such by the expression parser

				leftEval();

			}else if(t.getType() == Token::TT_EOF)
			{
				error("Unexpected end of file in function block", t.getLine());

			}else
			{
				error("Unexpected token in function block: " + t.getLexem(), t.getLine());
			}

			t = lexer.peekToken();
		}

		lexer.readToken();

		return BT_FUNC; //Regular processing
	}

    void Compiler::ifStatement(const BlockType &blockType, const Scope &scope, uint32_t &blocksInThisBlock)
    {
    	//Note: The concept of if block parsing was developed back in KniftoScript2 days, and I have completely forgotten if there
    	//were any better solutions that didn't make it into the implementation. Anyway, the current one messes up the whole compiler.
    	//Although this code reminds me of the wonderful holiday I had when I was thinking about advanced If blocks how they are right now, we might
    	//improve this so the code becomes more structured and clearer. TODO: Maybe improve if block parsing

    	//And also: TODO: I have a bad feeling about this function. Check if everything would work as intended

    	ExpressionInfo info = expression(true);
		if(!info.getType().isNumeric())
		{
			error("Condition in if statement must be numeric, but non-numeric type " + info.getType().toString() + " was given", lexer.getCurrentLine());
		}

		uint32_t currentIfUID = getNewUID();

		writeASM(StringUtils::append("JUMPFALSE if_uid", currentIfUID) + "_end");
		asmout << OP_JUMPFALSE;
		asmout.writeMarker(StringUtils::append("if_uid", currentIfUID) + "_end");
		writeASM("BEGIN");
		asmout << OP_BEGIN;

		BlockType bt = block(BT_IF, Scope(scope.getNestId()+1, blocksInThisBlock++, currentIfUID));

		writeASM("END");
		asmout << OP_END;

		if(bt == BT_ELSE || bt == BT_ELSEIF)
		{
			writeASM(StringUtils::append("JUMP if_else_uid", currentIfUID) + "_end");
		}

		writeASM(String("if_uid") + currentIfUID + "_end:");
		asmout.defineMarker(String("if_uid") + currentIfUID + "_end");

		if(bt == BT_ELSE || bt == BT_ELSEIF)
		{
			writeASM(String("# if_else_uid") + currentIfUID + "_start:");

			if(bt == BT_ELSE)
			{
				writeASM("BEGIN");
				asmout << OP_BEGIN;

				block(BT_ELSE, Scope(scope.getNestId(), blocksInThisBlock++, getNewUID()));

				writeASM("END");
				asmout << OP_END;
			}else
			{
				ifStatement(blockType, scope, blocksInThisBlock);
			}

			writeASM(String("if_else_uid") + currentIfUID + "_end:");
			asmout.defineMarker(String("if_else_uid") + currentIfUID + "_end");
		}
    }

    void Compiler::usingStatement()
    {
    	Token t = lexer.peekToken();

    	if(t.is(Token::TT_KEYWORD, "namespace"))
    	{
    		error("Namespace imports currently unsupported", t.getLine());
    	}

    	if(t.getType() != Token::TT_IDENT)
    	{
    		error("Expected namespace identifier after using keyword, but found: " + t.getLexem(), t.getLine());
    	}

    	DataType type = dataType();

    	if(type.getTypeBase() != DataType::DTB_OBJECT)
    	{
    		error(String("Expected class identifier after using keyword."), t.getLine());
    	}

    	t = lexer.readToken();

    	if(!t.is(Token::TT_OPERATOR, ";"))
    	{
    		error("Expected semicolon after using statement, but found: " + t.getLexem(), t.getLine());
    	}

    	//TODO: Store the retrieved class somewhere for lookup
    }

    void Compiler::leftEval()
	{
    	ExpressionInfo left = expression(false);

    	Token t = lexer.readToken();
    	if(!t.is(Token::TT_OPERATOR, ";"))
    	{
    		error("Expected semicolon after expression, but found: " + t.getLexem(), t.getLine());
    	}

    	if(left.getFactorType() != ExpressionInfo::FT_VOID)
    	{
    		writeASM("DISCARD");
    		asmout << OP_DISCARD;
    	}
	}

	void Compiler::variableDec(uint32_t varFlags, const Scope &currentScope)
	{
		DataType type = dataType();

		Token ident = name();

		if(!(varFlags & VF_LOCAL) && (currentClassProto->getGlobalPrototype(ident.getLexem(), true) != null))
		{
			//Globals must not override globals or overridden globals
			error("Double declaration of variable " + ident.getLexem() + " in global scope", ident.getLine());

		}else if((varFlags & VF_LOCAL) && (currentFunctionProto->getLocalPrototype(ident.getLexem(), currentScope) != null))
		{
			//Locals must not override locals
			error("Double declaration of variable " + ident.getLexem() + " in local scope", ident.getLine());
		}


		Token t = lexer.readToken();

		VariablePrototype v = VariablePrototype(ident.getLexem(), type, (varFlags & VF_PRIVATE), currentScope);

		if(varFlags & VF_STATIC)
		{

			currentClassProto->declareStaticPrototype(v);

		}else if(varFlags & VF_LOCAL)
		{
			currentFunctionProto->declareLocalPrototype(v);

		}else
		{
			currentClassProto->declareGlobalPrototype(v);
		}

		/* Removed, as variable declaration does not happen via bytecode anymore
		writeASM("DECLAREVAR '" + ident.getLexem() + "'," + (int)type.getTypeID() + "," + (int)priv + "," + (int)native);

		uint8_t flags = 0;
		if(native)
		{
			flags |= 1; //TODO: Maybe store this in a macro
		}
		if(priv)
		{
			flags |= 2;
		}
		asmout << OP_DECLAREVAR;
		asmout.writeTString(ident.getLexem());
		asmout.write(flags);
		writeDatatypeToBytecode(type);*/

		if(t.is(Token::TT_OPERATOR, "="))
		{
			ExpressionInfo info(DataType(DataType::DTB_NOTYPE), true, ExpressionInfo::FT_LITERAL);

			if(varFlags & VF_STATIC)
			{
				info = staticExpression();

			}else
			{
				info = expression(true);
			}

			if(!DataType::canCastImplicitly(info.getType(), type))
			{
				error("Type mismatch in variable initialization: Can not implicitly cast " + info.getType().toString() + " to " + type.toString(), t.getLine());
			}

			t = lexer.readToken();

			if(!t.is(Token::TT_OPERATOR, ";"))
			{
				error("Expected semicolon after variable initialization, but found: " + t.getLexem(), t.getLine());
			}

			writeASM("POPVAR '" + ident.getLexem() + "'");
			asmout << OP_POPVAR;

		}else if(!t.is(Token::TT_OPERATOR, ";"))
		{
			error("Expected semicolon or assignment after variable declaration, but found: " + t.getLexem(), t.getLine());
		}
	}

    void Compiler::functionDec(uint32_t funcFlags)
    {
    	Token t = lexer.readToken(); //Read the left recognizer

    	DataType returnType = DataType(DataType::DTB_NOTYPE);
    	if(!(funcFlags & VF_CONSTRUCTOR))
    	{
    		t = lexer.peekToken();

    		if(!t.is(Token::TT_KEYWORD, "void"))
    		{
    			returnType = dataType();

    		}else
    		{
    			lexer.readToken();
    		}
    	}


		Token ident = (funcFlags & VF_CONSTRUCTOR) ? t : name(); //The ident of a constructor is it's recognizer (init)

		t = lexer.readToken();

		if(!t.is(Token::TT_OPERATOR,"("))
		{
			error("Expected parameter list after function name, but found: " + t.getLexem(), t.getLine());
		}


		std::vector<DataType> paramTypes;
		std::vector<String> paramNames;

		if(!lexer.peekToken().is(Token::TT_OPERATOR,")"))
		{
			do
			{
				DataType type = dataType();
				paramTypes.push_back(type);

				bool byVal = false;

				t = lexer.peekToken();
				if(t.is(Token::TT_OPERATOR, "$"))
				{
					byVal = true;
					lexer.readToken(); //consume the $
				}

				t = name();

				//Rather than creating a new vector just for the byVals, we insert the dollar into the varname for later evaluation
				paramNames.push_back(byVal ? ("$" + t.getLexem()) : t.getLexem());

				t = lexer.readToken();

				if(!(t.is(Token::TT_OPERATOR,",") || (t.is(Token::TT_OPERATOR,")"))))
				{
					error("Expected , or closing parentheses in parameter list, but found: " + t.getLexem(), t.getLine());
				}

			}while(t.is(Token::TT_OPERATOR,","));

		}else
		{
			t = lexer.readToken();
		}

		if(!t.is(Token::TT_OPERATOR,")"))
		{
			error("Expected closing parentheses after parameter list, but found: " + t.getLexem(), t.getLine());
		}

		//We are ready to create a prototype
		FunctionPrototype proto = FunctionPrototype(ident.getLexem(), paramTypes, returnType, funcFlags & VF_PRIVATE, funcFlags & VF_NATIVE);

		//Check if function has a body
		t = lexer.peekToken();
		if(t.is(Token::TT_OPERATOR, ";"))
		{
			//This is just a function declaration, so just store the adressless prototype and carry on

			currentClassProto->declareFunctionPrototype(proto);
			return; //We are finished here (for now. this prototype needs an adress and a body, which must be supplied by the programmer)

		}else if(funcFlags & VF_NATIVE) //If function is native, insist for the semicolon!
		{
			error(String("Native functions must not have a body. Expected semicolon after parameter list") ,t.getLine());
		}

		//Okay, function has a body. Let's turn it into bytecode and give the prototype an adress

		FunctionPrototype *existingFuncProto = currentClassProto->getFunctionPrototype(ident.getLexem(), paramTypes, false); //Do not include inherited functions, we want to be able to override them!
		if(existingFuncProto != null)
		{
			if(existingFuncProto->hasAdress())
			{
				error("Double definition of function " + FunctionPrototype::createInfoString(ident.getLexem(), paramTypes) + " in class " + currentClassProto->getClassId().toString(), ident.getLine());
			}
		}


		int currentFunctionUID = getNewUID();
		Scope scopeOfFuncBlock = Scope(1,0,currentFunctionUID); //nestId of 0 is reserved for globals


		writeASM("# definition of function " + FunctionPrototype::createInfoString(ident.getLexem(), paramTypes));
		writeASM("JUMP func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_end:");
		asmout << OP_JUMP;
		asmout.writeMarker("func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_end:");

		//This is where the function begins
		proto.setAdress(asmout.getSize());

		currentClassProto->declareFunctionPrototype(proto);//The prototype is finished, so register it
		currentFunctionProto = currentClassProto->getFunctionPrototype(ident.getLexem(), paramTypes); //We need a pointer to the stored prototype for changes (must not be const)

		writeASM("func_" + ident.getLexem() + "_uid" + currentFunctionUID +  "_start:");
		asmout.defineMarker("func_" + ident.getLexem() + "_uid" + currentFunctionUID +  "_start");

		writeASM("BEGIN");
		asmout << OP_BEGIN;

		for(int i = paramTypes.size()-1; i >= 0 ; i--)
		{
			String paraName = paramNames[i];

			bool byVal = StringUtils::startsWith(paraName, "$");
			if(byVal)
			{
				//In the code above we inserted a dollar in front of all byValue parameters. Here we remove it

				paraName = StringUtils::substring(paraName, 1, paraName.length());
			}

			//Declare locals prototypes
			VariablePrototype v = VariablePrototype(paraName, paramTypes[i], true, scopeOfFuncBlock); //Parameters are always private
			currentFunctionProto->declareLocalPrototype(v);

			//Get the locals prototypes from stack
			if(byVal)
			{
				writeASM("DEREFER");

				asmout << OP_DEREFER;
			}

			writeASM("POPVAR '" + paraName + "'");
			asmout << OP_POPVAR;
			asmout.writeBString(paraName);
		}

		block(BT_FUNC, scopeOfFuncBlock);

		//TODO: Check if we need to output a return here

		writeASM("END");
		asmout << OP_END;

		writeASM("func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_end:");
		asmout.defineMarker("func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_end");
    }

    DataType Compiler::functionCall(const String &funcName, bool member, const DataType &baseType)
    {
		Token t = lexer.readToken();
		if(!t.is(Token::TT_OPERATOR,"("))
		{
			error("Expected opening parentheses after function name in function call", t.getLine());
		}

		int paramCount = 0;

		TypeList paramTypes;

		t = lexer.peekToken();
		if(!t.is(Token::TT_OPERATOR,")"))
		{

			do
			{
				ExpressionInfo info = expression(true);

				paramTypes.push_back(info.getType());

				paramCount++;
				t = lexer.readToken();

			}while(t.is(Token::TT_OPERATOR,","));

			if(!t.is(Token::TT_OPERATOR,")"))
			{
				error("Expected closing parentheses after parameter list in function call", t.getLine());
			}

		}else
		{
			lexer.readToken();
		}

		if(member)
		{
			if(baseType.getTypeBase() != DataType::DTB_OBJECT)
			{
				error("Only non-abstract object types have callable member functions, but given type was: " + baseType.toString(), t.getLine());
			}

			ClassPrototype *classProto = lookupClassPrototype(baseType.getClassId());

			if(classProto == null)
			{
				error("[Compiler bug] Specifier of given base type for member function call points to non-existent class: " + baseType.toString(), t.getLine());
			}

			FunctionPrototype *functionProto = classProto->getFunctionPrototype(funcName, paramTypes);

			if(functionProto == null)
			{
				error("Function '" + FunctionPrototype::createInfoString(funcName, paramTypes) + "' was not defined in given object type class '" + baseType.toString() + "'", t.getLine());
			}
			
			if(!functionProto->hasAdress())
			{
				error("Function '" + FunctionPrototype::createInfoString(funcName, paramTypes) + "' in given object type class '" + baseType.toString() + "' was declared but never defined", t.getLine());
			}

			if(functionProto->isPrivate())
			{
				error("Function '" + FunctionPrototype::createInfoString(funcName, paramTypes) + "' in given object type class '" + baseType.toString() + "' is private", t.getLine());
			}

			writeASM("CALLMEMBER '" + funcName + "'," + paramCount);
			asmout << OP_CALLMEMBER;
			asmout.writeBString(funcName);
			asmout.writeUByte(paramCount);

			return functionProto->getReturnType();

		}else
		{
			FunctionPrototype *functionProto = currentClassProto->getFunctionPrototype(funcName, paramTypes, true); //Also check for functions of superclasses

			if(functionProto == null)
			{
				error("Function '" + FunctionPrototype::createInfoString(funcName, paramTypes) + "' was not defined in current class", t.getLine());
			}

			if(!functionProto->hasAdress())
			{
				error("Function '" + FunctionPrototype::createInfoString(funcName, paramTypes) + "' in current class was declared but never defined", t.getLine());
			}

			//No need to check for private, as we are calling from the same script
			
			writeASM("CALL '" + funcName + "'," + paramCount); //TODO: We might direct the call to the respective superclass if it is not defined in the current one
			asmout << OP_CALL;
			asmout.writeBString(funcName);
			asmout.writeUByte(paramCount);

			return functionProto->getReturnType();
		}
    }

    DataType Compiler::dataType()
    {
    	Token t = lexer.readToken();

    	if(isPrimitive(t))
    	{

    		return DataType::byName(t.getLexem());

    	}else if(t.getType() == Token::TT_IDENT)
    	{
    		Token t2 = lexer.peekToken();

    		if(t2.is(Token::TT_OPERATOR, ":"))
    		{
    			if(!namespaceExists(t.getLexem()))
    			{
    				error("Identifier '" + t.getLexem() + "' does not name a namespace", t.getLine());
    			}

    			lexer.readToken();
    			t2 = lexer.readToken();

    			if(t2.getType() != Token::TT_IDENT)
    			{
    				error("Expected script identifier after namespace operator, but found: " + t2.getLexem(), t2.getLine());
    			}

    			if(lookupClassPrototype(ClassId(t.getLexem(), t2.getLexem())) == null)
    			{
    				error("Identifier '" + t2.getLexem() + "' does not name a class in namespace '" + t.getLexem() + "'", t2.getLine());
    			}

    			return DataType(DataType::DTB_OBJECT, ClassId(t.getLexem(), t2.getLexem()));

    		}else
    		{
    			//No namespace given -> search the default locations
    			ClassPrototype *classProto = lookupClassPrototypeByName(t.getLexem());

				if(classProto == null)
				{
					error("Identifier '" + t.getLexem() + "' does not name a class in current/default/imported namespace", t.getLine());
				}

    			return DataType(DataType::DTB_OBJECT, classProto->getClassId()); //TODO: The class of a data type is currently pointed by name. we might point by pointer to be more efficient
    		}

    	}else
    	{
    		error("Expected primitve data type or script identifier, but found: " + t.getLexem(), t.getLine());
    	}

    	return DataType(DataType::DTB_NOTYPE); //should never happen
    }

    Token Compiler::name()
    {
    	Token t = lexer.readToken();

    	if(t.getType() != Token::TT_IDENT)
		{
			error("Expected name after data type, but found: " + t.getLexem(), t.getLine());
		}

		if(namespaceExists(t.getLexem()) || (lookupClassPrototype(ClassId("",t.getLexem())) != null))
		{
			error("Name '" + t.getLexem() + "' is already beeing used as a classname or namespace", t.getLine());
		}

		return t;
    }

    ExpressionInfo Compiler::expression(const bool mustYieldResult)
    {
        ExpressionInfo leftInfo = logicExpression();

        Token t = lexer.peekToken();
        if(isAssignmentOperator(t))
        {
        	if(leftInfo.getFactorType() != ExpressionInfo::FT_VARIABLE_REF)
        	{
        		error("Variable reference required on left side of assignment operator", t.getLine());
        	}

        	writeASM("CLONE"); //This produces the result of the assignment
        	asmout << OP_CLONE;

        	if(!t.is(Token::TT_OPERATOR, "="))
        	{
        		//We have a compound operator that requires a second instance of the variable reference for the pre-assign operation
        		writeASM("CLONE");
        		asmout << OP_CLONE;
        	}

        	ExpressionInfo rightInfo = expression(true);

        	if(!DataType::canCastImplicitly(rightInfo.getType(), leftInfo.getType()))
			{
				error("Type mismatch in assignment: Can not implicitly cast " + rightInfo.getType().toString() + " to " + leftInfo.getType().toString(), t.getLine());
			}

			if(t.is(Token::TT_OPERATOR, "+="))
			{
				writeASM("ADD");
				asmout << OP_ADD;

			}else if(t.is(Token::TT_OPERATOR, "-="))
			{
				writeASM("SUBTRACT");
				asmout << OP_SUBTRACT;

			}else if(t.is(Token::TT_OPERATOR, "*="))
			{
				writeASM("MULTIPLY");
				asmout << OP_MULTIPLY;

			}else if(t.is(Token::TT_OPERATOR, "/="))
			{
				writeASM("DIVIDE");
				asmout << OP_DIVIDE;

			}else if(!t.is(Token::TT_OPERATOR, "="))
			{
				error("[Compiler bug] Unknown assignment operator: " + t.getLexem(), t.getLine());
			}

        	writeASM("POPREF"); //Pops to reference
        	asmout << OP_POPREF;

        }

        if(mustYieldResult && leftInfo.getFactorType() == ExpressionInfo::FT_VOID)
        {
        	error("Expression must yield a value, but just contains a call to a void function", t.getLine());
        }

        return leftInfo; //The result of an assignment is the left hand side of the assignment operator (always a reference to the variable that was assigned to)
    }

    ExpressionInfo Compiler::logicExpression()
    {
    	ExpressionInfo info = relationalExpression();

		Token t = lexer.peekToken();
		while(isLogicOp(t))
		{
			if(info.getFactorType() == ExpressionInfo::FT_VOID)
			{
				error("Logic operations not possible with void type expression", t.getLine());
			}

			lexer.readToken();

			ExpressionInfo rightInfo = relationalExpression();

			if(!info.getType().isNumeric())
			{
				error(String("Left hand side of operator ") + t.getLexem() + " is of non-numeric type(" + info.getType().toString() + "), but " + t.getLexem() + " requires numeric types (representing booleans)", t.getLine());
			}
			if(!rightInfo.getType().isNumeric())
			{
				error(String("Right hand side of operator ") + t.getLexem() + " is of non-numeric type(" + rightInfo.getType().toString() +"), but " + t.getLexem() + " requires numeric types (representing booleans)", t.getLine());
			}

			info = ExpressionInfo(DataType(DataType::DTB_INT), rightInfo.isConstant() && info.isConstant(), ExpressionInfo::FT_MATH_EXPR); //Type is always int after logic operation

			if(t.is(Token::TT_OPERATOR,"|"))
			{
				writeASM("LOGICOR");
				asmout << OP_LOGICOR;

			}else if(t.is(Token::TT_OPERATOR,"&"))
			{
				writeASM("LOGICAND");
				asmout << OP_LOGICAND;
			}

			t = lexer.peekToken();
		}

		return info;
    }

    ExpressionInfo Compiler::relationalExpression()
    {
        ExpressionInfo info = arithmeticExpression();

        Token t = lexer.peekToken();
        while(isRelationalOp(t))
        {
        	if(info.getFactorType() == ExpressionInfo::FT_VOID)
			{
				error("Relation operations not possible with void type expression", t.getLine());
			}

            lexer.readToken();

            ExpressionInfo rightInfo = arithmeticExpression();
            info = ExpressionInfo(DataType(DataType::DTB_INT), rightInfo.isConstant() && info.isConstant(), ExpressionInfo::FT_MATH_EXPR); //Type is always int after relation

            if(t.is(Token::TT_OPERATOR,"=="))
            {
                writeASM("COMPARE");
                asmout << OP_COMPARE;

            }else if(t.is(Token::TT_OPERATOR,"<"))
            {
                writeASM("LESS");
                asmout << OP_LESS;

            }else if(t.is(Token::TT_OPERATOR,">"))
            {
                writeASM("GREATER");
                asmout << OP_GREATER;

            }else if(t.is(Token::TT_OPERATOR,"<="))
            {
                writeASM("LESSEQUAL");
                asmout << OP_LESSEQUAL;

            }else if(t.is(Token::TT_OPERATOR,">="))
            {
                writeASM("GREATEREQUAL");
                asmout << OP_GREATEREQUAL;

            }else if(t.is(Token::TT_OPERATOR,"!="))
            {
                writeASM("COMPARE");
                asmout << OP_COMPARE;

                writeASM("INVERT");
                asmout << OP_INVERT;
            }

            t = lexer.peekToken();
        }

        return info;
    }

    ExpressionInfo Compiler::arithmeticExpression()
    {
    	ExpressionInfo info = term();

        Token t = lexer.peekToken();
        while(isAddOp(t))
        {
        	if(info.getFactorType() == ExpressionInfo::FT_VOID)
			{
				error("Mathematic operations not possible with void type expression", t.getLine());
			}

            lexer.readToken();

            ExpressionInfo rightInfo = term();

            if(t.is(Token::TT_OPERATOR,"+"))
            {
            	if(info.getType() == DataType::DTB_STRING || rightInfo.getType() == DataType::DTB_STRING)
            	{
            		info = ExpressionInfo(DataType(DataType::DTB_STRING), info.isConstant() && rightInfo.isConstant(), ExpressionInfo::FT_MATH_EXPR);

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
                asmout << OP_ADD;

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

                writeASM("SUBTRACT");
                asmout << OP_SUBTRACT;
            }

            t = lexer.peekToken();
        }

        return info;
    }

    ExpressionInfo Compiler::term()
    {
    	ExpressionInfo info = castFactor();

        Token t = lexer.peekToken();
        while(isMultiplyOp(t))
        {
        	if(info.getFactorType() == ExpressionInfo::FT_VOID)
			{
				error("Mathematic operations not possible with void type expression", t.getLine());
			}

            lexer.readToken();

            ExpressionInfo rightInfo = castFactor();

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
                asmout << OP_MULTIPLY;

            }else if(t.is(Token::TT_OPERATOR,"/"))
            {
                writeASM("DIVIDE");
                asmout << OP_DIVIDE;
            }

            t = lexer.peekToken();
        }

        return info;
    }

    ExpressionInfo Compiler::castFactor()
	{
		ExpressionInfo info = signedFactor();

		Token t = lexer.peekToken();
		if(t.is(Token::TT_OPERATOR,"->"))
		{
			if(info.getFactorType() == ExpressionInfo::FT_VOID)
			{
				error("Casts not possible with void type expression", t.getLine());
			}

			if(info.getType() == DataType::DTB_NOTYPE)
			{
				error("Can not cast nulltype to anything", t.getLine());
			}

			lexer.readToken();

			DataType type = dataType();

			if(type == DataType::DTB_OBJECT)
			{
				writeASM(String("TOOBJECTINSTANCE '") + type.getClassId().getNamespace() + "','" + type.getClassId().getClassname() + "'");
				asmout << OP_TOOBJECTINSTANCE;
				asmout.writeBString(type.getClassId().getNamespace());
				asmout.writeBString(type.getClassId().getClassname());

			}else
			{
				writeASM(String("TO") + type.getTypeBase());
				asmout << (OP_TOINT + type.getTypeBase());
			}

			return ExpressionInfo(type, false, ExpressionInfo::FT_MATH_EXPR);
		}

		return info;
	}

    ExpressionInfo Compiler::signedFactor()
	{
		bool negate = false;
		bool invert = false;
		bool toVal = false;

		Token t = lexer.peekToken();
		if(t.is(Token::TT_OPERATOR,"-"))
		{
			negate = true;
			lexer.readToken();

		}else if(t.is(Token::TT_OPERATOR,"!"))
		{
			invert = true;
			lexer.readToken();

		}else if(t.is(Token::TT_OPERATOR,"$"))
		{
			toVal = true;
			lexer.readToken();
		}

		ExpressionInfo info = memberFactor();

		if(info.getFactorType() == ExpressionInfo::FT_VOID)
		{
			error("Unary inversion/negation not possible with void type expression", t.getLine());
		}

		if(negate)
		{
			if(!info.getType().isNumeric())
			{
				error(String("Negation operator is applicable on numeric types only. Given type is: ") + info.getType().toString() ,t.getLine());
			}

			writeASM("NEGATE");
			asmout << OP_NEGATE;
		}

		if(invert)
		{
			if(!info.getType().isNumeric())
			{
				error(String("Inversion operator is applicable on numeric types only. Given type is: ") + info.getType().toString() ,t.getLine());
			}

			writeASM("INVERT");
			asmout << OP_INVERT;

			info = ExpressionInfo(DataType(DataType::DTB_INT), info.isConstant(), ExpressionInfo::FT_MATH_EXPR);
		}

		if(toVal)
		{
			writeASM("DEREFER");
			asmout << OP_DEREFER;

			info = ExpressionInfo(info.getType(), info.isConstant(), ExpressionInfo::FT_MATH_EXPR); //TODO: I have a bad feeling about this. Maybe re-think if all the values are correct
		}

		return info;
	}

    ExpressionInfo Compiler::memberFactor()
    {
        ExpressionInfo info = factor();

        Token t = lexer.peekToken();
        while(t.is(Token::TT_OPERATOR,"."))
        {
        	if(info.getFactorType() == ExpressionInfo::FT_VOID)
			{
				error("Member access not possible with void type expression", t.getLine());
			}

        	if(!(info.getType().getTypeBase() == DataType::DTB_OBJECT))
			{
				error("Only non-abstract object types have accessible members, but given type is: " + info.getType().toString(), t.getLine());
			}

            lexer.readToken();
            t = lexer.readToken();
            if(t.getType() != Token::TT_IDENT)
            {
                error(String("Expected identifier after member directive, but found: ") + t.getLexem(), t.getLine());
            }

            Token member = t;

            t = lexer.peekToken();
            if(t.is(Token::TT_OPERATOR,"("))
            {
                DataType ftype = functionCall(member.getLexem(), true, info.getType());

                info = ExpressionInfo(ftype, false, (ftype == DataType::DTB_NOTYPE) ? ExpressionInfo::FT_VOID : ExpressionInfo::FT_FUNCTION_RETURN);

            }else
            {
            	ClassPrototype *s = lookupClassPrototype(info.getType().getClassId());
				if(s == null)
				{
					error("Specifier of given object type points to non-existent class '" + info.getType().toString() + "'", member.getLine());
				}

				VariablePrototype *v = s->getGlobalPrototype(member.getLexem(), true); //No need to access locals
				if(v == null)
				{
					error("Referenced class '" + info.getType().toString() + "' has no member '" + member.getLexem() + "'", member.getLine());
				}

				if(v->isPrivate() && !(s->getClassId() == currentClassProto->getClassId()))
				{
					//If a private object member is accessed from a script of the same type as the object, the access modifier is ignored (like all script methods are declared as friend)

					error("Variable '" + member.getLexem() + "' in referenced class '" + info.getType().toString() + "' is private", member.getLine());
				}

                info = ExpressionInfo(v->getType(), false, ExpressionInfo::FT_VARIABLE_REF);

                writeASM("GETMEMBER '" + member.getLexem() + "'");
                asmout << OP_GETMEMBER;
                asmout.writeBString(member.getLexem());
            }
        }

        return info;
    }

    ExpressionInfo Compiler::indexFactor()
    {
    	ExpressionInfo info = factor();

    	Token t = lexer.peekToken();
    	if(t.is(Token::TT_OPERATOR, "["))
		{
    		if(info.getFactorType() == ExpressionInfo::FT_VOID)
    		{
    			error("Array access not possible with void type expression", t.getLine());
    		}

    		if(info.getType().getTypeBase() != DataType::DTB_STRING)
    		{
    			error("Currently only strings are accessible by the [] operator", t.getLine());
    		}

			lexer.readToken();

			ExpressionInfo index = expression(true);
			if(index.getType().getTypeBase() != DataType::DTB_INT)
			{
				error("Invalid array index type. Array indices must be of type int", t.getLine());
			}

			writeASM("GETINDEX");
			asmout << OP_GETINDEX;

			t = lexer.readToken();
			if(!t.is(Token::TT_OPERATOR, "]"))
			{
				error("Missing closing square brackets after array index expression",t.getLine());
			}

			return ExpressionInfo(info.getType(), false, ExpressionInfo::FT_VARIABLE_REF);
		}

    	return info;
    }

    ExpressionInfo Compiler::factor()
    {
        Token t = lexer.readToken();

        if(t.getType() == Token::TT_NUMBER)
        {
            DataType numberType = getTypeOfNumberString(t.getLexem());

            if(numberType == DataType::DTB_NOTYPE)
            {
                error("Number literal '" + t.getLexem() + "' does not fit in any numerical data type",t.getLine());
            }

            writeASM(String("PUSH ") + ((int)numberType.getTypeBase()) + ", " + t.getLexem());
            asmout << OP_PUSHINT + numberType.getTypeBase();
            switch(numberType.getTypeBase())
            {
            case DataType::DTB_INT:
            	asmout.writeInt(parseInt(t.getLexem()));
            	break;

            case DataType::DTB_LONG:
            	asmout.writeLong(parseLong(t.getLexem()));
            	break;

            case DataType::DTB_FLOAT:
            	asmout.writeFloat(parseFloat(t.getLexem()));
            	break;

            case DataType::DTB_DOUBLE:
            	asmout.writeDouble(parseDouble(t.getLexem()));
            	break;

            default:
            	error("[Compiler bug] Unknown number type", t.getLine());
            	break;
            }

            return ExpressionInfo(numberType, true, ExpressionInfo::FT_LITERAL);

        }else if(t.getType() == Token::TT_STRING)
        {
            writeASM("PUSHSTRING '" + escapeASMChars(t.getLexem()) + "'");
            asmout << OP_PUSHSTRING;
            asmout.writeIString(t.getLexem()); //TODO: Process escape sequences here

           return ExpressionInfo(DataType(DataType::DTB_STRING), true, ExpressionInfo::FT_LITERAL);

        }else if(t.getType() == Token::TT_IDENT)
        {
            Token t2 = lexer.peekToken();
            if(t2.is(Token::TT_OPERATOR,"("))
            {
                DataType type = functionCall(t.getLexem(), false);

                return ExpressionInfo(type, false, (type == DataType::DTB_NOTYPE) ? ExpressionInfo::FT_VOID : ExpressionInfo::FT_FUNCTION_RETURN);

            }else
            {
            	VariablePrototype *v = lookupVariablePrototype(t.getLexem());

				if(v == null)
				{
					error("Variable/type '" + t.getLexem() + "' was not declared in this scope", t.getLine());
				}

                writeASM("PUSHVAR '" + t.getLexem() + "'");
                asmout << OP_PUSHVAR;
                asmout.writeBString(t.getLexem());

                return ExpressionInfo(v->getType(), false, ExpressionInfo::FT_VARIABLE_REF);
            }

        }else if(t.is(Token::TT_OPERATOR,"("))
        {
            ExpressionInfo info = expression(true);

            Token t2 = lexer.readToken();
            if(!t2.is(Token::TT_OPERATOR,")"))
            {
                error("Missing closing parentheses after expression",t.getLine());
            }

            return info;

        }/*else if(t.is(Token::TT_OPERATOR,"["))
        {
            DataType type = arrayLiteral();

            Token t2 = lexer.readToken();
            if(!t2.is(Token::TT_OPERATOR,")"))
            {
                error("Missing closing square brackets after array literal expression",t.getLine());
            }

            return ExpressionInfo(type, false, ExpressionInfo::FT_LITERAL);

        }*/else if(t.is(Token::TT_KEYWORD,"new"))
        {
            DataType classtype = dataType();

            if(!(classtype == DataType::DTB_OBJECT))
            {
            	error("Expected non-abstract object type identifier after new keyword, but found: " + classtype.toString(), t.getLine());
            }

			Token t2 = lexer.readToken();
			if(!t2.is(Token::TT_OPERATOR,"("))
			{
				error("Expected parameter list after scriptname in constructor call, but found: " + t2.getLexem(), t2.getLine());
			}

			int paramCount = 0;

			TypeList paramTypes = TypeList();

			t2 = lexer.peekToken();
			if(!t2.is(Token::TT_OPERATOR,")"))
			{

				do
				{
					ExpressionInfo info = expression(true);

					paramTypes.push_back(info.getType());

					paramCount++;
					t2 = lexer.readToken();

				}while(t2.is(Token::TT_OPERATOR,","));

				if(!t2.is(Token::TT_OPERATOR,")"))
				{
					error("Expected closing parentheses after parameter list in constructor call, but found: " + t2.getLexem(), t2.getLine());
				}

			}else
			{
				lexer.readToken();
			}
			
			ClassPrototype *cl = lookupClassPrototype(classtype.getClassId());
			if(cl == null)
			{
				error("Given identifier '" + classtype.toString() + "' in new statement does not name a type", t2.getLine());
			}
			
			FunctionPrototype *constr = cl->getFunctionPrototype("init", paramTypes);
			
			if(constr == null)
			{
				error("Constructor " + FunctionPrototype::createInfoString(classtype.toString(), paramTypes) + " was not declared", t2.getLine());
				
			}else
			{
				if(constr->isPrivate())
				{
					error("Constructor " + FunctionPrototype::createInfoString(classtype.toString(), paramTypes) + " is private", t2.getLine());
				}
			
				writeASM("NEW '" + classtype.getClassId().getNamespace() + "','" + classtype.getClassId().getClassname() + "'," + paramCount);
				asmout << OP_NEW;
				asmout.writeBString(classtype.getClassId().getNamespace());
				asmout.writeBString(classtype.getClassId().getClassname());
				asmout.writeUByte(paramCount);
			}

			return ExpressionInfo(classtype, false, ExpressionInfo::FT_FUNCTION_RETURN); //Constructors are actually void functions, but the new statement returns the instance, so it is considered as a non-void function call

        }else if(t.is(Token::TT_KEYWORD,"null"))
        {
            writeASM("PUSHNULL");
            asmout << OP_PUSHNULL;

            return ExpressionInfo(DataType(DataType::DTB_NOTYPE), true, ExpressionInfo::FT_LITERAL);

        }else if(t.is(Token::TT_KEYWORD,"this"))
        {
        	//TODO: Private variables in "this" are currently not accessible, this has to be changed
        	//Note: the public API for scripts should not give any access to locals. Same goes for the one used for variable type stuff during compile time
        	//so this.xyz won't let the program access locals. (Functions where locals have same name as globals which should still be accessed can be by using 'this')

        	t = lexer.peekToken();

        	if(t.is(Token::TT_OPERATOR, "("))
        	{
        		if(!currentFunctionProto->isConstructor())
        		{
        			error("Constructor delegation not possible outside of constructor", t.getLine());
        		}

        		functionCall("init",false); //Calling a constructor of the current script

        		return ExpressionInfo(DataType(DataType::DTB_NOTYPE), true, ExpressionInfo::FT_VOID); //Constructors are void functions thus returning nothing

        	}else
        	{

        		writeASM("PUSHTHIS");
        		asmout << OP_PUSHTHIS;

        		return ExpressionInfo(DataType(DataType::DTB_OBJECT, currentClassProto->getClassId()), false, ExpressionInfo::FT_LITERAL); //Although "PUSHTHIS" actually creates a reference, returning FT_VARIABLE_REF would allow the program to assign to it
        	}

        }else if(t.is(Token::TT_KEYWORD,"true"))
        {
        	writeASM("PUSHINT 1");
        	asmout << OP_PUSHINT;
        	asmout.writeInt(1);

        }else if(t.is(Token::TT_KEYWORD,"false"))
        {
        	writeASM("PUSHINT 0");
        	asmout << OP_PUSHINT;
        	asmout.writeInt(0);

        }else if(t.getType() == Token::TT_EOF)
        {
            error("Unexpected end of file while parsing expression", t.getLine());

        }else
        {
            error("Unexpected token in expression factor: " + t.getLexem(), t.getLine());
        }

        return ExpressionInfo(DataType(DataType::DTB_NOTYPE), false, ExpressionInfo::FT_LITERAL); //Should never happen
    }

    /*ArrayType Compiler::arrayLiteral()
    {
    	Token t = lexer.peekToken();
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
    				error("Type mismatch in array literal: First element in list was of type " + type->toString() + ", but element #" + (int)elementCount + " was of type " + info.getType().toString() + " (implicit casts in array literals are not yet allowed)", lexer.getCurrentLine());
    			}
    		}

    		t = lexer.readToken();
    		if(!t.is(Token::TT_OPERATOR, ",") && !t.is(Token::TT_OPERATOR, "]"))
    		{
    			error("Expected ',' after array literal element #" + (int)elementCount + ", but found: " + t.getLexem(), t.getLine());
    		}

    		elementCount++;
    	}

    	return ArrayType(&type, elementCount);
    }*/

    void Compiler::writeDatatypeToBytecode(const DataType &t)
    {
    	uint8_t id = t.getTypeBase();

    	if(t.isArray())
    	{
    		id |= 8;
    	}

    	asmout.writeUByte(id);

    	if(t == DataType::DTB_OBJECT)
    	{
    		asmout.writeBString(t.getClassId().getNamespace());
    		asmout.writeBString(t.getClassId().getClassname());
    	}
    }

    DataType Compiler::getTypeOfNumberString(const String &s)
    {
    	bool fp = (StringUtils::indexOf(s, '.') != -1);

    	//TODO: Consider big types here
    	return fp ? DataType(DataType::DTB_FLOAT) : DataType(DataType::DTB_INT);
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

    bool Compiler::isModifier(const Token &t)
    {
    	return t.is(Token::TT_KEYWORD, "public") || t.is(Token::TT_KEYWORD, "private") || t.is(Token::TT_KEYWORD, "native") || t.is(Token::TT_KEYWORD, "static");
    }

    bool Compiler::isPrimitive(const Token &t)
    {
    	if(t.getType() != Token::TT_KEYWORD)
    	{
    		return false;
    	}

    	for(uint16_t i = 0; i < DATATYPE_COUNT; i++)
    	{

    		if(t.getLexem() == DATATYPES[i])
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

    int32_t Compiler::parseInt(const String &s)
    {
    	int32_t v = 0;

    	for(uint32_t i = 0; i < s.length(); i++)
    	{
    		v += s[i] - '0';
    		v *= 10;
    	}

    	return v;
    }

    int64_t Compiler::parseLong(const String &s)
    {
    	int64_t v = 0;

		for(uint32_t i = 0; i < s.length(); i++)
		{
			v += s[i] - '0';
			v *= 10;
		}

		return v;
    }

    float Compiler::parseFloat(const String &s)
    {
    	float v = 0;
    	bool decimals = false;
    	float decimalFactor = 0.1f;

		for(uint32_t i = 0; i < s.length(); i++)
		{
			if(s[i] == '.')
			{
				decimals = true;

			}else
			{
				if(decimals)
				{
					v += ((double)(s[0] - '0')) * decimalFactor;
					decimalFactor *= 0.1;

				}else
				{
					v += s[i] - '0';
					v *= 10;
				}
			}
		}

		return v;
    }

    double Compiler::parseDouble(const String &s)
    {
    	double v = 0;
		bool decimals = false;
		double decimalFactor = 0.1f;

		for(uint32_t i = 0; i < s.length(); i++)
		{
			if(s[i] == '.')
			{
				decimals = true;

			}else
			{
				if(decimals)
				{
					v += ((double)(s[0] - '0')) * decimalFactor;
					decimalFactor *= 0.1;

				}else
				{
					v += s[i] - '0';
					v *= 10;
				}
			}
		}

		return v;
    }

    void Compiler::error(const String &s, int line)
    {
    	//TODO: Do more stuff with the error message than just throwing an exception

    	throw ScalesException(ScalesException::ET_COMPILER, String("Error in line ") + line + ": " + s);
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
			"native",
			"void",
			"class",
			"static",
			"links",
			"extends",
			"using",
			"return",
			"while",
			"if",
			"elseif",
			"else",
			"break",
			"null",
			"this",
			"goto",
			"new",
			"init",
			"true",
			"false",

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
			"[",
			"]",
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

}
