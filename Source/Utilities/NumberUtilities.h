#ifndef _NUMBER_UTILITIES_H_
#define _NUMBER_UTILITIES_H_

#include <cstdint>

namespace Utilities {

	uint32_t byteLength(int8_t n);
	uint32_t unsignedByteLength(uint8_t n);
	uint32_t shortLength(int16_t n);
	uint32_t unsignedShortLength(uint16_t n);
	uint32_t intLength(int32_t n);
	uint32_t unsignedIntLength(uint32_t n);
	uint32_t longLength(int64_t n);
	uint32_t unsignedLongLength(uint64_t n);

}

#endif // _NUMBER_UTILITIES_H_
