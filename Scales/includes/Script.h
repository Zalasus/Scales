/*
 * Script.h
 *
 *  Created on: 29.04.2014
 *      Author: Niklas Weissner
 */

#ifndef SCRIPT_H_
#define SCRIPT_H_

#include <vector>
#include <stdint.h>

#include "Nein.h"
#include "DataType.h"
#include "Variable.h"

using std::vector;

namespace Scales
{


	class Function
	{
	public:

		Function(const String &name, const vector<DataType> paramTypes, DataType returnType, AccessType accessType, bool native, bool event, uint32_t adress);

		bool isEvent() const;
		bool isNative() const;
		String getName() const;
		DataType getReturnType() const;

		bool is(const String &name, const vector<DataType> &paramTypes) const;

		uint32_t getAdress() const;

		static String createInfoString(const String &name, vector<DataType> &paramTypes);

	private:

		String functionName;
		vector<DataType> paramTypes;
		DataType returnType;
		AccessType accessType;

		vector<vector<Variable>> localsStack;

		bool native;
		bool event;

		uint32_t adress;
	};


	class Script
	{
	public:
		Script(const String &pName, bool pStatic);

		void declareFunction(const Function &func);
		Function *getFunction(const String &name, const vector<DataType> &paramTypes);

		void declareGlobal(Variable &v);

		Variable *getVariable(const String &name);

		Variable *getLocal(const String &name);
		Variable *getGlobal(const String &name);

		bool isStatic() const;
		String getName() const;

	private:

		String scriptname;
		bool staticScript;

		vector<Function> functions;

		vector<Variable> globals;
	};


}

#endif /* SCRIPT_H_ */
