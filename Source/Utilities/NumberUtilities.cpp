#include "NumberUtilities.h"

#include <bitset>
#include <charconv>
#include <iomanip>
#include <ios>
#include <sstream>

uint32_t Utilities::byteLength(int8_t n) {
	return n < 0 ? n < -99 ? 4 : n < -9 ? 3 : 2 : n < 10 ? 1 : n < 100 ? 2 : 3;
}

uint32_t Utilities::unsignedByteLength(uint8_t n) {
	return n < 10 ? 1 : n < 100 ? 2 : 3;
}

uint32_t Utilities::shortLength(int16_t n) {
	return n < 0 ? n < -99 ? n < -9999 ? 6 : n < -999 ? 5 : 4 : n < -9 ? 3 : 2 : n < 100 ? n < 10 ? 1 : 2 : n < 1000 ? 3 : n < 10000 ? 4 : 5;
}

uint32_t Utilities::unsignedShortLength(uint16_t n) {
	return n < 100 ? n < 10 ? 1 : 2 : n < 1000 ? 3 : n < 10000 ? 4 : 5;
}

uint32_t Utilities::intLength(int32_t n) {
	return n < 0 ? n < -999999 ? n < -99999999 ? n < -999999999 ? 11 : 10 : n < -9999999 ? 9 : 8 : n < -999 ? n < -99999 ? 7 : n < -9999 ? 6 : 5 : n < -99 ? 4 : n < -9 ? 3 : 2 : n < 100000 ? n < 100 ? n < 10 ? 1 : 2 : n < 1000 ? 3 : n < 10000 ? 4 : 5 : n < 10000000 ? n < 1000000 ? 6 : 7 : n < 100000000 ? 8 : n < 1000000000 ? 9 : 10;
}

uint32_t Utilities::unsignedIntLength(uint32_t n) {
	return n < 100000 ? n < 100 ? n < 10 ? 1 : 2 : n < 1000 ? 3 : n < 10000 ? 4 : 5 : n < 10000000 ? n < 1000000 ? 6 : 7 : n < 100000000 ? 8 : n < 1000000000 ? 9 : 10;
}

uint32_t Utilities::longLength(int64_t n) {
	return n < 0 ? n < -999999999L ? n < -99999999999999L ? n < -9999999999999999L ? n < -999999999999999999L ? 20 : n < -99999999999999999L ? 19 : 18 : n < -999999999999999L ? 17 : 16 : n < -99999999999L ? n < -9999999999999L ? 15 : n < -999999999999L ? 14 : 13 : n < -9999999999L ? 12 : 11 : n < -9999L ? n < -999999L ? n < -99999999L ? 10 : n < -9999999L ? 9 : 8 : n < -99999L ? 7 : 6 : n < -99L ? n < -999L ? 5 : 4 : n < -9L ? 3 : 2 : n < 1000000000L ? n < 10000L ? n < 100L ? n < 10L ? 1 : 2 : n < 1000L ? 3 : 4 : n < 10000000L ? n < 100000L ? 5 : n < 1000000L ? 6 : 7 : n < 100000000L ? 8 : 9 : n < 100000000000000L ? n < 1000000000000L ? n < 100000000000L ? n < 10000000000L ? 10 : 11 : 12 : n < 10000000000000L ? 13 : 14 : n < 100000000000000000L ? n < 10000000000000000L ? n < 1000000000000000L ? 15 : 16 : 17 : n < 1000000000000000000L ? 18 : 19;
}

uint32_t Utilities::unsignedLongLength(uint64_t n) {
	return n < 1000000000L ? n < 10000L ? n < 100L ? n < 10L ? 1 : 2 : n < 1000L ? 3 : 4 : n < 10000000L ? n < 100000L ? 5 : n < 1000000L ? 6 : 7 : n < 100000000L ? 8 : 9 : n < 100000000000000L ? n < 1000000000000L ? n < 100000000000L ? n < 10000000000L ? 10 : 11 : 12 : n < 10000000000000L ? 13 : 14 : n < 100000000000000000L ? n < 10000000000000000L ? n < 1000000000000000L ? 15 : 16 : 17 : n < 1000000000000000000L ? 18 : 19;
}

std::string Utilities::toBinary(uint8_t value) {
	std::stringstream binaryStream;
	binaryStream << std::bitset<8>(value).to_string();

	return binaryStream.str();
}

std::string Utilities::toBinary(uint16_t value, Endianness endianness) {
	std::stringstream binaryStream;
	binaryStream << std::bitset<16>(toEndian(value, endianness)).to_string();

	return binaryStream.str();
}

std::string Utilities::toBinary(uint32_t value, Endianness endianness) {
	std::stringstream binaryStream;
	binaryStream << std::bitset<32>(toEndian(value, endianness)).to_string();

	return binaryStream.str();
}

std::string Utilities::toBinary(uint64_t value, Endianness endianness) {
	std::stringstream binaryStream;
	binaryStream << std::bitset<64>(toEndian(value, endianness)).to_string();

	return binaryStream.str();
}

