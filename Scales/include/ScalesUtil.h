/*
 * ScalesUtil.h
 *
 *  Created on: 03.10.2014
 *      Author: Niklas Weissner
 */

#ifndef SCALESUTIL_H_
#define SCALESUTIL_H_


#ifdef _DEBUG

	//TODO: Redefine these macros so they do something useful in debug config
	#define SCALES_NEW new
	#define SCALES_DELETE delete

#else

	#define SCALES_NEW new
	#define SCALES_DELETE delete

#endif

#endif /* SCALESUTIL_H_ */
