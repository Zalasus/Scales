/*
 * ScalesTest.cpp
 *
 *  Created on: 30.03.2014
 *      Author: Niklas Weissner
 */

#include <iostream>
#include <fstream>

#include "comp/Lexer.h"
#include "comp/Compiler.h"
#include "DataType.h"
#include "Exception.h"

using namespace Scales;

int main()
{

	std::ifstream in("oos3.sss", std::ios::in);

	if(in.is_open())
	{
		ScriptSystem ss;

		Compiler *c = new Compiler(in, ss);

		try
		{
			c->compile();

		}catch(ScalesException &e)
		{
			std::cout << e.getMessage() << std::endl;
		}

		delete c;

		ScriptIdent ident = ScriptIdent("Main");

		Script *scr = ss.getScript(ident);

		if(scr == null)
		{
			std::cout << "The script " << ident.toString() << " was not declared in the script system" << std::endl;

		}else
		{
			std::cout << "The script " << ident.toString() << " contains " << scr->getBytecodeLength() << " bytes of bytecode." << std::endl;

		}

	}

	in.close();


	/*ScriptSystem ss = ScriptSystem();

	ss.declareLink("Console", thingThatExtendsScriptable);

	*/

}
