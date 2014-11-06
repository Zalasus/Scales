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

	class IValue //interface for polymorphic memory elements
	{
		virtual ~IValue();
	};

	class Managed {};

	class Value{};

	class MemoryElement //pointer compound used
	{
	public:

		enum storageType_t
		{
			ST_MAPPED,
			ST_STORED
		};

	private:

		DataType type;
		storageType_t storType;
		IValue *value;

	};
}


#endif /* SCALESMEMORY_H_ */
