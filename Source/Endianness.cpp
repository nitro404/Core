#include "Endianness.h"

#include <fmt/core.h>

bool isBigEndian() {
	static uint32_t i = 1;
	static const bool IS_BIG_ENDIAN = static_cast<uint32_t>(*(reinterpret_cast<uint8_t *>(&i))) == 1;

	return IS_BIG_ENDIAN;
}

bool isLittleEndian() {
	static const bool IS_LITTLE_ENDIAN = !isBigEndian();

	return IS_LITTLE_ENDIAN;
}

Endianness getEndianness() {
	static const Endianness ENDIANNESS = isBigEndian() ? Endianness::BigEndian : Endianness::LittleEndian;

	return ENDIANNESS;
}

int16_t littleToBigEndian(int16_t value) {
	return static_cast<int16_t>(
		(static_cast<int16_t>(static_cast<uint8_t>(value)            )) << 8 |
		(static_cast<int16_t>(static_cast<uint8_t>(value >> 8) & 0xff))
	);
}

uint16_t littleToBigEndian(uint16_t value) {
	return static_cast<uint16_t>(
		(static_cast<uint16_t>(static_cast<uint8_t>(value)            )) << 8 |
		(static_cast<uint16_t>(static_cast<uint8_t>(value >> 8) & 0xff))
	);
}

int32_t littleToBigEndian(int32_t value) {
	return static_cast<int32_t>(
		(static_cast<int32_t>(static_cast<uint8_t>(value)             )) << 24 |
		(static_cast<int32_t>(static_cast<uint8_t>(value >> 8)  & 0xff)) << 16 |
		(static_cast<int32_t>(static_cast<uint8_t>(value >> 16) & 0xff)) << 8  |
		(static_cast<int32_t>(static_cast<uint8_t>(value >> 24) & 0xff))
	);
}

uint32_t littleToBigEndian(uint32_t value) {
	return static_cast<uint32_t>(
		(static_cast<uint32_t>(static_cast<uint8_t>(value)             )) << 24 |
		(static_cast<uint32_t>(static_cast<uint8_t>(value >> 8)  & 0xff)) << 16 |
		(static_cast<uint32_t>(static_cast<uint8_t>(value >> 16) & 0xff)) << 8  |
		(static_cast<uint32_t>(static_cast<uint8_t>(value >> 24) & 0xff))
	);
}

int64_t littleToBigEndian(int64_t value) {
	return static_cast<int64_t>(
		(static_cast<int64_t>(static_cast<uint8_t>(value)             )) << 56 |
		(static_cast<int64_t>(static_cast<uint8_t>(value >> 8)  & 0xff)) << 48 |
		(static_cast<int64_t>(static_cast<uint8_t>(value >> 16) & 0xff)) << 40 |
		(static_cast<int64_t>(static_cast<uint8_t>(value >> 24) & 0xff)) << 32 |
		(static_cast<int64_t>(static_cast<uint8_t>(value >> 32) & 0xff)) << 24 |
		(static_cast<int64_t>(static_cast<uint8_t>(value >> 40) & 0xff)) << 16 |
		(static_cast<int64_t>(static_cast<uint8_t>(value >> 48) & 0xff)) << 8  |
		(static_cast<int64_t>(static_cast<uint8_t>(value >> 56) & 0xff))
	);
}

uint64_t littleToBigEndian(uint64_t value) {
	return static_cast<uint64_t>(
		(static_cast<uint64_t>(static_cast<uint8_t>(value)             )) << 56 |
		(static_cast<uint64_t>(static_cast<uint8_t>(value >> 8)  & 0xff)) << 48 |
		(static_cast<uint64_t>(static_cast<uint8_t>(value >> 16) & 0xff)) << 40 |
		(static_cast<uint64_t>(static_cast<uint8_t>(value >> 24) & 0xff)) << 32 |
		(static_cast<uint64_t>(static_cast<uint8_t>(value >> 32) & 0xff)) << 24 |
		(static_cast<uint64_t>(static_cast<uint8_t>(value >> 40) & 0xff)) << 16 |
		(static_cast<uint64_t>(static_cast<uint8_t>(value >> 48) & 0xff)) << 8  |
		(static_cast<uint64_t>(static_cast<uint8_t>(value >> 56) & 0xff))
	);
}

int16_t bigToLittleEndian(int16_t value) {
	return static_cast<int16_t>(
		(static_cast<int16_t>(static_cast<uint8_t>(value >> 8)       )) << 8 |
		(static_cast<int16_t>(static_cast<uint8_t>(value)      & 0xff))
	);
}

