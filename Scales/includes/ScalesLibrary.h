/*
 * ScalesLibrary.h
 *
 *  Created on: 19.07.2014
 *      Author: Zalasus
 */

namespace Scales
{
	class Library;
}

#ifndef SCALESLIBRARY_H_
#define SCALESLIBRARY_H_

#include "ScalesUtil.h"
#include "ScalesClass.h"

#include <vector>

namespace Scales
{

	class Library
	{
	public:

		void add(const ClassPrototype &proto);

		uint32_t getSize() const;
		const std::vector<ClassPrototype> getAll() const;

		bool storeToFile(String filename) const;
		uint32_t loadFromFile(String filename);

	private:

		std::vector<ClassPrototype> prototypes;

	};

}


#endif /* SCALESLIBRARY_H_ */
