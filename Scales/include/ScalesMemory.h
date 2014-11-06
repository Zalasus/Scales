/*
 * ScalesMemory.h
 *
 *  Created on: 13.10.2014
 *      Author: Niklas Weissner
 */

#ifndef SCALESMEMORY_H_
#define SCALESMEMORY_H_


namespace Scales
{

	enum memoryManagementPolicy_t
	{
		MMP_KEEP_FOREVER = 1,
		MMP_KEEP_WHEN_USED = 2,
	};

	class Managed
	{
	public:

		virtual ~Managed() = 0;

		virtual memoryManagementPolicy_t getManagementPolicy() = 0;
		virtual size_t getUserCount() = 0;
	};

	class MemPtr
	{
	public:

		//We need a smart pointer class that simplifies access of the VMs memory

		MemPtr(const MemPtr &pPtr);

		Value &operator*();
		Value *operator->();

	private:

	};

	class Value : public Managed
	{
		~Value();

		memoryManagementPolicy_t getManagementPolicy();
		size_t getUserCount();
	};

	typedef std::vector<Value> ValueList;
}


#endif /* SCALESMEMORY_H_ */
