/*
 * ScalesTest.cpp
 *
 *  Created on: 30.03.2014
 *      Author: Niklas Weissner
 */

#include <iostream>
#include <fstream>

#include <time.h>

#include "comp/Lexer.h"
#include "comp/Compiler.h"
#include "DataType.h"
#include "Exception.h"

using namespace Scales;

int main()
{

	std::cout << "Compiling starts now" << std::endl;

	clock_t t = clock();

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
			std::cerr << e.getMessage() << std::endl;
		}

		delete c;

		ScriptIdent ident = ScriptIdent("Main");

		Script *scr = ss.getScript(ident);

		if(scr == null)
		{
			std::cerr << "The script " << ident.toString() << " was not declared in the script system" << std::endl;

		}else
		{
			std::cout << "The script " << ident.toString() << " contains " << scr->getBytecodeLength() << " bytes of bytecode." << std::endl;

		}

	}

	in.close();

	t = clock() - t;

	std::cout << "Finished. That took me about " << t << " ticks, which is roughly " << ((float)t)/CLOCKS_PER_SEC << " seconds" << std::endl;

	/*ScriptSystem ss = ScriptSystem();

	ss.declareLink("Console", thingThatExtendsScriptable);

	*/

}
