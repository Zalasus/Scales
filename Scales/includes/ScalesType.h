/*
 * ScalesType.h
 *
 *  Created on: 08.07.2014
 *      Author: Zalasus
 */

namespace Scales
{
	class DataType;
}

#ifndef SCALESTYPE_H_
#define SCALESTYPE_H_

#include "ScalesUtil.h"
#include "ScalesClass.h"

namespace Scales
{

	class DataType
	{
	public:

		enum DataTypeBase
		{
			DTB_INT = 0,
			DTB_LONG = 1,
			DTB_FLOAT = 2,
			DTB_DOUBLE = 3,
			DTB_STRING = 4,
			DTB_OBJECT = 5,
			DTB_ABSTRACT_OBJECT = 6,
			DTB_NOTYPE = 255
		};

		DataType(DataTypeBase pBase, const ClassId &classId = ClassId::EMPTY);

		DataTypeBase getTypeBase() const;
		ClassId getClassId() const;

		bool isNumeric() const;
		bool isArray() const;

		String toString() const;

		bool operator==(const DataType &t) const;
		bool operator==(const DataTypeBase tb) const;

		static bool canCastImplicitly(const DataType &from, const DataType &to);
		static DataType mathCast(const DataType &left, const DataType &right);
		static DataType byName(const String &name);

	private:

		DataTypeBase base;
		ClassId classId;
	};


	typedef std::vector<DataType> TypeList;

}


#endif /* SCALESTYPE_H_ */
