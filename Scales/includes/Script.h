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
		const String &getName() const;
		const DataType &getReturnType() const;
		const AccessType &getAccessType() const;

		bool is(const String &name, const vector<DataType> &paramTypes) const;

		uint32_t getAdress() const;

		static const String createInfoString(const String &name, vector<DataType> &paramTypes);

	private:

		String functionName;
		vector<DataType> paramTypes;
		DataType returnType;
		AccessType accessType;

		bool native;
		bool event;

		uint32_t adress;
	};


	class Script
	{
	public:
		Script(const ScriptIdent &scriptident);

		void declareFunction(const Function &func);
		Function *getFunction(const String &name, const vector<DataType> &paramTypes);

		void declareLocal(Variable &v);
		void leaveLocalScope();
		void enterLocalScope();
		void destroyAllLocals();

		void declareGlobal(Variable &v);

		Variable *getLocal(const String &name);
		Variable *getGlobal(const String &name);

		const ScriptIdent &getIdent() const;

	private:

		const ScriptIdent ident;

		uint32_t currentLocalScope;

		vector<Function> functions;

		vector<Variable> globals;
		vector<Variable> locals;
	};


	class ScriptInstance
	{
	public:

		ScriptInstance(const Script &s);

	private:

		const Script &myClass;

		uint32_t programCounter;

		vector<Value> astack;
		vector<uint32_t> callstack;

	};

}

#endif /* SCRIPT_H_ */
