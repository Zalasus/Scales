/*
 * ASMStream.h
 *
 *  Created on: 14.05.2014
 *      Author: Niklas Weissner
 */

#ifndef ASMSTREAM_H_
#define ASMSTREAM_H_

#include "Nein.h"
#include "Bytecode.h"

#include <unordered_map>

using std::unordered_map;

namespace Scales
{

	class ASMStream
	{
	public:

		void write(uint8_t b);
		void writeUShort(uint16_t s);
		void writeUInt(uint32_t i);
		void writeString(const String &s);
		void writeSString(const String &s);
		void writeTString(const String &s);

		void defineMarker(const String &name, uint32_t adress);
		void defineMarker(const String &name);

		void writeMarker(const String &name);

		void reset();

		bool hasUndefinedMarkers();

		uint8_t *getBytecode();
		uint32_t getSize();
		ByteArrayOutputStream &getStream();

		ASMStream &operator<<(Opcode op);
		ASMStream &operator<<(const String &s);
		ASMStream &operator<<(uint32_t i);

	private:

		ByteArrayOutputStream stream;

		unordered_map<String, uint32_t> definedMarkers;
		unordered_map<uint32_t, String> requestedMarkers;

	};
}



#endif /* ASMSTREAM_H_ */
