/*
 * ScalesReflection.h
 *
 *  Created on: 08.10.2014
 *      Author: Zalasus
 */

#ifndef SCALESREFLECTION_H_
#define SCALESREFLECTION_H_


namespace Scales
{

	class Reflectable
	{

	};

}


#define SCALES_REGISTER_REFLECTABLE_BEGIN(classname, reflectionTargetName)

#define SCALES_REGISTER_REFLECTABLE_END(classname, reflectionTargetName)

#define SCALES_REGISTER_FUNCTION_1P(functionName, returnType, paramType)
#define SCALES_REGISTER_FUNCTION_1P_STATIC(functionName, returnType, paramType)

#define SCALES_REGISTER_FUNCTION_2P(functionName, returnType, paramType1, paramType2)
#define SCALES_REGISTER_FUNCTION_2P_STATIC(functionName, returnType, paramType1, paramType2)

#define SCALES_REGISTER_FIELD(fieldName, type)
#define SCALES_REGISTER_FIELD_STATIC(fieldName, type)

#endif /* SCALESREFLECTION_H_ */
