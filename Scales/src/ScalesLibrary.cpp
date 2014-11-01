/*
 * ScalesLibrary.cpp
 *
 *  Created on: 19.07.2014
 *      Author: Zalasus
 */

#include "ScalesLibrary.h"

namespace Scales
{

	void Library::add(const ClassPrototype &proto)
	{
		prototypes.push_back(proto);
	}

	uint32_t Library::getSize() const
	{
		return prototypes.size();
	}

	const std::vector<ClassPrototype> Library::getAll() const
	{
		return prototypes;
	}

	bool Library::storeToFile(String filename) const
	{
		//TODO: Implement library saving

		return false;
	}

	uint32_t Library::loadFromFile(String filename)
	{
		//TODO: Implement library loading

		return 0;
	}


}

