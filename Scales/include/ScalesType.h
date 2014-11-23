/*
 * ScalesType.h
 *
 *  Created on: 04.10.2014
 *      Author: Zalasus
 */

#ifndef SCALESTYPE_H_
#define SCALESTYPE_H_

#include <vector>

#include "ScalesString.h"

namespace Scales
{

	class Class;

	class DataType
	{
	public:

		enum dataTypeBase_t
		{
			DTB_INT = 0,
			DTB_LONG = 1,
			DTB_FLOAT = 2,
			DTB_DOUBLE = 3,
			DTB_STRING = 4,
			DTB_ABSTRACT_OBJECT = 5,
			DTB_OBJECT = 6,
			DTB_VOID = 255
		};

		DataType(dataTypeBase_t pBase, const Class *pTypeClass = nullptr);

		dataTypeBase_t getBase() const;
		const Class *getTypeClass() const;

		bool isNumeric() const;
		bool isArray() const;

		String toString() const;

		bool operator==(const DataType &t) const;
		bool operator==(const dataTypeBase_t tb) const;

		static bool canCastImplicitly(const DataType &from, const DataType &to);
		static DataType mathCast(const DataType &left, const DataType &right);
		static dataTypeBase_t byName(const String &name);

		static const DataType INT;
		static const DataType LONG;
		static const DataType FLOAT;
		static const DataType DOUBLE;
		static const DataType STRING;
		static const DataType _VOID;

	private:

		dataTypeBase_t base;
		const Class *typeClass;
	};

	typedef std::vector<DataType> TypeList;

}


#endif /* SCALESTYPE_H_ */
