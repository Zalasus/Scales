/*
 * ScalesMemory.h
 *
 *  Created on: 10.07.2014
 *      Author: Niklas Weissner
 */

#ifndef SCALESMEMORY_H_
#define SCALESMEMORY_H_


namespace Scales
{

	class MemoryManager
	{
	public:

		virtual void takeover(Variable *var);
	};

}


#endif /* SCALESMEMORY_H_ */
