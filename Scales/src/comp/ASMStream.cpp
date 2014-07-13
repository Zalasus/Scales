/*
 * ASMStream.cpp
 *
 *  Created on: 14.05.2014
 *      Author: Niklas Weissner
 */

#include "comp/ASMStream.h"


namespace Scales
{

	ByteArrayOutputStream::ByteArrayOutputStream()
	{
		bufferSize = 32;
		count = 0;

		buffer = new uint8_t[bufferSize];
	}

	ByteArrayOutputStream::~ByteArrayOutputStream()
	{
		delete[] buffer;
	}

	void ByteArrayOutputStream::write(uint8_t c)
	{
		uint32_t newcount = count + 1;

		if(newcount > bufferSize)
		{
			uint32_t shiftedLen = bufferSize << 1;
			uint32_t newLen = (shiftedLen >= newcount) ? shiftedLen : newcount;

			uint8_t *newBuffer = new uint8_t[newLen];

			for(uint32_t i = 0; i < bufferSize; i++)
			{
				newBuffer[i] = buffer[i];
			}

			delete[] buffer;
			buffer = newBuffer;
			bufferSize = newLen;
		}

		buffer[count] = c;

		count = newcount;
	}

	uint32_t ByteArrayOutputStream::size()
	{
		return count;
	}

	void ByteArrayOutputStream::reset()
	{
		count = 0;
	}

	/**
	 * Copies the buffer to a new memory block including only the cells that were written to.
	 * The stream does not take care of cleaning up the ressources created by this method.
	 */
	uint8_t *ByteArrayOutputStream::toNewArray()
	{
		uint8_t *newBuffer = new uint8_t[count];

		for(uint32_t i = 0; i < count; i++)
		{
			newBuffer[i] = buffer[i];
		}

		return newBuffer;
	}

	/**
	 * Copies the buffer to a new memory block including all allocated cells.
	 * The stream does not take care of cleaning up the ressources created by this method.
	 */
	uint8_t *ByteArrayOutputStream::newBufferCopy()
	{
		uint8_t *newBuffer = new uint8_t[bufferSize];

		for(uint32_t i = 0; i < bufferSize; i++)
		{
			newBuffer[i] = buffer[i];
		}

		return newBuffer;
	}

	uint8_t *ByteArrayOutputStream::getBuffer()
	{
		return buffer;
	}


	//---------------------------------------------


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
			stream.write(s[i]);

			if(i == 0xFFFF)
			{
				break;
			}
		}
	}

	void ASMStream::writeTString(const String &s)
	{
		write(s.length());

		for(int32_t i = 0; i < s.length(); i++)
		{
			stream.write(s[i]);

			if(i == 0xFF)
			{
				break;
			}
		}
	}

	void ASMStream::writeString(const String &s)
	{
		writeUInt(s.length());

		for(int32_t i = 0; i < s.length(); i++)
		{
			stream.write(s[i]);
		}
	}

	void ASMStream::defineMarker(const String &name, uint32_t adress)
	{
		definedMarkers[name] = adress;

		//Check if this marker was already requested. If yes -> write it to its destination
		for(auto it = requestedMarkers.begin(); it != requestedMarkers.end();)
		{
			if(it->second == name)
			{
				uint32_t adr = it->first;

				if(adr < stream.size()-3)
				{
					stream.getBuffer()[adr] = (adress & 0xFF);
					stream.getBuffer()[adr+1] = ((adress & 0xFF00) >> 8);
					stream.getBuffer()[adr+2] = ((adress & 0xFF0000) >> 16);
					stream.getBuffer()[adr+3] = ((adress & 0xFF000000) >> 24);

				}//else fatal error

				it = requestedMarkers.erase(it);
			}else
			{
				it++;
			}
		}
	}

	void ASMStream::defineMarker(const String &name)
	{
		defineMarker(name, stream.size());
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

	void ASMStream::reset()
	{
		stream.reset();

		definedMarkers.clear();
		requestedMarkers.clear();
	}

	bool ASMStream::hasUndefinedMarkers()
	{
		return (requestedMarkers.size() > 0);
	}

	uint8_t *ASMStream::getBuffer()
	{
		return stream.getBuffer();
	}

	uint32_t ASMStream::getSize()
	{
		return stream.size();
	}

	ByteArrayOutputStream &ASMStream::getStream()
	{
		return stream;
	}

	ASMStream &ASMStream::operator<<(Opcode op)
	{
		write(op);

		return *this;
	}

	ASMStream &ASMStream::operator<<(const String &s)
	{
		writeString(s);

		return *this;
	}

	ASMStream &ASMStream::operator<<(uint32_t i)
	{
		writeUInt(i);

		return *this;
	}
}
