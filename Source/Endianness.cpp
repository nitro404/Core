#include "Endianness.h"

#include <bit>

bool isBigEndian() {
	return std::endian::native == std::endian::big;
}

bool isLittleEndian() {
	return std::endian::native == std::endian::little;
}

Endianness getEndianness() {
	static const Endianness ENDIANNESS = std::endian::native == std::endian::big ? Endianness::BigEndian : Endianness::LittleEndian;

	return ENDIANNESS;
}

int16_t fromBigEndian(int16_t value) {
	return isBigEndian() ? value : std::byteswap(value);
}

uint16_t fromBigEndian(uint16_t value) {
	return isBigEndian() ? value : std::byteswap(value);
}

int32_t fromBigEndian(int32_t value) {
	return isBigEndian() ? value : std::byteswap(value);
}

uint32_t fromBigEndian(uint32_t value) {
	return isBigEndian() ?  value : std::byteswap(value);
}

int64_t fromBigEndian(int64_t value) {
	return isBigEndian() ? value : std::byteswap(value);
}

uint64_t fromBigEndian(uint64_t value) {
	return isBigEndian() ? value : std::byteswap(value);
}

int16_t fromLittleEndian(int16_t value) {
	return isLittleEndian() ? value : std::byteswap(value);
}

uint16_t fromLittleEndian(uint16_t value) {
	return isLittleEndian() ? value : std::byteswap(value);
}

int32_t fromLittleEndian(int32_t value) {
	return isLittleEndian() ? value : std::byteswap(value);
}

uint32_t fromLittleEndian(uint32_t value) {
	return isLittleEndian() ? value : std::byteswap(value);
}

int64_t fromLittleEndian(int64_t value) {
	return isLittleEndian() ? value : std::byteswap(value);
}

uint64_t fromLittleEndian(uint64_t value) {
	return isLittleEndian() ? value : std::byteswap(value);
}

int16_t fromEndian(int16_t value, Endianness endianness) {
	return endianness == Endianness::BigEndian ? fromBigEndian(value) : fromLittleEndian(value);
}

uint16_t fromEndian(uint16_t value, Endianness endianness) {
	return endianness == Endianness::BigEndian ? fromBigEndian(value) : fromLittleEndian(value);
}

int32_t fromEndian(int32_t value, Endianness endianness) {
	return endianness == Endianness::BigEndian ? fromBigEndian(value) : fromLittleEndian(value);
}

uint32_t fromEndian(uint32_t value, Endianness endianness) {
	return endianness == Endianness::BigEndian ? fromBigEndian(value) : fromLittleEndian(value);
}

int64_t fromEndian(int64_t value, Endianness endianness) {
	return endianness == Endianness::BigEndian ? fromBigEndian(value) : fromLittleEndian(value);
}

uint64_t fromEndian(uint64_t value, Endianness endianness) {
	return endianness == Endianness::BigEndian ? fromBigEndian(value) : fromLittleEndian(value);
}

int16_t toBigEndian(int16_t value) {
	return isBigEndian() ? value : std::byteswap(value);
}

uint16_t toBigEndian(uint16_t value) {
	return isBigEndian() ? value : std::byteswap(value);
}

int32_t toBigEndian(int32_t value) {
	return isBigEndian() ? value : std::byteswap(value);
}

uint32_t toBigEndian(uint32_t value) {
	return isBigEndian() ? value : std::byteswap(value);
}

int64_t toBigEndian(int64_t value) {
	return isBigEndian() ? value : std::byteswap(value);
}

uint64_t toBigEndian(uint64_t value) {
	return isBigEndian() ? value :std::byteswap(value);
}

int16_t toLittleEndian(int16_t value) {
	return isLittleEndian() ? value : std::byteswap(value);
}

uint16_t toLittleEndian(uint16_t value) {
	return isLittleEndian() ? value : std::byteswap(value);
}

int32_t toLittleEndian(int32_t value) {
	return isLittleEndian() ? value : std::byteswap(value);
}

uint32_t toLittleEndian(uint32_t value) {
	return isLittleEndian() ? value : std::byteswap(value);
}

int64_t toLittleEndian(int64_t value) {
	return isLittleEndian() ? value : std::byteswap(value);
}

uint64_t toLittleEndian(uint64_t value) {
	return isLittleEndian() ? value : std::byteswap(value);
}

int16_t toEndian(int16_t value, Endianness endianness) {
	return endianness == Endianness::BigEndian ? toBigEndian(value) : toLittleEndian(value);
}

uint16_t toEndian(uint16_t value, Endianness endianness) {
	return endianness == Endianness::BigEndian ? toBigEndian(value) : toLittleEndian(value);
}

int32_t toEndian(int32_t value, Endianness endianness) {
	return endianness == Endianness::BigEndian ? toBigEndian(value) : toLittleEndian(value);
}

uint32_t toEndian(uint32_t value, Endianness endianness) {
	return endianness == Endianness::BigEndian ? toBigEndian(value) : toLittleEndian(value);
}

int64_t toEndian(int64_t value, Endianness endianness) {
	return endianness == Endianness::BigEndian ? toBigEndian(value) : toLittleEndian(value);
}

uint64_t toEndian(uint64_t value, Endianness endianness) {
	return endianness == Endianness::BigEndian ? toBigEndian(value) : toLittleEndian(value);
}
