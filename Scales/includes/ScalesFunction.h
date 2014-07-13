/*
 * ScalesFunction.h
 *
 *  Created on: 08.07.2014
 *      Author: Niklas Weissner
 */

#ifndef SCALESFUNCTION_H_
#define SCALESFUNCTION_H_


namespace Scales
{

	class FunctionPrototype
	{
	public:
		FunctionPrototype(const String &pName, const TypeList &pParamTypes, const DataType &pReturnType, bool pNative);
		FunctionPrototype(const String &pName, const TypeList &pParamTypes, const DataType &pReturnType, bool pNative, uint32_t pAdress);

		String getName() const;
		const TypeList getParameterTypes() const;
		DataType getReturnType() const;
		bool isNative() const;
		uint32_t getAdress() const;

		bool hasAdress() const;
		void setAdress(uint32_t adr);

	private:

		String name;
		TypeList paramTypes;
		DataType returnType;
		bool native;

		bool hasAdr;
		uint32_t adress;
	};

}


#endif /* SCALESFUNCTION_H_ */
