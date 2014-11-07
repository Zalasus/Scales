/*
 * ScalesUtil.cpp
 *
 *  Created on: 07.11.2014
 *      Author: Zalasus
 */

#include "ScalesUtil.h"
#include "ScalesException.h"

namespace Scales
{

	template <typename T>
	CheckedPtr<T>::CheckedPtr(T *pPtr)
	: ptr(pPtr)
	{
	}

	template <typename T>
	T *CheckedPtr<T>::operator->()
	{
		if(ptr == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Illegal access of null pointer");
		}

		return ptr;
	}

	template <typename T>
	T &CheckedPtr<T>::operator*()
	{
		if(ptr == nullptr)
		{
			SCALES_EXCEPT(Exception::ET_RUNTIME, "Illegal access of null pointer");
		}

		return *ptr;
	}

	template <typename T>
	bool CheckedPtr<T>::operator==(T *pPtr)
	{
		return ptr == pPtr;
	}

	template <typename T>
	T *CheckedPtr<T>::getPtr()
	{
		return ptr;
	}


}



