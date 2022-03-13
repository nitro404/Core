#ifndef _ENDIANNESS_H_
#define _ENDIANNESS_H_

#include <cstdint>

enum class Endianness {
	BigEndian,
	LittleEndian
};

bool isBigEndian();
bool isLittleEndian();
Endianness getEndianness();
int16_t littleToBigEndian(int16_t value);
uint16_t littleToBigEndian(uint16_t value);
int32_t littleToBigEndian(int32_t value);
uint32_t littleToBigEndian(uint32_t value);
int64_t littleToBigEndian(int64_t value);
uint64_t littleToBigEndian(uint64_t value);
int16_t bigToLittleEndian(int16_t value);
uint16_t bigToLittleEndian(uint16_t value);
int32_t bigToLittleEndian(int32_t value);
uint32_t bigToLittleEndian(uint32_t value);
int64_t bigToLittleEndian(int64_t value);
uint64_t bigToLittleEndian(uint64_t value);
int16_t fromBigEndian(int16_t value);
uint16_t fromBigEndian(uint16_t value);
int32_t fromBigEndian(int32_t value);
uint32_t fromBigEndian(uint32_t value);
int64_t fromBigEndian(int64_t value);
uint64_t fromBigEndian(uint64_t value);
int16_t fromLittleEndian(int16_t value);
uint16_t fromLittleEndian(uint16_t value);
int32_t fromLittleEndian(int32_t value);
uint32_t fromLittleEndian(uint32_t value);
int64_t fromLittleEndian(int64_t value);
uint64_t fromLittleEndian(uint64_t value);
int16_t fromEndian(int16_t value, Endianness endianness);
uint16_t fromEndian(uint16_t value, Endianness endianness);
int32_t fromEndian(int32_t value, Endianness endianness);
uint32_t fromEndian(uint32_t value, Endianness endianness);
int64_t fromEndian(int64_t value, Endianness endianness);
uint64_t fromEndian(uint64_t value, Endianness endianness);
int16_t toBigEndian(int16_t value);
uint16_t toBigEndian(uint16_t value);
int32_t toBigEndian(int32_t value);
uint32_t toBigEndian(uint32_t value);
int64_t toBigEndian(int64_t value);
uint64_t toBigEndian(uint64_t value);
int16_t toLittleEndian(int16_t value);
uint16_t toLittleEndian(uint16_t value);
int32_t toLittleEndian(int32_t value);
uint32_t toLittleEndian(uint32_t value);
int64_t toLittleEndian(int64_t value);
uint64_t toLittleEndian(uint64_t value);
int16_t toEndian(int16_t value, Endianness endianness);
uint16_t toEndian(uint16_t value, Endianness endianness);
int32_t toEndian(int32_t value, Endianness endianness);
uint32_t toEndian(uint32_t value, Endianness endianness);
int64_t toEndian(int64_t value, Endianness endianness);
uint64_t toEndian(uint64_t value, Endianness endianness);

#endif // _ENDIANNESS_H_
