#ifndef _NUMBER_UTILITIES_H_
#define _NUMBER_UTILITIES_H_

#include "Endianness.h"

#include <cstdint>
#include <optional>
#include <string>

namespace Utilities {

	uint32_t byteLength(int8_t n);
	uint32_t unsignedByteLength(uint8_t n);
	uint32_t shortLength(int16_t n);
	uint32_t unsignedShortLength(uint16_t n);
	uint32_t intLength(int32_t n);
	uint32_t unsignedIntLength(uint32_t n);
	uint32_t longLength(int64_t n);
	uint32_t unsignedLongLength(uint64_t n);
	std::string toBinary(uint8_t value);
	std::string toBinary(uint16_t value, Endianness endianness = Endianness::BigEndian);
	std::string toBinary(uint32_t value, Endianness endianness = Endianness::BigEndian);
	std::string toBinary(uint64_t value, Endianness endianness = Endianness::BigEndian);
	std::string toOctal(uint8_t value);
	std::string toOctal(uint16_t value, Endianness endianness = Endianness::BigEndian);
	std::string toOctal(uint32_t value, Endianness endianness = Endianness::BigEndian);
	std::string toOctal(uint64_t value, Endianness endianness = Endianness::BigEndian);
	std::string toHexadecimal(uint8_t value);
	std::string toHexadecimal(uint16_t value, Endianness endianness = Endianness::BigEndian);
	std::string toHexadecimal(uint32_t value, Endianness endianness = Endianness::BigEndian);
	std::string toHexadecimal(uint64_t value, Endianness endianness = Endianness::BigEndian);
	uint8_t byteFromBinary(const std::string & data, bool * error);
	std::optional<uint8_t> byteFromBinary(const std::string & data);
	uint16_t shortFromBinary(const std::string & data, bool * error, Endianness endianness = Endianness::BigEndian);
	std::optional<uint16_t> shortFromBinary(const std::string & data, Endianness endianness = Endianness::BigEndian);
	uint32_t integerFromBinary(const std::string & data, bool * error, Endianness endianness = Endianness::BigEndian);
	std::optional<uint32_t> integerFromBinary(const std::string & data, Endianness endianness = Endianness::BigEndian);
	uint64_t longFromBinary(const std::string & data, bool * error, Endianness endianness = Endianness::BigEndian);
	std::optional<uint64_t> longFromBinary(const std::string & data, Endianness endianness = Endianness::BigEndian);
	uint8_t byteFromOctal(const std::string & data, bool * error);
	std::optional<uint8_t> byteFromOctal(const std::string & data);
	uint16_t shortFromOctal(const std::string & data, bool * error, Endianness endianness = Endianness::BigEndian);
	std::optional<uint16_t> shortFromOctal(const std::string & data, Endianness endianness = Endianness::BigEndian);
	uint32_t integerFromOctal(const std::string & data, bool * error, Endianness endianness = Endianness::BigEndian);
	std::optional<uint32_t> integerFromOctal(const std::string & data, Endianness endianness = Endianness::BigEndian);
	uint64_t longFromOctal(const std::string & data, bool * error, Endianness endianness = Endianness::BigEndian);
	std::optional<uint64_t> longFromOctal(const std::string & data, Endianness endianness = Endianness::BigEndian);
	uint8_t byteFromHexadecimal(const std::string & data, bool * error);
	std::optional<uint8_t> byteFromHexadecimal(const std::string & data);
	uint16_t shortFromHexadecimal(const std::string & data, bool * error, Endianness endianness = Endianness::BigEndian);
	std::optional<uint16_t> shortFromHexadecimal(const std::string & data, Endianness endianness = Endianness::BigEndian);
	uint32_t integerFromHexadecimal(const std::string & data, bool * error, Endianness endianness = Endianness::BigEndian);
	std::optional<uint32_t> integerFromHexadecimal(const std::string & data, Endianness endianness = Endianness::BigEndian);
	uint64_t longFromHexadecimal(const std::string & data, bool * error, Endianness endianness = Endianness::BigEndian);
	std::optional<uint64_t> longFromHexadecimal(const std::string & data, Endianness endianness = Endianness::BigEndian);


}

#endif // _NUMBER_UTILITIES_H_