uint16_t bigToLittleEndian(uint16_t value) {
	return static_cast<uint16_t>(
		(static_cast<uint16_t>(static_cast<uint8_t>(value >> 8)       )) << 8 |
		(static_cast<uint16_t>(static_cast<uint8_t>(value)      & 0xff))
	);
}

int32_t bigToLittleEndian(int32_t value) {
	return static_cast<int32_t>(
		(static_cast<int32_t>(static_cast<uint8_t>(value)             )) << 24 |
		(static_cast<int32_t>(static_cast<uint8_t>(value >> 8)  & 0xff)) << 16 |
		(static_cast<int32_t>(static_cast<uint8_t>(value >> 16) & 0xff)) << 8  |
		(static_cast<int32_t>(static_cast<uint8_t>(value >> 24) & 0xff))
	);
}

uint32_t bigToLittleEndian(uint32_t value) {
	return static_cast<uint32_t>(
		(static_cast<uint32_t>(static_cast<uint8_t>(value)             )) << 24 |
		(static_cast<uint32_t>(static_cast<uint8_t>(value >> 8)  & 0xff)) << 16 |
		(static_cast<uint32_t>(static_cast<uint8_t>(value >> 16) & 0xff)) << 8  |
		(static_cast<uint32_t>(static_cast<uint8_t>(value >> 24) & 0xff))
	);
}

int64_t bigToLittleEndian(int64_t value) {
	return static_cast<int64_t>(
		static_cast<int64_t>((static_cast<uint8_t>(value)             )) << 56 |
		static_cast<int64_t>((static_cast<uint8_t>(value >> 8)  & 0xff)) << 48 |
		static_cast<int64_t>((static_cast<uint8_t>(value >> 16) & 0xff)) << 40 |
		static_cast<int64_t>((static_cast<uint8_t>(value >> 24) & 0xff)) << 32 |
		static_cast<int64_t>((static_cast<uint8_t>(value >> 32) & 0xff)) << 24 |
		static_cast<int64_t>((static_cast<uint8_t>(value >> 40) & 0xff)) << 16 |
		static_cast<int64_t>((static_cast<uint8_t>(value >> 48) & 0xff)) << 8  |
		static_cast<int64_t>((static_cast<uint8_t>(value >> 56) & 0xff))
	);
}

uint64_t bigToLittleEndian(uint64_t value) {
	return static_cast<uint64_t>(
		static_cast<uint64_t>((static_cast<uint8_t>(value)             )) << 56 |
		static_cast<uint64_t>((static_cast<uint8_t>(value >> 8)  & 0xff)) << 48 |
		static_cast<uint64_t>((static_cast<uint8_t>(value >> 16) & 0xff)) << 40 |
		static_cast<uint64_t>((static_cast<uint8_t>(value >> 24) & 0xff)) << 32 |
		static_cast<uint64_t>((static_cast<uint8_t>(value >> 32) & 0xff)) << 24 |
		static_cast<uint64_t>((static_cast<uint8_t>(value >> 40) & 0xff)) << 16 |
		static_cast<uint64_t>((static_cast<uint8_t>(value >> 48) & 0xff)) << 8  |
		static_cast<uint64_t>((static_cast<uint8_t>(value >> 56) & 0xff))
	);
}

int16_t fromBigEndian(int16_t value) {
	if(isBigEndian()) {
		return value;
	}

	return bigToLittleEndian(value);
}

uint16_t fromBigEndian(uint16_t value) {
	if(isBigEndian()) {
		return value;
	}

	return bigToLittleEndian(value);
}

int32_t fromBigEndian(int32_t value) {
	if(isBigEndian()) {
		return value;
	}

	return bigToLittleEndian(value);
}

uint32_t fromBigEndian(uint32_t value) {
	if(isBigEndian()) {
		return value;
	}

	return bigToLittleEndian(value);
}

int64_t fromBigEndian(int64_t value) {
	if(isBigEndian()) {
		return value;
	}

	return bigToLittleEndian(value);
}

uint64_t fromBigEndian(uint64_t value) {
	if(isBigEndian()) {
		return value;
	}

	return bigToLittleEndian(value);
}

int16_t fromLittleEndian(int16_t value) {
	if(isLittleEndian()) {
		return value;
	}

	return littleToBigEndian(value);
}

uint16_t fromLittleEndian(uint16_t value) {
	if(isLittleEndian()) {
		return value;
	}

	return littleToBigEndian(value);
}

int32_t fromLittleEndian(int32_t value) {
	if(isLittleEndian()) {
		return value;
	}

	return littleToBigEndian(value);
}

uint32_t fromLittleEndian(uint32_t value) {
	if(isLittleEndian()) {
		return value;
	}

	return littleToBigEndian(value);
}

