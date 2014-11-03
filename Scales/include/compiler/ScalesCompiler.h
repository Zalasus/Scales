/*
 * ScalesCompiler.h
 *
 *  Created on: 03.10.2014
 *      Author: Zalasus
 */

#ifndef SCALESCOMPILER_H_
#define SCALESCOMPILER_H_

#include <cstdlib>
#include <istream>
#include <vector>

namespace Scales
{

	enum compilerFlags_t
	{
		CF_GENERIC = 1
	};

	class Class;

	class Compiler
	{
	public:
		virtual ~Compiler();

		/**
		 * Reads Scales source code from stream and compiles it into class data.
		 * It then creates the generated classes as children of the Root the Compiler
		 * object was created from.
		 *
		 * \param in Reference to a input stream from which to read the source data.
		 * \return The count of newly generated classes
		 */
		virtual size_t compile(std::istream *in) = 0;
		virtual std::vector<const Class*> listGeneratedClasses() = 0;


	};

}


#endif /* SCALESCOMPILER_H_ */
