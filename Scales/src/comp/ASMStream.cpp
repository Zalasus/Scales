/*
 * ASMStream.cpp
 *
 *  Created on: 14.05.2014
 *      Author: Niklas Weissner
 */

#include "comp/ASMStream.h"


namespace Scales
{

	void ASMStream::write(uint8_t b)
	{
		stream.write(b);
	}

	void ASMStream::writeUInt(uint32_t i)
	{
		stream.write(i & 0xFF);
		stream.write((i & 0xFF00) >> 8);
		stream.write((i & 0xFF0000) >> 16);
		stream.write((i & 0xFF000000) >> 24);
	}

	void ASMStream::writeUShort(uint16_t s)
	{
		stream.write(s & 0xFF);
		stream.write((s & 0xFF00) >> 8);
	}

	void ASMStream::writeSString(const String &s)
	{
		writeUShort(s.length());

		for(int32_t i = 0; i < s.length(); i++)
		{
			stream.write(s.charAt(i));

			if(i == 0xFFFF)
			{
				break;
			}
		}
	}

	void ASMStream::defineMarker(const String &name, uint32_t adress)
	{
		definedMarkers[name] = adress;

		//Check if this marker was already requested. If yes -> write it to its destination
		for(auto element = requestedMarkers.begin(); element != requestedMarkers.end(); ++element)
		{
			if(element->second.equals(name))
			{
				uint32_t adr = element->first;

				if(adr < stream.size()-3)
				{
					stream.getBuffer()[adr] = (adress & 0xFF);
					stream.getBuffer()[adr+1] = ((adress & 0xFF00) >> 8);
					stream.getBuffer()[adr+2] = ((adress & 0xFF0000) >> 16);
					stream.getBuffer()[adr+3] = ((adress & 0xFF000000) >> 24);

				}//else fatal error

				//erase element from requested-marker-list
				requestedMarkers.erase(adr);
			}
		}

	}

	void ASMStream::defineMarker(const String &name)
	{
		definedMarkers[name] = stream.size();
	}

	void ASMStream::writeMarker(const String &name)
	{
		if(definedMarkers.count(name) == 0)
		{
			//Marker not yet defined. Mark this position in the stream as demanding a marker

			requestedMarkers[stream.size()] = name;

			writeUInt(0xCAFEBABE); //Just write stupid stuff there; it is goint to be replaced anyway

		}else
		{
			writeUInt(definedMarkers[name]);
		}

	}

	uint8_t *ASMStream::getBytecode()
	{
		if(requestedMarkers.size() > 0)
		{
			//TODO: Error! Undefined markers were used
		}

		return stream.getBuffer();
	}

	uint32_t ASMStream::getSize()
	{
		return stream.size();
	}


}