int64_t fromLittleEndian(int64_t value) {
	if(isLittleEndian()) {
		return value;
	}

	return littleToBigEndian(value);
}

uint64_t fromLittleEndian(uint64_t value) {
	if(isLittleEndian()) {
		return value;
	}

	return littleToBigEndian(value);
}

int16_t fromEndian(int16_t value, Endianness endianness) {
	if(endianness == Endianness::BigEndian) {
		return fromBigEndian(value);
	}
	else {
		return fromLittleEndian(value);
	}
}

uint16_t fromEndian(uint16_t value, Endianness endianness) {
	if(endianness == Endianness::BigEndian) {
		return fromBigEndian(value);
	}
	else {
		return fromLittleEndian(value);
	}
}

int32_t fromEndian(int32_t value, Endianness endianness) {
	if(endianness == Endianness::BigEndian) {
		return fromBigEndian(value);
	}
	else {
		return fromLittleEndian(value);
	}
}

uint32_t fromEndian(uint32_t value, Endianness endianness) {
	if(endianness == Endianness::BigEndian) {
		return fromBigEndian(value);
	}
	else {
		return fromLittleEndian(value);
	}
}

int64_t fromEndian(int64_t value, Endianness endianness) {
	if(endianness == Endianness::BigEndian) {
		return fromBigEndian(value);
	}
	else {
		return fromLittleEndian(value);
	}
}

uint64_t fromEndian(uint64_t value, Endianness endianness) {
	if(endianness == Endianness::BigEndian) {
		return fromBigEndian(value);
	}
	else {
		return fromLittleEndian(value);
	}
}

int16_t toBigEndian(int16_t value) {
	if(isBigEndian()) {
		return value;
	}

	return littleToBigEndian(value);
}

uint16_t toBigEndian(uint16_t value) {
	if(isBigEndian()) {
		return value;
	}

	return littleToBigEndian(value);
}

int32_t toBigEndian(int32_t value) {
	if(isBigEndian()) {
		return value;
	}

	return littleToBigEndian(value);
}

uint32_t toBigEndian(uint32_t value) {
	if(isBigEndian()) {
		return value;
	}

	return littleToBigEndian(value);
}

int64_t toBigEndian(int64_t value) {
	if(isBigEndian()) {
		return value;
	}

	return littleToBigEndian(value);
}

uint64_t toBigEndian(uint64_t value) {
	if(isBigEndian()) {
		return value;
	}

	return littleToBigEndian(value);
}

int16_t toLittleEndian(int16_t value) {
	if(isLittleEndian()) {
		return value;
	}

	return bigToLittleEndian(value);
}

uint16_t toLittleEndian(uint16_t value) {
	if(isLittleEndian()) {
		return value;
	}

	return bigToLittleEndian(value);
}

int32_t toLittleEndian(int32_t value) {
	if(isLittleEndian()) {
		return value;
	}

	return bigToLittleEndian(value);
}

uint32_t toLittleEndian(uint32_t value) {
	if(isLittleEndian()) {
		return value;
	}

	return bigToLittleEndian(value);
}

int64_t toLittleEndian(int64_t value) {
	if(isLittleEndian()) {
		return value;
	}

	return bigToLittleEndian(value);
}

uint64_t toLittleEndian(uint64_t value) {
	if(isLittleEndian()) {
		return value;
	}

	return bigToLittleEndian(value);
}

int16_t toEndian(int16_t value, Endianness endianness) {
	if(endianness == Endianness::BigEndian) {
		return toBigEndian(value);
	}
	else {
		return toLittleEndian(value);
	}
}

uint16_t toEndian(uint16_t value, Endianness endianness) {
	if(endianness == Endianness::BigEndian) {
		return toBigEndian(value);
	}
	else {
		return toLittleEndian(value);
	}
}

int32_t toEndian(int32_t value, Endianness endianness) {
	if(endianness == Endianness::BigEndian) {
		return toBigEndian(value);
	}
	else {
		return toLittleEndian(value);
	}
}

uint32_t toEndian(uint32_t value, Endianness endianness) {
	if(endianness == Endianness::BigEndian) {
		return toBigEndian(value);
	}
	else {
		return toLittleEndian(value);
	}
}

int64_t toEndian(int64_t value, Endianness endianness) {
	if(endianness == Endianness::BigEndian) {
		return toBigEndian(value);
	}
	else {
		return toLittleEndian(value);
	}
}

uint64_t toEndian(uint64_t value, Endianness endianness) {
	if(endianness == Endianness::BigEndian) {
		return toBigEndian(value);
	}
	else {
		return toLittleEndian(value);
	}
}
