
#include "compiler/ScalesDefaultCompiler.h"

#include "ScalesException.h"


#ifdef SCALES_COMPILER_WRITEASM

	#include <iostream>

	#define writeASM(s) std::cout << __LINE__ << ":\t" << s << std::endl;

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

	//TODO: check for double declarations (not data type double but... ah, you know what I mean)
	
	//TODO: improve assembly/bytecode builder & define bytecode in ScalesBytecode.h

    DefaultCompiler::DefaultCompiler(Root *pRoot, compilerFlags_t pFlags)
    :	root(pRoot),
		currentClass(nullptr),
		currentFunction(nullptr),
		lastUID(0),
		lexer(Lexer::CODING_ASCII, KEYWORDS, KEYWORD_COUNT, OPERATORS, OPERATOR_COUNT, true)
    {
    }

    DefaultCompiler::~DefaultCompiler()
    {
    }

    size_t DefaultCompiler::compile(std::istream *in)
    {
    	lexer.setDataSource(in);
    	classList.clear();

    	mainBlock();

    	return classList.size();
    }

    std::vector<const Class*> DefaultCompiler::listGeneratedClasses()
	{
    	return classList;
	}

    void DefaultCompiler::mainBlock()
    {
    	String nspace = "";

    	Token t = lexer.readToken();

		while(!t.isType(Token::TT_EOF))
		{
			if(t.is(Token::TT_KEYWORD,"class"))
			{
				classDef(nspace);

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
				error("Expected class header or namespace definition, but found: " + t.getLexem(), t.getLine());
			}

			t = lexer.readToken();
		}
    }

    void DefaultCompiler::classDef(const String &nspace)
    {
    	//a new class has begun, all imported classes loose their validity
    	importList.clear();

    	Token t = lexer.readToken();
		if(!t.isType(Token::TT_IDENT))
		{
			error("Expected classname after 'class' keyword, but found: " + t.getLexem(), t.getLine());
		}

		ClassID id = ClassID(nspace, t.getLexem());
		const Class *superclass = nullptr;

		t = lexer.peekToken();
		if(t.is(Token::TT_KEYWORD, "extends"))
		{
			lexer.readToken(); //Consume the keyword

			t = lexer.peekToken(); //only peeking because classID() is doing the processing later
			if(!t.isType(Token::TT_IDENT))
			{
				error("Expected class identifier after extends keyword, but found: " + t.getLexem(), t.getLine());
			}

			ClassID cid = classID();
			superclass = lookupClass(cid);

			if(superclass == nullptr)
			{
				error("Given superclass of class " + id.toString() + " was not found", t.getLine());
			}

			t = lexer.peekToken();
		}

		currentClass = root->createClass(id, superclass); //That's the pointer we are going to use most of the time to build this class
		if(currentClass == nullptr)
		{
			error("Double definition of class " + id.toString(), t.getLine());
		}

		if(t.is(Token::TT_KEYWORD, "binds"))
		{
			lexer.readToken(); //Consume the "binds" keyword

			t = lexer.readToken();
			if(t.getType() != Token::TT_IDENT)
			{
				error("Expected native bind unit name after binds keyword, but found: " + t.getLexem(), t.getLine());
			}

			currentClass->setNativeTarget(t.getLexem());

			t = lexer.peekToken();
		}

    	writeASM("#-------Class " + id.toString() + " start ");

    	while(!t.is(Token::TT_KEYWORD,"end"))
    	{
    		if(t.getType() == Token::TT_IDENT) // private variable / left eval
    		{

    			if((namespaceExists(t.getLexem()) || lookupClassByName(t.getLexem()) != nullptr)) //private variable
				{

    				variableDec(VF_PRIVATE, Scope::GLOBAL);

				}else
				{
					leftEval(Scope::GLOBAL);
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
    				lexer.readToken(); //last token was only peek'd, but was identified as modifier so take it off stream before processing

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
    					if(currentClass->getNativeTarget().empty())
    					{
    						error("Native modifier can't be used in unbound classes", t.getLine());
    					}

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

    				t = lexer.peekToken();

    			}while(isModifier(t));


    			if((flags & VF_PUBLIC) && (flags & VF_PRIVATE))
				{
					error("Conflicting access modifiers. Element was declared both public and private.", t.getLine());
				}

    			if((flags & VF_STATIC) && !(flags & VF_NATIVE))
				{
					error("Statics are only valid when native.", t.getLine());
				}

    			//Now that we have collected the flags we need to check what kind of element we are declaring/defining
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

    		}else if(t.isType(Token::TT_EOF))
    		{

    			error("Unexpected end of stream in class block", t.getLine());

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
			error("Fatal error: DefaultCompiler generated invalid bytecode. Markers were requested but not defined. This is most likely a compiler bug.", 0);
		}

    	//we need to check if class is complete (all functions have adresses)
    	const std::vector<const Function*> funcs = currentClass->listFunctions();
    	for(auto iter = funcs.begin(); iter != funcs.end(); iter++)
		{
    		const Function *f = (*iter);

			if(!f->hasAdress() && !f->isNative())
			{
				error("Non-native function '" + Function::getInfoString(f->getName(), f->getParamTypes()) + "' of current class was declared but never defined", t.getLine());
			}
		}

    	currentClass->copyProgramArray(asmout.getBuffer(), asmout.getSize());// give the class it's bytecode

    	asmout.reset();

    	classList.push_back(currentClass); //The class is finished, so store it for listing
    }

    BlockInfo DefaultCompiler::block(BlockInfo::blockType_t blockType, const Scope &scope)
	{
    	uint32_t blocksInThisBlock = 0;
    	uint32_t localsInThisBlock = 0;

    	BlockInfo::blockType_t nextBlock = BlockInfo::BT_FUNC;

    	if(blockType != BlockInfo::BT_FUNC) //Block header of function blocks are created by the function declaration routine
    	{
    		writeASM("BEGIN x");
    		asmout << OP_BEGIN;
    		asmout.writeMarker(String("block_uid") + scope.getUniqueId() + "_localCount");
    	}

		Token t = lexer.peekToken();

		while(!t.is(Token::TT_KEYWORD,"end"))
		{
			if(t.getType() == Token::TT_IDENT) // private variable / left eval
			{

				if((namespaceExists(t.getLexem()) || lookupClassByName(t.getLexem()) != nullptr)) //private variable
				{

					variableDec(VF_LOCAL | VF_PRIVATE, scope);

				}else
				{
					leftEval(scope); //If the ident is no type, it is the beginning of an expression or an error
				}

			}else if(isPrimitive(t)) //private variable
			{
				variableDec(VF_LOCAL | VF_PRIVATE, scope);
				localsInThisBlock++; //TODO: We need to ensure the above statement alway creates only ONE local

			}else if(isAccessModifier(t) || t.is(Token::TT_KEYWORD, "native") || t.is(Token::TT_KEYWORD, "static"))
			{
				error("No variable modifiers allowed in function block", t.getLine());

			}else if(t.is(Token::TT_KEYWORD, "begin"))
			{
				lexer.readToken();

				block(BlockInfo::BT_SUB, Scope(scope.getNestId() + 1, blocksInThisBlock++, getNewUID()));

			}else if(t.is(Token::TT_KEYWORD, "return"))
			{
				lexer.readToken();

				t = lexer.peekToken();

				if(t.is(Token::TT_OPERATOR, ";"))
				{
					lexer.readToken();

				}else
				{
					if(currentFunction->getReturnType() == DataType::DTB_VOID)
					{
						error("Can not return a value in a void function. Expected semicolon after 'return' keyword", t.getLine());
					}

					ExpressionInfo info = expression(true, scope);

					if(!DataType::canCastImplicitly(info.getType(), currentFunction->getReturnType()))
					{
						error("Type mismatch in return statement: Can not implicitly cast " + info.getType().toString() + " to " + currentFunction->getReturnType().toString(), t.getLine());
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
				if(blockType == BlockInfo::BT_DOWHILE) //a while closes a do-while block in the way an "end" does
				{
					lexer.readToken(); //consume the while, then return

					break; //further processing after end of loop

				}else
				{
					lexer.readToken();

					int32_t currentWhileUID = getNewUID();

					writeASM(String("while_uid") + currentWhileUID + "_start:");
					asmout.defineMarker(String("while_uid") + currentWhileUID + "_start");

					ExpressionInfo info = expression(true, scope);
					if(!info.getType().isNumeric())
					{
						error("Loop condition in while statement must be numeric, but non-numeric type " + info.getType().toString() + " was given", t.getLine());
					}

					writeASM(String("JUMPFALSE while_uid") + currentWhileUID + "_end");
					asmout << OP_JUMP_IF_FALSE;
					asmout.writeMarker(String("while_uid") + currentWhileUID + "_end");

					block(BlockInfo::BT_WHILE, Scope(scope.getNestId() + 1, blocksInThisBlock++, currentWhileUID));

					writeASM(String("JUMP while_uid") + currentWhileUID + "_start");
					asmout << OP_JUMP;
					asmout.writeMarker(String("while_uid") + currentWhileUID + "_start");

					writeASM(String("while_uid") + currentWhileUID + "_end:");
					asmout.defineMarker(String("while_uid") + currentWhileUID + "_end");
				}

			}else if(t.is(Token::TT_KEYWORD, "do"))
			{
				lexer.readToken();

				int32_t currentDoWhileUID = getNewUID();

				writeASM(String("dowhile_uid") + currentDoWhileUID + "_start:");
				asmout.defineMarker(String("dowhile_uid") + currentDoWhileUID + "_start");


				block(BlockInfo::BT_DOWHILE, Scope(scope.getNestId() + 1, blocksInThisBlock++, currentDoWhileUID));


				ExpressionInfo info = expression(true, scope);
				if(!info.getType().isNumeric())
				{
					error("Loop condition in do-while statement must be numeric, but non-numeric type " + info.getType().toString() + " was given", t.getLine());
				}

				writeASM("INVERT"); //TODO: Think of what kind of architecture we want to implement. RISC, so we would do something like what we have here, or CISC, where we would define a new op JUMPTRUE.
				                    //RISC would make bytecode bigger, but on the other hand it would reduce the number of ops we have to check in the ID cycle.
				writeASM(String("JUMPFALSE dowhile_uid") + currentDoWhileUID + "_start");
				asmout << OP_INVERT;
				asmout << OP_JUMP_IF_FALSE;
				asmout.writeMarker(String("dowhile_uid") + currentDoWhileUID + "_start");

				writeASM(String("#dowhile_uid") + currentDoWhileUID + "_end:");

				Token t2 = lexer.readToken();
				if(!t2.is(Token::TT_OPERATOR, ";"))
				{
					error("Expected semicolon after loop condition of do-while statement, but found: " + t2.getLexem(), t2.getLine());
				}

			}else if(t.is(Token::TT_KEYWORD, "if"))
			{
				lexer.readToken();

				ifStatement(blockType, scope, blocksInThisBlock);

			}else if(t.is(Token::TT_KEYWORD, "else"))
			{
				lexer.readToken();

				if(blockType == BlockInfo::BT_IF)
				{

					nextBlock = BlockInfo::BT_ELSE;
					break; //Returns to ifStatement() after loop end, further processing happens there

				}else if(blockType == BlockInfo::BT_ELSEIF)
				{

					//Should never occur. Don't take it seriously
					error("[COMPILER BUG] That isn't going to do you any good, Flynn.", t.getLine());

				}else
				{
					error("Else-block must be part of if- or elseif-block", t.getLine());
				}

			}else if(t.is(Token::TT_KEYWORD, "elseif"))
			{
				lexer.readToken();

				if(blockType == BlockInfo::BT_IF)
				{

					nextBlock = BlockInfo::BT_ELSEIF;
					break; //Returns to ifStatement() after end of loop, further processing happens there

				}else if(blockType == BlockInfo::BT_ELSEIF)
				{

					//Should never occur. Don't take it seriously
					error("[COMPILER BUG] I'm sorry, Dave. I'm afraid I can't do that.", t.getLine());

				}else if(blockType == BlockInfo::BT_ELSE)
				{

					error("Else-block must be defined before else-block", t.getLine());

				}else
				{
					error("Elseif-block must be part of if-block", t.getLine());
				}

			}else if(t.is(Token::TT_KEYWORD, "using"))
    		{
    			lexer.readToken();

    			usingStatement();

    		}else if(isStartOfExpression(t)) //start of expression
			{
				//some non-ident-tokens like this, new and openening parentheses may open expressions. If not, they are errors and detected as such by the expression parser

				leftEval(scope);

			}else if(t.getType() == Token::TT_EOF)
			{
				error("Unexpected end of stream in function block", t.getLine());

			}else
			{
				error("Unexpected token in function block: " + t.getLexem(), t.getLine());
			}

			t = lexer.peekToken();
		}

		if(t.is(Token::TT_KEYWORD, "end")) //if there was an "end", consume it. otherwise, leave the token in the stream
		{
			lexer.readToken();
		}

		writeASM(String("END ") + localsInThisBlock); //block footer is always generated; no need to check for func
		asmout << OP_END;
		asmout.writeUInt(localsInThisBlock);

		asmout.defineMarker(String("block_uid") + scope.getUniqueId() + "_localCount", localsInThisBlock);

		return BlockInfo(localsInThisBlock, 0, nextBlock); //return to surrounding block
	}

    BlockInfo DefaultCompiler::ifStatement(const BlockInfo::blockType_t &blockType, const Scope &scope, uint32_t &blocksInThisBlock)
    {
    	//Note: The concept of if block parsing was developed back in KniftoScript2 days, and I have completely forgotten if there
    	//were any better solutions that didn't make it into the implementation. Anyway, the current one messes up the whole compiler.
    	//Although this code reminds me of the wonderful holiday I had when I was thinking about advanced If blocks how they are right now, we might
    	//improve this so the code becomes more structured and clearer. TODO: Maybe improve if block parsing

    	//And also: TODO: I have a bad feeling about this function. Check if everything would work as intended

    	uint32_t localsInThisIf = 0;

    	ExpressionInfo info = expression(true, scope);
		if(!info.getType().isNumeric())
		{
			error("Condition in if statement must be numeric, but non-numeric type " + info.getType().toString() + " was given", lexer.getCurrentLine());
		}

		uint32_t currentIfUID = getNewUID();

		writeASM(String("JUMPFALSE if_uid") + currentIfUID + "_end");
		asmout << OP_JUMP_IF_FALSE;
		asmout.writeMarker(String("if_uid") + currentIfUID + "_end");

		BlockInfo binf = block(BlockInfo::BT_IF, Scope(scope.getNestId()+1, blocksInThisBlock++, currentIfUID));

		if(binf.getFollowingBlock() == BlockInfo::BT_ELSE || binf.getFollowingBlock() == BlockInfo::BT_ELSEIF)
		{
			writeASM(String("JUMP if_else_uid") + currentIfUID + "_end");
			asmout << OP_JUMP;
			asmout.writeMarker(String("if_else_uid") + currentIfUID + "_end");
		}

		writeASM(String("if_uid") + currentIfUID + "_end:");
		asmout.defineMarker(String("if_uid") + currentIfUID + "_end");

		if(binf.getFollowingBlock() == BlockInfo::BT_ELSE || binf.getFollowingBlock() == BlockInfo::BT_ELSEIF)
		{
			writeASM(String("# if_else_uid") + currentIfUID + "_start:");

			if(binf.getFollowingBlock() == BlockInfo::BT_ELSE)
			{
				uint32_t currentElseUID = getNewUID();

				block(BlockInfo::BT_ELSE, Scope(scope.getNestId(), blocksInThisBlock++, currentElseUID));

			}else
			{
				ifStatement(blockType, scope, blocksInThisBlock);
			}

			writeASM(String("if_else_uid") + currentIfUID + "_end:");
			asmout.defineMarker(String("if_else_uid") + currentIfUID + "_end");
		}

		return BlockInfo(localsInThisIf, 0, BlockInfo::BT_FUNC);
    }

    void DefaultCompiler::usingStatement()
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

    	ClassID cid = classID();
    	const Class *cl = root->getClass(cid);
    	if(cl == nullptr)
    	{
    		error("Class " + cid.toString() + " in using statement was not found", t.getLine());
    	}

    	t = lexer.readToken();

    	if(!t.is(Token::TT_OPERATOR, ";"))
    	{
    		error("Expected semicolon after using statement, but found: " + t.getLexem(), t.getLine());
    	}

    	//First, remove all imports with the same classname as the new import (since redefinition is allowed)
    	auto iter = importList.begin();
		while(iter != importList.end())
		{
			if((*iter)->getID().getClassname() == cid.getClassname())
			{
				iter = importList.erase(iter);

			}else
			{
			   iter++;
			}
		}

    	importList.push_back(cl);
    }

    void DefaultCompiler::leftEval(const Scope &scope)
	{
    	ExpressionInfo left = expression(false, scope); //nope, this is not a value context. we don't want anything to stay on the stack, please

    	Token t = lexer.readToken();
    	if(!t.is(Token::TT_OPERATOR, ";"))
    	{
    		error("Expected semicolon after expression, but found: " + t.getLexem(), t.getLine());
    	}
	}

	void DefaultCompiler::variableDec(uint32_t varFlags, const Scope &currentScope)
	{
		DataType type = dataType();

		Token ident = name();

		if(!(varFlags & VF_LOCAL) && (currentClass->getField(ident.getLexem()) != nullptr)) // note: we don't check for inherited fields. they may be overridden.
		{
			//Globals must not override globals or overridden globals
			error("Double declaration of variable " + ident.getLexem() + " in global scope", ident.getLine());

		}else if((varFlags & VF_LOCAL) && (lookupLocal(ident.getLexem(), currentScope) != nullptr))
		{
			//Locals must not override locals
			error("Double declaration of variable " + ident.getLexem() + " in local scope", ident.getLine());
		}


		Token t = lexer.readToken();

		if(varFlags & VF_LOCAL)
		{
			locals.push_back(Local(ident.getLexem(), type, currentScope, locals.size()));

			// imperative declaration of local
			writeASM("DECLARELOCAL '" + ident.getLexem() + "'," + (int)type.getBase());
			asmout << OP_DECLARELOCAL;
			asmout.writeBString(ident.getLexem());
			writeDatatypeToBytecode(type);

		}else // var is global
		{
			Field *f = currentClass->createField(ident.getLexem(), type);

			if(f == nullptr)
			{
				//This could actually mean there was a double declaration, but since we already checked for that, we call it a compiler bug

				error("[COMPILER BUG] Uncatched null during field creation: " + ident.getLexem(), ident.getLine());
			}

			f->setStatic(varFlags & VF_STATIC);
			f->setPublic(!(varFlags & VF_PRIVATE));
			f->setNative(varFlags & VF_NATIVE);
		}

		if(t.is(Token::TT_OPERATOR, "="))
		{
			if(varFlags & VF_NATIVE)
			{
				error("Can not initialize native variables. Expected semicolon after variable identifier.", t.getLine());
			}

			ExpressionInfo info = expression(true, currentScope);

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
			asmout << OP_POP_VAR;

		}else if(!t.is(Token::TT_OPERATOR, ";"))
		{
			error("Expected semicolon or assignment after variable declaration, but found: " + t.getLexem(), t.getLine());
		}
	}

    void DefaultCompiler::functionDec(uint32_t funcFlags)
    {
    	Token t = lexer.readToken(); //Read the left recognizer

    	DataType returnType = DataType(DataType::DTB_VOID);
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


		TypeList paramTypes;
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

				paramNames.push_back(t.getLexem());

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

		//first, check if there's already a function like this one in this very class
		const Function *oldFunc = currentClass->getFunction(ident.getLexem(), paramTypes);
		if(oldFunc != nullptr)
		{
			//There's already a function with that name and those specific parameter types. That's only valid if that function hasn't got a body yet.
			// otherwise -> error
			if(oldFunc->hasAdress())
			{
				error("Function " + Function::getInfoString(ident.getLexem(), paramTypes) + " was already defined in class " + currentClass->getID().toString(), ident.getLine());

			}else
			{
				// The old function is only a prototype. since we can't modify it from here, we remove the prototype from the class and recreate it later
				currentClass->removeFunction(oldFunc);
			}

		}else if(currentClass->getSuperclass() != nullptr) //if there's no similar function, and the class has a superclass, check if a function is inherited from a superclass
		{
			const Function *oldSuperFunc = currentClass->getSuperclass()->getJoinedFunction(ident.getLexem(), paramTypes);

			if(oldSuperFunc != nullptr)
			{
				//there is a similar function, but we don't mind. it may be overridden, as long as return type, access type etc. are the same

				if(oldSuperFunc->isNative()) //natives, however, must never be overridden
				{
					error("Native function " + Function::getInfoString(ident.getLexem(), paramTypes) + " in superclass must not be overridden", ident.getLine());

				}else if(!(oldSuperFunc->getReturnType() == returnType) || (oldSuperFunc->isPublic() != !(funcFlags & VF_PRIVATE)))
				{
					error("Overriding function " + Function::getInfoString(ident.getLexem(), paramTypes) + " must not change return or access type", ident.getLine());
				}

				//nope, everything in order. we may override the function

			}
		}

		//We are ready to create the function in the class
		currentFunction = currentClass->createFunction(ident.getLexem(), paramTypes);
		if(currentFunction == nullptr)
		{
			//because of the last check, this should never occur. check it anyway.

			error("Double definition of function " + Function::getInfoString(ident.getLexem(), paramTypes) + " in class " + currentClass->getID().toString(), ident.getLine());
		}

		currentFunction->setReturnType(returnType);
		currentFunction->setStatic(funcFlags & VF_STATIC);
		currentFunction->setPublic(!(funcFlags & VF_PRIVATE));
		currentFunction->setNative(funcFlags & VF_NATIVE);

		//Check if function has a body
		t = lexer.peekToken();
		if(t.is(Token::TT_OPERATOR, ";"))
		{
			//This is just a prototype declaration, so we leave the function adressless and carry on

			lexer.readToken(); // but remove the semicolon from the stream first!

			writeASM("#prototype'd " + Function::getInfoString(ident.getLexem(), paramTypes));

			return; //We are finished here (for now. this prototype needs an adress and a body sooner or later, which must be supplied by the programmer)

		}else if(funcFlags & VF_NATIVE) //If function is native, insist for the semicolon!
		{
			error(String("Native functions must not have a body. Expected semicolon after parameter list") ,t.getLine());
		}


		//Okay, function has a body. Let's turn it into bytecode and give the prototype an adress

		int currentFunctionUID = getNewUID();
		Scope scopeOfFuncBlock = Scope(1,0,currentFunctionUID); //nestId of 0 is reserved for globals


		writeASM("# definition of function " + Function::getInfoString(ident.getLexem(), paramTypes));
		writeASM("JUMP func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_end");
		asmout << OP_JUMP;
		asmout.writeMarker("func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_end");

		currentFunction->setAdress(asmout.getSize()); //Now give the function an adress

		writeASM("func_" + ident.getLexem() + "_uid" + currentFunctionUID +  "_start:");
		asmout.defineMarker("func_" + ident.getLexem() + "_uid" + currentFunctionUID +  "_start");

		writeASM("BEGIN x");
		asmout << OP_BEGIN;
		asmout.writeMarker("func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_localCount");

		locals.clear(); //delete any locals remaining from the last compiled function

		for(int i = paramTypes.size()-1; i >= 0 ; i--)
		{
			String paraName = paramNames[i];

			//Declare local
			locals.push_back(Local(paraName, paramTypes[i], scopeOfFuncBlock, locals.size()));

			writeASM("DECLARELOCAL '" + paraName + "'," + (uint32_t)paramTypes[i].getBase());
			asmout << OP_DECLARELOCAL;
			asmout.writeBString(paraName);
			writeDatatypeToBytecode(paramTypes[i]);

			//Get the local from stack

			writeASM("POPVAR '" + paraName + "'");
			asmout << OP_POP_VAR;
			asmout.writeBString(paraName);
		}

		BlockInfo binf = block(BlockInfo::BT_FUNC, scopeOfFuncBlock);

		asmout.defineMarker("func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_localCount", binf.getLocalCount());

		writeASM("RETURN");
		asmout << OP_RETURN;

		writeASM("func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_end:");
		asmout.defineMarker("func_" + ident.getLexem() + "_uid" + currentFunctionUID + "_end");
    }

    TypeList DefaultCompiler::parameterList(const Scope &scope)
    {
		TypeList paramTypes;

		Token t = lexer.peekToken();
		if(!t.is(Token::TT_OPERATOR,")"))
		{

			do
			{
				ExpressionInfo info = expression(true, scope);

				paramTypes.push_back(info.getType());

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

		return paramTypes;
    }

    DataType DefaultCompiler::functionCall(const String &funcName, bool member, const Scope &scope, const DataType &baseType)
    {
		Token t = lexer.readToken();
		if(!t.is(Token::TT_OPERATOR,"("))
		{
			error("Expected opening parentheses after function name in function call", t.getLine());
		}

		TypeList paramTypes = parameterList(scope);

		if(member)
		{
			if(baseType.getBase() != DataType::DTB_OBJECT)
			{
				error("Only non-abstract object types have callable member functions, but given type was: " + baseType.toString(), t.getLine());
			}

			const Class *fieldClass = lookupClass(baseType.getTypeClass()->getID());

			if(fieldClass == nullptr)
			{
				error("[COMPILER BUG] Specifier of given base type for member function call points to non-existent class: " + baseType.toString(), t.getLine());
			}

			const Function *membFunction = fieldClass->getFunction(funcName, paramTypes);

			if(membFunction == nullptr)
			{
				error("Function '" + Function::getInfoString(funcName, paramTypes) + "' was not defined in given object type class '" + baseType.toString() + "'", t.getLine());
			}
			
			if(!membFunction->isPublic())
			{
				error("Function '" + Function::getInfoString(funcName, paramTypes) + "' in given object type class '" + baseType.toString() + "' is private", t.getLine());
			}

			if(!membFunction->hasAdress() && !membFunction->isNative())
			{
				//since we are checking for this once a class is finished, this should be very unlikely to occur. we still check for it since imported bytecode may contain undefined functions
				error("Non-native function '" + Function::getInfoString(funcName, paramTypes) + "' in given object type class '" + baseType.toString() + "' was declared but never defined", t.getLine());
			}

			writeASM("CALLMEMBER '" + funcName + "'," + paramTypes.size());
			asmout << OP_CALL_MEMBER;
			asmout.writeBString(funcName);
			asmout.writeUByte(paramTypes.size());

			return membFunction->getReturnType();

		}else
		{
			const Function *ownFunction = currentClass->getJoinedFunction(funcName, paramTypes); //Also check for functions of superclasses

			if(ownFunction == nullptr)
			{
				error("Function '" + Function::getInfoString(funcName, paramTypes) + "' was not defined in current class", t.getLine());
			}

			//No need to check for private, as we are calling from the same class
			
			writeASM("CALL '" + funcName + "'," + paramTypes.size());
			asmout << OP_CALL;
			asmout.writeBString(funcName);
			asmout.writeUByte(paramTypes.size());

			return ownFunction->getReturnType();
		}
    }

    ClassID DefaultCompiler::classID()
    {
    	Token t = lexer.readToken();

    	if(!t.isType(Token::TT_IDENT))
    	{
    		error("Expected classname or namespace, but found: " + t.getLexem(), t.getLine());
    	}

    	Token t2 = lexer.peekToken();

    	if(t2.is(Token::TT_OPERATOR, ":"))
    	{
    		lexer.readToken();
    		t2 = lexer.readToken();

    		if(!t2.isType(Token::TT_IDENT))
			{
				error("Expected classname after scope operator, but found: " + t2.getLexem(), t2.getLine());
			}

    		return ClassID(t.getLexem(), t2.getLexem());

    	}else
    	{
    		return ClassID("",t.getLexem());
    	}
    }

    DataType DefaultCompiler::dataType()
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
    				error("Expected class identifier after namespace operator, but found: " + t2.getLexem(), t2.getLine());
    			}

    			if(lookupClass(ClassID(t.getLexem(), t2.getLexem())) == nullptr)
    			{
    				error("Identifier '" + t2.getLexem() + "' does not name a class in namespace '" + t.getLexem() + "'", t2.getLine());
    			}

    			return DataType(DataType::DTB_OBJECT, ClassID(t.getLexem(), t2.getLexem()));

    		}else
    		{
    			//No namespace given -> search the default locations
    			const Class *typeClass = lookupClassByName(t.getLexem());

				if(typeClass == nullptr)
				{
					error("Identifier '" + t.getLexem() + "' does not name a class in current/default/imported namespace", t.getLine());
				}

    			return DataType(DataType::DTB_OBJECT, typeClass->getID()); //TODO: The class of a data type is currently pointed by name. we might point by pointer to be more efficient
    		}

    	}else
    	{
    		error("Expected primitve data type or classname, but found: " + t.getLexem(), t.getLine());
    	}

    	return DataType(DataType::DTB_VOID); //should never happen
    }

    Token DefaultCompiler::name()
    {
    	Token t = lexer.readToken();

    	if(t.getType() != Token::TT_IDENT)
		{
			error("Expected name after data type, but found: " + t.getLexem(), t.getLine());
		}

		if(namespaceExists(t.getLexem()) || (lookupClass(ClassID("",t.getLexem())) != nullptr))
		{
			error("Name '" + t.getLexem() + "' is already beeing used as a classname or namespace", t.getLine());
		}

		return t;
    }

    ExpressionInfo DefaultCompiler::expression(const bool valueContext, const Scope &expressionScope)
    {
        ExpressionInfo leftInfo = logicExpression(expressionScope);

        Token t = lexer.peekToken();
        if(isAssignmentOperator(t))
        {
        	lexer.readToken(); //remove the assignment operator from the stream, so right eval doesn't get it

        	if(leftInfo.getFactorType() != ExpressionInfo::FT_VARIABLE_REF)
        	{
        		error("Variable reference required on left side of assignment operator", t.getLine());
        	}

        	if(!t.is(Token::TT_OPERATOR, "="))
        	{
        		//We have a compound operator that requires a second instance of the variable reference for the pre-assign operation
        		writeASM("CLONE");
        		asmout << OP_CLONE;
        	}

        	ExpressionInfo rightInfo = expression(true, expressionScope);

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
				error("[COMPILER BUG] Unknown assignment operator: " + t.getLexem(), t.getLine());
			}

			if(valueContext)
			{
				writeASM("SOFTPOPREF"); //Pops to reference and leaves the assigned value on the stack
				asmout << OP_POP_REF_SOFT;

			}else
			{
				writeASM("POPREF"); // this eliminates the need for a DISCARD in a non-value context, as nothing stays on the stack after assignment here
				asmout << OP_POP_REF;
			}

			//No need to check whether the right hand is void or not, since the right hand is the left hand of another expression, and as such a check is
			//always made because it's either the end of expression(no assignment -> check) or another assignment (left hand must be reference and not void)

			return rightInfo;

        }else
        {
			//There was no assignment, so the value of the expression is the "left hand side"(the only thing we've got)

			if(valueContext && leftInfo.getFactorType() == ExpressionInfo::FT_VOID) //in a value context, this must not be void
			{
				error("Expression was used in a value-context and must therefore yield a value, but just contains a call to a void function", t.getLine());
			}

			if(!valueContext && leftInfo.getFactorType() != ExpressionInfo::FT_VOID) //in a non-value context, any stack elements must be destroyed
			{
				writeASM("DISCARD");
				asmout << OP_DISCARD;
			}

			return leftInfo;
        }
    }

    ExpressionInfo DefaultCompiler::logicExpression(const Scope &expressionScope)
    {
    	ExpressionInfo info = relationalExpression(expressionScope);

		Token t = lexer.peekToken();
		while(isLogicOp(t))
		{
			if(info.getFactorType() == ExpressionInfo::FT_VOID)
			{
				error("Logic operations not possible with void type expression", t.getLine());
			}

			lexer.readToken();

			ExpressionInfo rightInfo = relationalExpression(expressionScope);

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
				asmout << OP_LOGIC_OR;

			}else if(t.is(Token::TT_OPERATOR,"&"))
			{
				writeASM("LOGICAND");
				asmout << OP_LOGIC_AND;
			}

			t = lexer.peekToken();
		}

		return info;
    }

    ExpressionInfo DefaultCompiler::relationalExpression(const Scope &expressionScope)
    {
        ExpressionInfo info = arithmeticExpression(expressionScope);

        Token t = lexer.peekToken();
        while(isRelationalOp(t))
        {
        	if(info.getFactorType() == ExpressionInfo::FT_VOID)
			{
				error("Relation operations not possible with void type expression", t.getLine());
			}

            lexer.readToken();

            ExpressionInfo rightInfo = arithmeticExpression(expressionScope);
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
                asmout << OP_LESS_EQUAL;

            }else if(t.is(Token::TT_OPERATOR,">="))
            {
                writeASM("GREATEREQUAL");
                asmout << OP_GREATER_EQUAL;

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

    ExpressionInfo DefaultCompiler::arithmeticExpression(const Scope &expressionScope)
    {
    	ExpressionInfo info = term(expressionScope);

        Token t = lexer.peekToken();
        while(isAddOp(t))
        {
        	if(info.getFactorType() == ExpressionInfo::FT_VOID)
			{
				error("Mathematic operations not possible with void type expression", t.getLine());
			}

            lexer.readToken();

            ExpressionInfo rightInfo = term(expressionScope);

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

    ExpressionInfo DefaultCompiler::term(const Scope &expressionScope)
    {
    	ExpressionInfo info = castFactor(expressionScope);

        Token t = lexer.peekToken();
        while(isMultiplyOp(t))
        {
        	if(info.getFactorType() == ExpressionInfo::FT_VOID)
			{
				error("Mathematic operations not possible with void type expression", t.getLine());
			}

            lexer.readToken();

            ExpressionInfo rightInfo = castFactor(expressionScope);

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

    ExpressionInfo DefaultCompiler::castFactor(const Scope &expressionScope)
	{
		ExpressionInfo info = signedFactor(expressionScope);

		Token t = lexer.peekToken();
		if(t.is(Token::TT_OPERATOR,"->"))
		{
			if(info.getFactorType() == ExpressionInfo::FT_VOID)
			{
				error("Casts not possible with void type expression", t.getLine());
			}

			if(info.getType() == DataType::DTB_VOID)
			{
				error("Can not cast nulltype to anything", t.getLine());
			}

			lexer.readToken();

			DataType type = dataType();

			if(type == DataType::DTB_OBJECT)
			{
				writeASM(String("TOOBJECT '") + type.getTypeClass()->getID().getNamespace() + "','" + type.getTypeClass()->getID().getClassname() + "'");
				asmout << OP_TO_OBJECT;
				asmout.writeBString(type.getTypeClass()->getID().getNamespace());
				asmout.writeBString(type.getTypeClass()->getID().getClassname());

			}else
			{
				writeASM(String("TO") + type.getBase());
				asmout << (OP_TO_INT + type.getBase());
			}

			return ExpressionInfo(type, false, ExpressionInfo::FT_MATH_EXPR);
		}

		return info;
	}

    ExpressionInfo DefaultCompiler::signedFactor(const Scope &expressionScope)
	{
		bool negate = false;
		bool invert = false;

		Token t = lexer.peekToken();
		if(t.is(Token::TT_OPERATOR,"-"))
		{
			negate = true;
			lexer.readToken();

		}else if(t.is(Token::TT_OPERATOR,"!"))
		{
			invert = true;
			lexer.readToken();

		}

		ExpressionInfo info = memberFactor(expressionScope);

		if(info.getFactorType() == ExpressionInfo::FT_VOID && (negate || invert))
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

		return info;
	}

    ExpressionInfo DefaultCompiler::memberFactor(const Scope &expressionScope)
    {
        ExpressionInfo info = indexFactor(expressionScope);

        Token t = lexer.peekToken();
        while(t.is(Token::TT_OPERATOR,"."))
        {
        	if(info.getFactorType() == ExpressionInfo::FT_VOID)
			{
				error("Member access not possible with void type expression", t.getLine());
			}

        	if(!(info.getType().getBase() == DataType::DTB_OBJECT))
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
                DataType ftype = functionCall(member.getLexem(), true, expressionScope, info.getType());

                info = ExpressionInfo(ftype, false, (ftype == DataType::DTB_VOID) ? ExpressionInfo::FT_VOID : ExpressionInfo::FT_FUNCTION_RETURN);

            }else
            {
            	const Class *s = lookupClass(info.getType().getTypeClass()->getID());
				if(s == nullptr)
				{
					error("Given object type has non-existent class '" + info.getType().toString() + "'", member.getLine());
				}

				const Field *f = s->getJoinedField(member.getLexem());
				if(f == nullptr)
				{
					error("Class of referenced type '" + info.getType().toString() + "' has no member '" + member.getLexem() + "'", member.getLine());
				}

				if(!f->isPublic() && !(s->getID() == currentClass->getID()))
				{
					//If a private object member is accessed from a class of the same type as the object, the access modifier is ignored (like all methods are declared as friend)
					//Otherwise, accessing privates is an access violation, so emit an error

					error("Variable '" + member.getLexem() + "' in referenced class '" + info.getType().toString() + "' is private", member.getLine());
				}

                info = ExpressionInfo(f->getType(), false, ExpressionInfo::FT_VARIABLE_REF);

                writeASM("GETMEMBER '" + member.getLexem() + "'");
                asmout << OP_GET_MEMBER;
                asmout.writeBString(member.getLexem());
            }
        }

        return info;
    }

    ExpressionInfo DefaultCompiler::indexFactor(const Scope &expressionScope)
    {
    	ExpressionInfo info = factor(expressionScope);

    	Token t = lexer.peekToken();
    	if(t.is(Token::TT_OPERATOR, "["))
		{
    		if(info.getFactorType() == ExpressionInfo::FT_VOID)
    		{
    			error("Array access not possible with void type expression", t.getLine());
    		}

    		if(info.getType().getBase() != DataType::DTB_STRING)
    		{
    			error("Currently only strings are accessible by the [] operator", t.getLine());
    		}

			lexer.readToken();

			ExpressionInfo index = expression(true, expressionScope);
			if(index.getType().getBase() != DataType::DTB_INT)
			{
				error("Invalid array index type. Array indices must be of type int", t.getLine());
			}

			writeASM("GETINDEX");
			asmout << OP_GET_INDEX;

			t = lexer.readToken();
			if(!t.is(Token::TT_OPERATOR, "]"))
			{
				error("Missing closing square brackets after array index expression",t.getLine());
			}

			return ExpressionInfo(info.getType(), false, ExpressionInfo::FT_VARIABLE_REF);
		}

    	return info;
    }

    ExpressionInfo DefaultCompiler::factor(const Scope &expressionScope)
    {
        Token t = lexer.readToken();

        if(t.getType() == Token::TT_NUMBER)
        {
            DataType numberType = getTypeOfNumberString(t.getLexem());

            if(numberType == DataType::DTB_VOID)
            {
                error("Number literal '" + t.getLexem() + "' does not fit in any numerical data type",t.getLine());
            }

            writeASM(String("PUSH ") + ((int)numberType.getBase()) + ", " + t.getLexem());
            asmout << OP_PUSH_INT + numberType.getBase();
            switch(numberType.getBase())
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
            	error("[COMPILER BUG] Unknown number type", t.getLine());
            	break;
            }

            return ExpressionInfo(numberType, true, ExpressionInfo::FT_LITERAL);

        }else if(t.getType() == Token::TT_STRING)
        {
            writeASM("PUSHSTRING '" + escapeASMChars(t.getLexem()) + "'");
            asmout << OP_PUSH_STRING;
            asmout.writeIString(t.getLexem()); //TODO: Process escape sequences here

           return ExpressionInfo(DataType(DataType::DTB_STRING), true, ExpressionInfo::FT_LITERAL);

        }else if(t.getType() == Token::TT_IDENT)
        {
        	//TODO: This whole part is most disgusting. Exchange it with something more efficient, and with less redundant code

            Token t2 = lexer.peekToken();
            if(t2.is(Token::TT_OPERATOR,"("))
            {
                DataType type = functionCall(t.getLexem(), false, expressionScope);

                return ExpressionInfo(type, false, (type == DataType::DTB_VOID) ? ExpressionInfo::FT_VOID : ExpressionInfo::FT_FUNCTION_RETURN);

            }else if(t2.is(Token::TT_OPERATOR, ":"))
            {
            	lexer.readToken();

            	Token secondIdent = lexer.readToken();
            	if(!secondIdent.isType(Token::TT_IDENT))
            	{
            		error("Expected identifier after scope operator, but found: " + secondIdent.getLexem(), secondIdent.getLine());
            	}

            	t2 = lexer.peekToken();
            	if(t2.is(Token::TT_OPERATOR, ":")) //static call / static variable access
            	{
            		ClassID targetClassID = ClassID(t.getLexem(), secondIdent.getLexem());
            		const Class *targetClass = root->getClass(targetClassID);
					if(targetClass == nullptr)
					{
						error("Given class " + targetClassID.toString() + " was not defined. ", secondIdent.getLine());
					}

            		lexer.readToken();
            		Token thirdIdent = lexer.readToken();
            		if(!thirdIdent.isType(Token::TT_IDENT))
					{
						error("Expected identifier after scope operator, but found: " + thirdIdent.getLexem(), thirdIdent.getLine());
					}

            		t2 = lexer.readToken();
            		if(t2.is(Token::TT_OPERATOR, "(")) //static call
            		{

            			TypeList paramTypes = parameterList(expressionScope);

            			const Function *targetFunc = targetClass->getFunction(thirdIdent.getLexem(), paramTypes);
            			if(targetFunc == nullptr)
            			{
            				error("Static function " + Function::getInfoString(targetClassID.toString() + ":" + thirdIdent.getLexem(), paramTypes) + " was not defined", thirdIdent.getLine());
            			}

            			if(!targetFunc->isStatic())
            			{
            				error("Can not call non-static function " + Function::getInfoString(targetClassID.toString() + ":" + thirdIdent.getLexem(), paramTypes) + " in static context", thirdIdent.getLine());
            			}

            			asmout << OP_CALL_STATIC;
            			asmout.writeBString(t.getLexem());
            			asmout.writeBString(secondIdent.getLexem());
            			asmout.writeBString(thirdIdent.getLexem());
            			asmout.writeUByte(paramTypes.size());


            			return ExpressionInfo(targetFunc->getReturnType(), false, (targetFunc->getReturnType() == DataType::DTB_VOID) ? ExpressionInfo::FT_VOID : ExpressionInfo::FT_FUNCTION_RETURN);

            		}else // static variable access
            		{
            			const Field *targetField = targetClass->getField(thirdIdent.getLexem());

            			if(targetField == nullptr)
						{
							error("Static field " + targetClassID.toString() + ":" + thirdIdent.getLexem() + " was not declared", thirdIdent.getLine());
						}

						if(!targetField->isStatic())
						{
							error("Can not access non-static field " + targetClassID.toString() + ":" + thirdIdent.getLexem() + " in static context", thirdIdent.getLine());
						}

						writeASM("PUSHSTATICREF");
						asmout << OP_PUSH_STATIC_REF;
						asmout.writeBString(t.getLexem());
						asmout.writeBString(secondIdent.getLexem());
						asmout.writeBString(thirdIdent.getLexem());

						return targetField->getType();
            		}

            	}else if(t2.is(Token::TT_OPERATOR, "(")) //static call (of class in default/imported ns)
            	{
					const Class *targetClass = lookupClass(ClassID("",t.getLexem()));
					if(targetClass == nullptr)
					{
						error("Given class " + t.getLexem() + " was not found in imported/local/default namespace", t.getLine());
					}

            		TypeList paramTypes = parameterList(expressionScope);

					const Function *targetFunc = targetClass->getFunction(secondIdent.getLexem(), paramTypes);
					if(targetFunc == nullptr)
					{
						error("Static function " + Function::getInfoString(targetClass->getID().toString() + ":" + secondIdent.getLexem(), paramTypes) + " was not defined", secondIdent.getLine());
					}

					if(!targetFunc->isStatic())
					{
						error("Can not call non-static function " + Function::getInfoString(targetClass->getID().toString() + ":" + secondIdent.getLexem(), paramTypes) + " in static context", secondIdent.getLine());
					}

					asmout << OP_CALL_STATIC;
					asmout.writeBString("");
					asmout.writeBString(t.getLexem());
					asmout.writeBString(secondIdent.getLexem());
					asmout.writeUByte(paramTypes.size());

					return ExpressionInfo(targetFunc->getReturnType(), false, (targetFunc->getReturnType() == DataType::DTB_VOID) ? ExpressionInfo::FT_VOID : ExpressionInfo::FT_FUNCTION_RETURN);

            	}else if(t2.isType(Token::TT_IDENT)) //variable declaration
            	{

            	}else //static variable access (of class in default/imported ns)
            	{

            	}

            }else
            {
            	DataType fieldType(DataType::DTB_VOID);

            	const Field *f = currentClass->getJoinedField(t.getLexem());
				if(f != nullptr)
				{
					//We found the field in global scope! We've already got it's name, so only extract it's type.
					fieldType = f->getType();

					writeASM("PUSHREF '" + t.getLexem() + "'");
					asmout << OP_PUSH_REF;
					asmout.writeBString(t.getLexem());

				}else
				{
					//Field was not found in global scope. Check for local scope.

					Local *l = lookupLocal(t.getLexem(), expressionScope);
					if(l != nullptr)
					{
						//there it is. since locals are not accessed by name during runtime, we need to get it's ordinal and push it accordingly

						fieldType = l->getType();

						writeASM(String("PUSHLOCALREF ") + l->getIndex());
						asmout << OP_PUSH_LOCAL_REF;
						asmout.writeUInt(l->getIndex());

					}else
					{
						//Also not found in local scope. That's an error
						error("Variable/type '" + t.getLexem() + "' was not declared in this scope", t.getLine());
					}
				}

                return ExpressionInfo(fieldType, false, ExpressionInfo::FT_VARIABLE_REF);
            }

        }else if(t.is(Token::TT_OPERATOR,"("))
        {
            ExpressionInfo info = expression(true, expressionScope);

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
            ClassID classtype = classID();

			Token t2 = lexer.readToken();
			if(!t2.is(Token::TT_OPERATOR,"("))
			{
				error("Expected parameter list after classname in constructor call, but found: " + t2.getLexem(), t2.getLine());
			}

			int paramCount = 0;

			TypeList paramTypes = TypeList();

			t2 = lexer.peekToken();
			if(!t2.is(Token::TT_OPERATOR,")"))
			{

				do
				{
					ExpressionInfo info = expression(true, expressionScope);

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
			
			const Class *instClass = lookupClass(classtype);
			if(instClass == nullptr)
			{
				error("Given identifier '" + classtype.toString() + "' in new statement does not name a type", t2.getLine());
			}
			
			const Function *constr = instClass->getFunction("init", paramTypes);
			if(constr == nullptr)
			{
				//there are no implicit constructors. if a class has no defined constructors, it can not be instantiated.
				error("Constructor " + Function::getInfoString(instClass->getID().toString(), paramTypes) + " was not declared", t2.getLine());
				
			}else
			{
				if(!constr->isPublic())
				{
					error("Constructor " + Function::getInfoString(instClass->getID().toString(), paramTypes) + " is private", t2.getLine());
				}
			}
			
			writeASM("NEW '" + instClass->getID().getNamespace() + "','" + instClass->getID().getClassname() + "'," + paramCount);
			asmout << OP_NEW;
			asmout.writeBString(instClass->getID().getNamespace());
			asmout.writeBString(instClass->getID().getClassname());
			asmout.writeUByte(paramCount);

			return ExpressionInfo(DataType(DataType::DTB_OBJECT, instClass->getID()), false, ExpressionInfo::FT_FUNCTION_RETURN); //Constructors are actually void functions, but the new statement returns the instance, so it is considered as a non-void function call

        }else if(t.is(Token::TT_KEYWORD,"null"))
        {
            writeASM("PUSHNULL");
            asmout << OP_PUSH_NULL;

            return ExpressionInfo(DataType(DataType::DTB_VOID), true, ExpressionInfo::FT_LITERAL);

        }else if(t.is(Token::TT_KEYWORD,"this"))
        {
        	//TODO: Private variables in "this" are currently not accessible, this has to be changed
        	//Note: the public API for classes should not give any access to locals. Same goes for the one used for variable type stuff during compile time
        	//so this.xyz won't let the program access locals. (Functions where locals have same name as globals which should still be accessed can be by using 'this')

        	t = lexer.peekToken();

        	if(t.is(Token::TT_OPERATOR, "("))
        	{
        		if(!currentFunction->isConstructor())
        		{
        			error("Constructor delegation not possible outside of constructor", t.getLine());
        		}

        		functionCall("init",false, expressionScope); //Calling a constructor of the current class

        		return ExpressionInfo(DataType(DataType::DTB_VOID), true, ExpressionInfo::FT_VOID); //Constructors are void functions thus returning nothing

        	}else
        	{

        		writeASM("PUSHTHIS");
        		asmout << OP_PUSH_THIS;

        		return ExpressionInfo(DataType(DataType::DTB_OBJECT, currentClass->getID()), false, ExpressionInfo::FT_LITERAL); //Although "PUSHTHIS" actually creates a reference, returning FT_VARIABLE_REF would allow the program to assign to it
        	}

        }else if(t.is(Token::TT_KEYWORD,"parent"))
        {

        	if(currentClass->getSuperclass() == nullptr)
        	{
        		error("Can't access parent in non-extending classes", t.getLine());
        	}

        	writeASM("PUSHPARENT");
        	asmout << OP_PUSH_PARENT;

        	return ExpressionInfo(DataType(DataType::DTB_OBJECT, currentClass->getSuperclass()->getID()), false, ExpressionInfo::FT_LITERAL); //Like above, we don't want the typechecker to allow assignments to parent

        }else if(t.is(Token::TT_KEYWORD,"true"))
        {
        	writeASM("PUSHINT 1");
        	asmout << OP_PUSH_INT;
        	asmout.writeInt(1);

        	return ExpressionInfo(DataType(DataType::DTB_INT), true, ExpressionInfo::FT_LITERAL);

        }else if(t.is(Token::TT_KEYWORD,"false"))
        {
        	writeASM("PUSHINT 0");
        	asmout << OP_PUSH_INT;
        	asmout.writeInt(0);

        	return ExpressionInfo(DataType(DataType::DTB_INT), true, ExpressionInfo::FT_LITERAL);

        }else if(t.getType() == Token::TT_EOF)
        {
            error("Unexpected end of file while parsing expression", t.getLine());

        }else
        {
            error("Unexpected token in expression factor: " + t.getLexem(), t.getLine());
        }

        return ExpressionInfo(DataType(DataType::DTB_VOID), false, ExpressionInfo::FT_LITERAL); //Should never happen
    }

    /*ArrayType DefaultCompiler::arrayLiteral()
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

    bool DefaultCompiler::namespaceExists(const String &s)
    {
    	return root->listClassesInNamespace(s).size() > 0;
    }

    Local *DefaultCompiler::lookupLocal(const String &pName, const Scope &pScope)
    {
    	for(uint32_t i = 0; i < locals.size(); i++)
    	{
    		if(locals[i].getName() == pName && locals[i].getScope().isVisibleIn(pScope))
    		{
    			return &locals[i];
    		}
    	}

    	return nullptr;
    }

    const Class *DefaultCompiler::lookupClass(const Scales::ClassID &classID)
    {
    	if(classID.getNamespace().empty())
    	{
    		//first, check import list
    		for(auto iter = importList.begin() ; iter != importList.end(); iter++)
    		{
    			if((*iter)->getID().getClassname() == classID.getClassname()) //return first imported class with same name as the one we are looking for
    			{
    				return *iter;
    			}
    		}


    		//no class found in import list. look in current namespace

    		const Class *c = root->getClass(ClassID(currentClass->getID().getNamespace(), classID.getClassname()));

    		if(c != nullptr)
    		{
    			return c;

    		}else
    		{
    			//not in current namespace. take id as it is (looking in default namespace). if that's null, we don't give a shit
    			return root->getClass(classID);
    		}

    	}else
    	{
    		return root->getClass(classID);
    	}
    }

    const Class *DefaultCompiler::lookupClassByName(const String &name)
    {
    	return lookupClass(ClassID("", name)); //TODO: I don't like how this is looking (also the above function). Better make tis a bit fancier
    }

    void DefaultCompiler::writeDatatypeToBytecode(const DataType &t)
    {
    	uint8_t id = t.getBase();

    	if(t.isArray())
    	{
    		id |= 8;
    	}

    	asmout.writeUByte(id);

    	if(t == DataType::DTB_OBJECT)
    	{
    		asmout.writeBString(t.getTypeClass()->getID().getNamespace());
    		asmout.writeBString(t.getTypeClass()->getID().getClassname());
    	}
    }

    DataType DefaultCompiler::getTypeOfNumberString(const String &s)
    {
    	bool fp = (StringUtils::indexOf(s, '.') != -1);

    	//TODO: Consider big types here
    	return fp ? DataType(DataType::DTB_FLOAT) : DataType(DataType::DTB_INT);
    }

    bool DefaultCompiler::isAddOp(const Token &t)
    {
    	return t.is(Token::TT_OPERATOR, "+") || t.is(Token::TT_OPERATOR, "-");
    }

    bool DefaultCompiler::isMultiplyOp(const Token &t)
    {
    	return t.is(Token::TT_OPERATOR, "*") || t.is(Token::TT_OPERATOR, "/");
    }

    bool DefaultCompiler::isLogicOp(const Token &t)
    {
    	return t.is(Token::TT_OPERATOR, "|") || t.is(Token::TT_OPERATOR, "&");
    }

    bool DefaultCompiler::isRelationalOp(const Token &t)
    {
    	return t.is(Token::TT_OPERATOR, "==") || t.is(Token::TT_OPERATOR, "!=") || t.is(Token::TT_OPERATOR, ">") || t.is(Token::TT_OPERATOR, "<") || t.is(Token::TT_OPERATOR, ">=") || t.is(Token::TT_OPERATOR, "<=");
    }

    bool DefaultCompiler::isAssignmentOperator(const Token &t)
    {
    	return t.is(Token::TT_OPERATOR,"=") || t.is(Token::TT_OPERATOR,"+=") || t.is(Token::TT_OPERATOR,"-=") || t.is(Token::TT_OPERATOR,"*=") || t.is(Token::TT_OPERATOR,"/=");
    }

    bool DefaultCompiler::isModifier(const Token &t)
    {
    	return t.is(Token::TT_KEYWORD, "public") || t.is(Token::TT_KEYWORD, "private") || t.is(Token::TT_KEYWORD, "native") || t.is(Token::TT_KEYWORD, "static");
    }

    bool DefaultCompiler::isPrimitive(const Token &t)
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

    bool DefaultCompiler::isStartOfExpression(const Token &t)
    {
    	return t.is(Token::TT_KEYWORD, "this") || t.is(Token::TT_KEYWORD, "new") || t.is(Token::TT_OPERATOR, "(") || t.is(Token::TT_KEYWORD, "parent");
    }

    bool DefaultCompiler::isAccessModifier(const Token &t)
    {
    	return t.is(Token::TT_KEYWORD, "public") || t.is(Token::TT_KEYWORD, "private"); //TODO: we might store all these keywords and stuff in predefined tokens, so we can do just: t == LE_ACCESS_PUBLIC
    }

    String DefaultCompiler::escapeASMChars(const String &s)
    {
    	//TODO: Do useful stuff here

    	return s;
    }

    uint32_t DefaultCompiler::getNewUID()
    {
    	return ++lastUID;
    }

    int32_t DefaultCompiler::parseInt(const String &s)
    {
    	int32_t v = 0;

    	for(uint32_t i = 0; i < s.length(); i++)
    	{
    		v += s[i] - '0';
    		v *= 10;
    	}

    	return v;
    }

    int64_t DefaultCompiler::parseLong(const String &s)
    {
    	int64_t v = 0;

		for(uint32_t i = 0; i < s.length(); i++)
		{
			v += s[i] - '0';
			v *= 10;
		}

		return v;
    }

    float DefaultCompiler::parseFloat(const String &s)
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

    double DefaultCompiler::parseDouble(const String &s)
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

    void DefaultCompiler::error(const String &s, int line)
    {
    	//TODO: Do more stuff with the error message than just throwing an exception

    	SCALES_EXCEPT(Exception::ET_COMPILER, String("Error in line ") + line + ": " + s);
    }
	
    const String DefaultCompiler::KEYWORDS[] =
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
		"binds",
		"extends",
		"using",
		"return",
		"do",
		"while",
		"for",
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
		"true",
		"false",

		"int",
		"long",
		"float",
		"double",
		"string",
		"object"
	};
	const uint32_t DefaultCompiler::KEYWORD_COUNT = sizeof(KEYWORDS)/sizeof(String);

	const String DefaultCompiler::DATATYPES[] =
	{
		"int",
		"long",
		"float",
		"double",
		"string",
		"object"
	};
	const uint32_t DefaultCompiler::DATATYPE_COUNT = sizeof(DATATYPES)/sizeof(String);

	const String DefaultCompiler::OPERATORS[] =
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
	const uint32_t DefaultCompiler::OPERATOR_COUNT = sizeof(OPERATORS)/sizeof(String);


	//class BlockInfo

	BlockInfo::BlockInfo(uint32_t pLocalCount, uint32_t pStackSize, blockType_t pFollowingBlock)
	: localCount(pLocalCount),
	  stackSize(pStackSize),
	  followingBlock(pFollowingBlock)
	{
	}

	uint32_t BlockInfo::getLocalCount() const
	{
		return localCount;
	}

	uint32_t BlockInfo::getStackSize() const
	{
		return stackSize;
	}

	BlockInfo::blockType_t BlockInfo::getFollowingBlock() const
	{
		return followingBlock;
	}

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



    Scope::Scope(uint32_t pNestId, uint32_t pRowId, uint32_t pUniqueId)
		: nestId(pNestId),
		  rowId(pRowId),
		  uniqueId(pUniqueId)
	{
	}

	uint32_t Scope::getNestId() const
	{
		return nestId;
	}

	uint32_t Scope::getRowId() const
	{
		return rowId;
	}

	uint32_t Scope::getUniqueId() const
	{
		return uniqueId;
	}

	bool Scope::isVisibleIn(const Scope &s) const
	{
		if(s.getNestId() == nestId)
		{
			return s.getUniqueId() == uniqueId;

		}else if(s.getNestId() > nestId)
		{
			return (s.getNestId() - nestId) == (s.getUniqueId() - uniqueId - s.getRowId());
		}

		return false;
	}

	bool Scope::isGlobal() const
	{
		return nestId == 0;
	}

	const Scope Scope::GLOBAL(0,0,0);


	Local::Local(const String &pName, const DataType &pType, const Scope &pScope, uint32_t pIndex)
	 : name(pName),
	   type(pType),
	   scope(pScope),
	   index(pIndex)
	{
	}

	String Local::getName() const
	{
		return name;
	}

	DataType Local::getType() const
	{
		return type;
	}

	Scope Local::getScope() const
	{
		return scope;
	}

	uint32_t Local::getIndex() const
	{
		return index;
	}
}
