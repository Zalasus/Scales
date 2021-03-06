/*
 * ScalesTest.cpp
 *
 *  Created on: 08.10.2014
 *      Author: Zalasus
 */

#include <fstream>
#include <iostream>
#include <vector>

#include "ScalesClass.h"
#include "ScalesRoot.h"
#include "ScalesType.h"
#include "ScalesUtil.h"
#include "ScalesString.h"
#include "ScalesReflection.h"
#include "ScalesException.h"
#include "ScalesObject.h"
#include "ScalesRunner.h"

class Scales_Native_Console : public Scales::Reflectable
{
public:

	static void println(const Scales::String &s);

	static Scales::String NEW_LINE;

};

SCALES_REGISTER_REFLECTABLE_BEGIN(Scales_Native_Console, nrpg_ni_console)

	SCALES_REGISTER_FUNCTION_1P_STATIC(println, Scales::DataType::DTB_VOID, Scales::DataType:DTB_STRING)

	SCALES_REGISTER_FIELD_STATIC(newLine, Scales::DataType::DTB_STRING)

SCALES_REGISTER_REFLECTABLE_END(Scales_Native_Console, nrpg_ni_console)


int main()
{
	Scales::Root root;

	//---Example of how to create classes from a script file
	Scales::Compiler *comp = root.getCompiler(Scales::CF_GENERIC);

	const Scales::Class *mainClass = nullptr;
	const Scales::Function *mainFunc = nullptr;
	if(comp != nullptr)
	{
		//There is a compiler we can use

		std::ifstream in("oos3.sss");
		try
		{
			comp->compile(&in);

			const std::vector<const Scales::Class*> classList = comp->listGeneratedClasses();

			std::cout << std::endl << "Compiler generated " << classList.size() << " classes: " << std::endl;
			for(auto iter = classList.begin(); iter != classList.end(); iter++)
			{
				const Scales::Class *cl = *iter;

				std::cout << cl->getID().toString() << "\t" << cl->getFieldCount() << "F\t" << cl->getJoinedFieldCount() << "JF\t" << cl->getProgramSize() << "P";

				if(cl->getFunction("main",Scales::TypeList()) != nullptr)
				{
					std::cout << "\tMAIN";

					mainClass = cl;
					mainFunc = cl->getFunction("main",Scales::TypeList());
				}

				std::cout << std::endl;
			}

		}catch(Scales::Exception &e)
		{
			std::cerr << e.getMessage() << std::endl;

			return -1;
		}
	}

	if(mainClass == nullptr)
	{
		std::cerr << "Error: No main class found; nothing to run." << std::endl;
	}

	Scales::Object *o = root.createObject(mainClass); //TODO: check where to use smartpointers and where not

	if(o == nullptr)
	{
		std::cerr << "Could not instantiate class" << std::endl;
	}


	try
	{
		Scales::Runner r(o, &root, mainFunc);
		r.run();

	}catch(Scales::Exception &e)
	{
		std::cerr << e.getMessage() << std::endl;
	}

	return 0;
}






