/*
 * ScalesTest.cpp
 *
 *  Created on: 30.03.2014
 *      Author: Niklas Weissner
 */

#include <iostream>
#include <fstream>

#include "comp/Lexer.h"

using namespace Scales;

int main()
{

	std::ifstream in("C:/test.sss", std::ios::in);

	if(in.is_open())
	{
		Lexer *l = new Lexer(in);

		l->declareKeyword(String("public"));
		l->declareKeyword(String("native"));
		l->declareKeyword(String("func"));
		l->declareKeyword(String("int"));
		l->declareKeyword(String("end"));
		l->declareKeyword(String("if"));
		l->declareOperator(String("("));
		l->declareOperator(String(")"));
		l->declareOperator(String("="));
		l->declareOperator(String("=="));
		l->declareOperator(String("+"));
		l->declareOperator(String(";"));
		l->declareOperator(String("."));

		Token t = l->readToken();
		while(t.getType() != TT_EOF)
		{
			std::cout << t.getType() << "\t" << t.getLexem() << std::endl;

			t = l->readToken();
		}

		delete l;

	}

	in.close();


}
