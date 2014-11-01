/*
 * ScalesTest.cpp
 *
 *  Created on: 30.03.2014
 *      Author: Zalasus
 */

#include <iostream>
#include <fstream>

#include <time.h>

#include "ScalesSystem.h"

#include "comp/Lexer.h"
#include "comp/Compiler.h"
#include "ScalesException.h"

void doStuff()
{
	Scales::ScalesSystem ss;

	std::istream in;

	Scales::Compiler compiler;
	Scales::Library l = compiler.compile(&in, &ss);

	ss.loadLibrary(l);



}

int main()
{

	std::cout << "Compiling starts now" << std::endl;

	clock_t t = clock();

	std::ifstream in("oos3.sss", std::ios::in);

	if(in.is_open())
	{
		Scales::ScalesSystem ss;

		Scales::Compiler c;

		try
		{
			c.compile(&in, &ss);

		}catch(Scales::ScalesException &e)
		{
			std::cerr << e.getMessage() << std::endl;
		}

	}

	in.close();

	t = clock() - t;

	std::cout << "Finished. That took me about " << t << " ticks, which is roughly " << ((float)t)/CLOCKS_PER_SEC << " seconds" << std::endl;

	/*ScriptSystem ss = ScriptSystem();

	ss.declareLink("Console", thingThatExtendsScriptable);

	*/

}