std::string Utilities::toOctal(uint8_t value) {
	std::stringstream binaryStream;
	binaryStream << std::oct << std::setw(3) << std::setfill('0') << static_cast<uint16_t>(value);

	return binaryStream.str();
}

std::string Utilities::toOctal(uint16_t value, Endianness endianness) {
	std::stringstream binaryStream;
	binaryStream << std::oct << std::setw(6) << std::setfill('0') << toEndian(value, endianness);

	return binaryStream.str();
}

std::string Utilities::toOctal(uint32_t value, Endianness endianness) {
	std::stringstream binaryStream;
	binaryStream << std::oct << std::setw(11) << std::setfill('0') << toEndian(value, endianness);

	return binaryStream.str();
}

std::string Utilities::toOctal(uint64_t value, Endianness endianness) {
	std::stringstream binaryStream;
	binaryStream << std::oct << std::setw(22) << std::setfill('0') << toEndian(value, endianness);

	return binaryStream.str();
}

std::string Utilities::toHexadecimal(uint8_t value) {
	std::stringstream binaryStream;

	binaryStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint16_t>(value);

	return binaryStream.str();
}

std::string Utilities::toHexadecimal(uint16_t value, Endianness endianness) {
	std::stringstream binaryStream;
	binaryStream << std::hex << std::setw(4) << std::setfill('0') << toEndian(value, endianness);

	return binaryStream.str();
}

std::string Utilities::toHexadecimal(uint32_t value, Endianness endianness) {
	std::stringstream binaryStream;
	binaryStream << std::hex << std::setw(8) << std::setfill('0') << toEndian(value, endianness);

	return binaryStream.str();
}

std::string Utilities::toHexadecimal(uint64_t value, Endianness endianness) {
	std::stringstream binaryStream;
	binaryStream << std::hex << std::setw(16) << std::setfill('0') << toEndian(value, endianness);

	return binaryStream.str();
}

uint8_t Utilities::byteFromBinary(const std::string & data, bool * error) {
	if(data.length() != 8) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	uint8_t value = 0u;
	const std::from_chars_result result = std::from_chars(data.data(), data.data() + data.length(), value, 2);

	if(result.ec != std::errc{} || result.ptr != data.data() + data.length()) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	if(error) {
		*error = false;
	}

	return value;
}

std::optional<uint8_t> Utilities::byteFromBinary(const std::string & data) {
	bool error = false;

	uint8_t value = byteFromBinary(data, &error);

	if(error) {
		return {};
	}

	return value;
}

uint16_t Utilities::shortFromBinary(const std::string & data, bool * error, Endianness endianness) {
	if(data.length() != 16) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	uint16_t value = 0u;
	const std::from_chars_result result = std::from_chars(data.data(), data.data() + data.length(), value, 2);

	if(result.ec != std::errc{} || result.ptr != data.data() + data.length()) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	if(error) {
		*error = false;
	}

	return fromEndian(value, endianness);
}

std::optional<uint16_t> Utilities::shortFromBinary(const std::string & data, Endianness endianness) {
	bool error = false;

	uint16_t value = shortFromBinary(data, &error, endianness);

	if(error) {
		return {};
	}

	return value;
}

uint32_t Utilities::integerFromBinary(const std::string & data, bool * error, Endianness endianness) {
	if(data.length() != 32) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	uint32_t value = 0u;
	const std::from_chars_result result = std::from_chars(data.data(), data.data() + data.length(), value, 2);

	if(result.ec != std::errc{} || result.ptr != data.data() + data.length()) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	if(error) {
		*error = false;
	}

	return fromEndian(value, endianness);
}

std::optional<uint32_t> Utilities::integerFromBinary(const std::string & data, Endianness endianness) {
	bool error = false;

	uint32_t value = integerFromBinary(data, &error, endianness);

	if(error) {
		return {};
	}

	return value;
}

uint64_t Utilities::longFromBinary(const std::string & data, bool * error, Endianness endianness) {
	if(data.length() != 64) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	uint64_t value = 0u;
	const std::from_chars_result result = std::from_chars(data.data(), data.data() + data.length(), value, 2);

	if(result.ec != std::errc{} || result.ptr != data.data() + data.length()) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	if(error) {
		*error = false;
	}

	return fromEndian(value, endianness);
}

std::optional<uint64_t> Utilities::longFromBinary(const std::string & data, Endianness endianness) {
	bool error = false;

	uint64_t value = longFromBinary(data, &error, endianness);

	if(error) {
		return {};
	}

	return value;
}

uint8_t Utilities::byteFromOctal(const std::string & data, bool * error) {
	if(data.length() != 3) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	uint8_t value = 0u;
	const std::from_chars_result result = std::from_chars(data.data(), data.data() + data.length(), value, 8);

	if(result.ec != std::errc{} || result.ptr != data.data() + data.length()) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	if(error) {
		*error = false;
	}

	return value;
}

