/*
 * ASMStream.h
 *
 *  Created on: 14.05.2014
 *      Author: Zalasus
 */

#ifndef ASMSTREAM_H_
#define ASMSTREAM_H_

#include "ScalesUtil.h"
#include "ScalesBytecode.h"
#include "ScalesString.h"

#include <unordered_map>

namespace Scales
{

	class ByteArrayOutputStream
	{
	public:
		ByteArrayOutputStream();
		~ByteArrayOutputStream();

		void write(uint8_t c);

		uint32_t size();

		void reset();

		uint8_t *toNewArray();
		uint8_t *newBufferCopy();
		uint8_t *getBuffer();

	private:
		uint8_t *buffer;
		uint32_t bufferSize;
		uint32_t count;

	};

	//TODO: This whole bytecode writer is a complete mess with no byte order checking and stuff, and is only half implemented. Improve!

	class ASMStream
	{
	public:

		void writeUByte(uint8_t i);
		void writeUInt(uint32_t i);

		void writeIString(const String &s);
		void writeBString(const String &s);

		void defineMarker(const String &name, uint32_t adress);
		void defineMarker(const String &name);

		void writeMarker(const String &name);

		void reset();

		bool hasUndefinedMarkers();

		uint8_t *getBuffer();
		uint32_t getSize();
		ByteArrayOutputStream &getStream();

		ASMStream &operator<<(opcode_t op);
		ASMStream &operator<<(const String &s);
		ASMStream &operator<<(uint32_t i);

	private:

		ByteArrayOutputStream stream;

		std::unordered_map<String, uint32_t> definedMarkers;
		std::unordered_map<uint32_t, String> requestedMarkers;

	};
}



#endif /* ASMSTREAM_H_ */
