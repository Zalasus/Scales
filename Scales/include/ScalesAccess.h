/*
 * ScalesAccess.h
 *
 *  Created on: 08.10.2014
 *      Author: Zalasus
 */

#ifndef SCALESACCESS_H_
#define SCALESACCESS_H_


namespace Scales
{

	class AccessElement
	{
	public:

		void setPublic(bool b);
		bool isPublic() const;

		void setStatic(bool b);
		bool isStatic() const;

		void setNative(bool b);
		bool isNative() const;

	private:

		bool vPublic;
		bool vStatic;
		bool vNative;
	};

}


#endif /* SCALESACCESS_H_ */
