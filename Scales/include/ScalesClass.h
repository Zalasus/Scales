/*
 * ScalesClass.h
 *
 *  Created on: 03.10.2014
 *      Author: Zalasus
 */

#ifndef SCALESCLASS_H_
#define SCALESCLASS_H_

#include <stdint.h>
#include <vector>

#include "ScalesString.h"
#include "ScalesFunction.h"
#include "ScalesField.h"
#include "ScalesType.h"

namespace Scales
{

	class Class
	{
	public:

		Class(const ClassID &classID, const Class *pSuper);
		~Class();

		ClassID getID() const;
		const Class *getSuperclass() const;

		//Create&Get -> The create function takes only explicitly needed attributes. It returns
		//a pointer to a modifiable object in which all other attributes can be set.
		//All get functions should be const and should return const data if they return pointers, so a Class can be "finalized" by storing it as const
		Function *createFunction(const String &functionName, const TypeList &paramTypes);
		void removeFunction(const Function *func);
		const Function *getFunction(const String &functionName, const TypeList &paramTypes) const;
		const Function *getJoinedFunction(const String &functionName, const TypeList &paramTypes) const;

		Field *createField(const String &fieldName, const DataType &type); //Note: The supeclass is known at instantiation, so this method calculates the proper field index right at calling and gives it to the field constructor TODO: put this in a doxy
		const Field *getField(const String &fieldName) const;
		/**
		 * \return A pointer to a Field object of this class or it's superclass.
		 */
		const Field *getJoinedField(const String &fieldName) const;

		uint32_t getFieldCount() const;
		uint32_t getJoinedFieldCount() const;

		void setNativeTarget(const String &target);
		String getNativeTarget() const;

		progAdress_t getProgramSize() const;

		progAdress_t getJoinedProgramSize() const;
		const progUnit_t *getJoinedProgramArray() const;

		/**
		 * Copies the given array of program data. Deallocation of the given
		 * data is not handled by this function. The user has to deallocate
		 * it manually if needed. The copied data however is handled by the
		 * Class.
		 */
		void copyProgramArray(const progUnit_t *data, progAdress_t size);


	private:

		ClassID classID;
		const Class *super;

		std::vector<Function*> functions;
		std::vector<Field*> fields;

		String nativeTarget;

		progUnit_t *program;
		progAdress_t programSize;
	};

}


#endif /* SCALESCLASS_H_ */
