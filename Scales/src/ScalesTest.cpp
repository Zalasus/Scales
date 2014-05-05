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
#include "comp/Exception.h"

using namespace Scales;

int main()
{

	std::ifstream in("oos3.sss", std::ios::in);

	if(in.is_open())
	{

		Compiler *c = new Compiler(in);

		try
		{
			c->compile();

		}catch(Exception &e)
		{
			std::cout << e.getMessage() << std::endl;
		}

		delete c;

	}

	in.close();


	/*ScriptSystem ss = ScriptSystem();

	ss.declareLink("Console", thingThatExtendsScriptable);

	*/

}
