/*
 * ScalesValue.h
 *
 *  Created on: 10.07.2014
 *      Author: Niklas Weissner
 */

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

}



#endif /* SCALESVALUE_H_ */