std::optional<uint8_t> Utilities::byteFromOctal(const std::string & data) {
	bool error = false;

	uint8_t value = byteFromOctal(data, &error);

	if(error) {
		return {};
	}

	return value;
}

uint16_t Utilities::shortFromOctal(const std::string & data, bool * error, Endianness endianness) {
	if(data.length() != 6) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	uint16_t value = 0u;
	const std::from_chars_result result = std::from_chars(data.data(), data.data() + data.length(), value, 8);

	if(result.ec != std::errc{} || result.ptr != data.data() + data.length()) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	if(error) {
		*error = false;
	}

	return fromEndian(value, endianness);
}

std::optional<uint16_t> Utilities::shortFromOctal(const std::string & data, Endianness endianness) {
	bool error = false;

	uint16_t value = shortFromOctal(data, &error, endianness);

	if(error) {
		return {};
	}

	return value;
}

uint32_t Utilities::integerFromOctal(const std::string & data, bool * error, Endianness endianness) {
	if(data.length() != 11) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	uint32_t value = 0u;
	const std::from_chars_result result = std::from_chars(data.data(), data.data() + data.length(), value, 8);

	if(result.ec != std::errc{} || result.ptr != data.data() + data.length()) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	if(error) {
		*error = false;
	}

	return fromEndian(value, endianness);
}

std::optional<uint32_t> Utilities::integerFromOctal(const std::string & data, Endianness endianness) {
	bool error = false;

	uint32_t value = integerFromOctal(data, &error, endianness);

	if(error) {
		return {};
	}

	return value;
}

uint64_t Utilities::longFromOctal(const std::string & data, bool * error, Endianness endianness) {
	if(data.length() != 22) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	uint64_t value = 0u;
	const std::from_chars_result result = std::from_chars(data.data(), data.data() + data.length(), value, 8);

	if(result.ec != std::errc{} || result.ptr != data.data() + data.length()) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	if(error) {
		*error = false;
	}

	return fromEndian(value, endianness);
}

std::optional<uint64_t> Utilities::longFromOctal(const std::string & data, Endianness endianness) {
	bool error = false;

	uint64_t value = longFromOctal(data, &error, endianness);

	if(error) {
		return {};
	}

	return value;
}

uint8_t Utilities::byteFromHexadecimal(const std::string & data, bool * error) {
	if(data.length() != 2) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	uint8_t value = 0u;
	const std::from_chars_result result = std::from_chars(data.data(), data.data() + data.length(), value, 16);

	if(result.ec != std::errc{} || result.ptr != data.data() + data.length()) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	if(error) {
		*error = false;
	}

	return value;
}

std::optional<uint8_t> Utilities::byteFromHexadecimal(const std::string & data) {
	bool error = false;

	uint8_t value = byteFromHexadecimal(data, &error);

	if(error) {
		return {};
	}

	return value;
}

uint16_t Utilities::shortFromHexadecimal(const std::string & data, bool * error, Endianness endianness) {
	if(data.length() != 4) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	uint16_t value = 0u;
	const std::from_chars_result result = std::from_chars(data.data(), data.data() + data.length(), value, 16);

	if(result.ec != std::errc{} || result.ptr != data.data() + data.length()) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	if(error) {
		*error = false;
	}

	return fromEndian(value, endianness);
}

std::optional<uint16_t> Utilities::shortFromHexadecimal(const std::string & data, Endianness endianness) {
	bool error = false;

	uint16_t value = shortFromHexadecimal(data, &error, endianness);

	if(error) {
		return {};
	}

	return value;
}

uint32_t Utilities::integerFromHexadecimal(const std::string & data, bool * error, Endianness endianness) {
	if(data.length() != 8) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	uint32_t value = 0u;
	const std::from_chars_result result = std::from_chars(data.data(), data.data() + data.length(), value, 16);

	if(result.ec != std::errc{} || result.ptr != data.data() + data.length()) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	if(error) {
		*error = false;
	}

	return fromEndian(value, endianness);
}

std::optional<uint32_t> Utilities::integerFromHexadecimal(const std::string & data, Endianness endianness) {
	bool error = false;

	uint32_t value = integerFromHexadecimal(data, &error, endianness);

	if(error) {
		return {};
	}

	return value;
}

uint64_t Utilities::longFromHexadecimal(const std::string & data, bool * error, Endianness endianness) {
	if(data.length() != 16) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	uint64_t value = 0u;
	const std::from_chars_result result = std::from_chars(data.data(), data.data() + data.length(), value, 16);

	if(result.ec != std::errc{} || result.ptr != data.data() + data.length()) {
		if(error) {
			*error = true;
		}

		return 0u;
	}

	if(error) {
		*error = false;
	}

	return fromEndian(value, endianness);
}

std::optional<uint64_t> Utilities::longFromHexadecimal(const std::string & data, Endianness endianness) {
	bool error = false;

	uint64_t value = longFromHexadecimal(data, &error, endianness);

	if(error) {
		return {};
	}

	return value;
}
