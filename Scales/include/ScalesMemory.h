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

	class Value : public Managed
	{



	};
}


#endif /* SCALESMEMORY_H_ */
