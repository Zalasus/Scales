/*
 * ScalesValue.h
 *
 *  Created on: 10.07.2014
 *      Author: Zalasus
 */

namespace Scales
{
	class Value;
}

#ifndef SCALESVALUE_H_
#define SCALESVALUE_H_


namespace Scales
{

	class Value
	{
	public:

		enum ValueType
		{
			VT_OBJECT
		};

		virtual ~Value();

		virtual ValueType getValueType() const = 0;

	};


	class ValuePtr
	{
	public:
		ValuePtr(Value *ref);
		~ValuePtr();

		Value *getValue();
		uint32_t getUserCount();

		void addUser(Variable *v);
		void removeUser(Variable *v);

	private:
		Value *value;
		uint32_t users;
	};

}



#endif /* SCALESVALUE_H_ */
