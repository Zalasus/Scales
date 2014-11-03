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

	class ClassID
	{
	public:

		ClassID(const String &pNspace, const String &pClassname);

		String getNamespace() const;
		String getClassname() const;

		bool operator==(const ClassID &right);

		String toString() const;

		static const ClassID EMPTY;

	private:

		String nspace;
		String classname;
	};

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
			DTB_OBJECT = 5,
			DTB_ABSTRACT_OBJECT = 6,
			DTB_VOID = 255
		};

		DataType(dataTypeBase_t pBase);
		DataType(dataTypeBase_t pBase, const ClassID &pClassID);

		dataTypeBase_t getBase() const;
		ClassID getClassID() const;

		bool isNumeric() const;
		bool isArray() const;

		String toString() const;

		bool operator==(const DataType &t) const;
		bool operator==(const dataTypeBase_t tb) const;

		static bool canCastImplicitly(const DataType &from, const DataType &to);
		static DataType mathCast(const DataType &left, const DataType &right);
		static dataTypeBase_t byName(const String &name);

	private:

		dataTypeBase_t base;
		ClassID classID;
	};

	typedef std::vector<DataType> TypeList;

}


#endif /* SCALESTYPE_H_ */
