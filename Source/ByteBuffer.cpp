#include "ByteBuffer.h"

#include "Compression/BZip2Utilities.h"
#include "Compression/LZMAUtilities.h"
#include "Compression/ZLibUtilities.h"
#include "Utilities/FileUtilities.h"
#include "Utilities/NumberUtilities.h"
#include "Utilities/StringUtilities.h"

#include <cryptopp/cryptlib.h>
#include <cryptopp/md5.h>
#include <cryptopp/sha.h>
#include <double-conversion/ieee.h>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <zstd.h>

#include <bitset>
#include <filesystem>
#include <fstream>
#include <functional>
#include <ios>
#include <sstream>
#include <utility>

static constexpr const char * BASE_64_CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static constexpr const char * BASE_16_CHARACTERS = "0123456789ABCDEF";

const Endianness ByteBuffer::DEFAULT_ENDIANNESS = Endianness::BigEndian;
const ByteBuffer::HashFormat ByteBuffer::DEFAULT_HASH_FORMAT = HashFormat::Hexadecimal;
const ByteBuffer ByteBuffer::EMPTY_BYTE_BUFFER;

ByteBuffer::ByteBuffer(Endianness endianness)
	: m_data(std::make_unique<std::vector<uint8_t>>())
	, m_endianness(endianness)
	, m_readOffset(0)
	, m_writeOffset(0) { }

ByteBuffer::ByteBuffer(size_t initialCapacity, Endianness endianness)
	: m_data(std::make_unique<std::vector<uint8_t>>(initialCapacity))
	, m_endianness(endianness)
	, m_readOffset(0)
	, m_writeOffset(0) { }

ByteBuffer::ByteBuffer(const uint8_t * data, size_t size, Endianness endianness)
	: m_data(std::make_unique<std::vector<uint8_t>>())
	, m_endianness(endianness)
	, m_readOffset(0)
	, m_writeOffset(size) {
	if(data != nullptr) {
		m_data->assign(data, data + size);
	}
}

ByteBuffer::ByteBuffer(const std::vector<uint8_t> & data, Endianness endianness)
	: m_data(std::make_unique<std::vector<uint8_t>>(data))
	, m_endianness(endianness)
	, m_readOffset(0)
	, m_writeOffset(data.size()) { }

ByteBuffer::ByteBuffer(std::unique_ptr<std::vector<uint8_t>> data, Endianness endianness)
	: m_data(data != nullptr ? std::move(data) : std::make_unique<std::vector<uint8_t>>())
	, m_endianness(endianness)
	, m_readOffset(0) {
	m_writeOffset = m_data->size();
}

ByteBuffer::ByteBuffer(const std::string & data, Endianness endianness)
	: m_data(std::make_unique<std::vector<uint8_t>>())
	, m_endianness(endianness)
	, m_readOffset(0)
	, m_writeOffset(data.size()) {
	m_data->assign(data.data(), data.data() + data.length());
}

ByteBuffer::ByteBuffer(ByteBuffer && buffer) noexcept
	: m_data(std::move(buffer.m_data))
	, m_endianness(buffer.m_endianness)
	, m_readOffset(buffer.m_readOffset)
	, m_writeOffset(buffer.m_writeOffset) { }

ByteBuffer::ByteBuffer(const ByteBuffer & buffer)
	: m_data(std::make_unique<std::vector<uint8_t>>(*buffer.m_data))
	, m_endianness(buffer.m_endianness)
	, m_readOffset(buffer.m_readOffset)
	, m_writeOffset(buffer.m_writeOffset) { }

ByteBuffer & ByteBuffer::operator = (const char * data) {
	m_data->assign(reinterpret_cast<const uint8_t *>(data), reinterpret_cast<const uint8_t *>(data + Utilities::stringLength(data)));
	m_readOffset = 0;
	m_writeOffset = 0;

	return *this;
}

ByteBuffer & ByteBuffer::operator = (const std::vector<uint8_t> & data) {
	*m_data = data;
	m_readOffset = 0;
	m_writeOffset = 0;

	return *this;
}

ByteBuffer & ByteBuffer::operator = (const std::string & data) {
	m_data->assign(reinterpret_cast<const uint8_t *>(data.data()), reinterpret_cast<const uint8_t *>(data.data() + data.length()));
	m_readOffset = 0;
	m_writeOffset = 0;

	return *this;
}

ByteBuffer & ByteBuffer::operator = (ByteBuffer && buffer) noexcept {
	if(this != &buffer) {
		m_data = std::move(buffer.m_data);
		m_endianness = buffer.m_endianness;
		m_readOffset = buffer.m_readOffset;
		m_writeOffset = buffer.m_writeOffset;
	}

	return *this;
}

ByteBuffer & ByteBuffer::operator = (const ByteBuffer & buffer) {
	m_data->assign(buffer.m_data->data(), buffer.m_data->data() + buffer.m_data->size());
	m_endianness = buffer.m_endianness;
	m_readOffset = buffer.m_readOffset;
	m_writeOffset = buffer.m_writeOffset;

	return *this;
}

ByteBuffer::~ByteBuffer() = default;

const std::vector<uint8_t> & ByteBuffer::getData() const {
	return *m_data;
}

std::vector<uint8_t> & ByteBuffer::getData() {
	return *m_data;
}

const uint8_t * ByteBuffer::getRawData() const {
	return m_data->data();
}

uint8_t * ByteBuffer::getRawData() {
	return m_data->data();
}

void ByteBuffer::setData(const uint8_t * data, size_t size) {
	if(data == nullptr) {
		m_data->clear();
	}
	else {
		m_data->assign(reinterpret_cast<const uint8_t *>(data), reinterpret_cast<const uint8_t *>(data + size));
	}

	m_readOffset = 0;
	m_writeOffset = 0;
}

void ByteBuffer::setData(const std::vector<uint8_t> & data) {
	m_data->assign(data.data(), data.data() + data.size());
	m_readOffset = 0;
	m_writeOffset = 0;
}

void ByteBuffer::setData(const std::string & data) {
	m_data->assign(reinterpret_cast<const uint8_t *>(data.data()), reinterpret_cast<const uint8_t *>(data.data() + data.length()));
	m_readOffset = 0;
	m_writeOffset = 0;
}

void ByteBuffer::setData(const ByteBuffer & buffer) {
	m_data->assign(buffer.m_data->data(), buffer.m_data->data() + buffer.m_data->size());
	m_readOffset = 0;
	m_writeOffset = 0;
}

bool ByteBuffer::isEmpty() const {
	return m_data->empty();
}

bool ByteBuffer::isNotEmpty() const {
	return !m_data->empty();
}

bool ByteBuffer::isFull() const {
	return m_data->size() == m_data->max_size();
}

size_t ByteBuffer::getSize() const {
	return m_data->size();
}

size_t ByteBuffer::getCapacity() const {
	return m_data->capacity();
}

void ByteBuffer::resize(size_t size, uint8_t value) {
	m_data->resize(size, value);
}

bool ByteBuffer::reserve(size_t capacity) {
	if(capacity < m_data->size()) {
		return false;
	}

	m_data->reserve(capacity);

	return true;
}

void ByteBuffer::shrinkToFit() {
	m_data->shrink_to_fit();
}

Endianness ByteBuffer::getEndianness() const {
	return m_endianness;
}

void ByteBuffer::setEndianness(Endianness endianness) const {
	m_endianness = endianness;
}

void ByteBuffer::fill(uint8_t value, size_t start, size_t end) {
	if(start >= end) {
		return;
	}

	memset(m_data->data() + (start * sizeof(uint8_t)), 0, ((end < m_data->size() ? end : m_data->size() - 1) - start + 1) * sizeof(uint8_t));
}

void ByteBuffer::reverse(size_t start, size_t end) {
	if(m_data->size() < 2) {
		return;
	}

	size_t size = m_data->size();
	size_t max = end < size ? end : size - 1;
	size_t middle = ((end - start) / 2) + 1;

	for(size_t i = start; i < middle; i++) {
		std::swap((*m_data)[i], (*m_data)[size - i]);
	}
}

void ByteBuffer::clear() {
	m_data->clear();
}

std::string ByteBuffer::getMD5(HashFormat hashFormat) const {
	return getHash<CryptoPP::Weak::MD5>(hashFormat);
}

std::string ByteBuffer::getSHA1(HashFormat hashFormat) const {
	return getHash<CryptoPP::SHA1>(hashFormat);
}

std::string ByteBuffer::getSHA256(HashFormat hashFormat) const {
	return getHash<CryptoPP::SHA256>(hashFormat);
}

std::string ByteBuffer::getSHA512(HashFormat hashFormat) const {
	return getHash<CryptoPP::SHA512>(hashFormat);
}

std::string ByteBuffer::getHash(HashType hashType, HashFormat hashFormat) const {
	switch(hashType) {
		case HashType::MD5: {
			return getMD5(hashFormat);
		}
		case HashType::SHA1: {
			return getSHA1(hashFormat);
		}
		case HashType::SHA256: {
			return getSHA256(hashFormat);
		}
		case HashType::SHA512: {
			return getSHA512(hashFormat);
		}
	}

	return {};
}

size_t ByteBuffer::getReadOffset() const {
	return m_readOffset;
}

void ByteBuffer::setReadOffset(size_t offset) const {
	m_readOffset = offset > m_data->size() ? m_data->size() : offset;
}

bool ByteBuffer::canReadBytes(size_t numberOfBytes) const {
	return m_data->size() - m_readOffset >= numberOfBytes;
}

bool ByteBuffer::skipReadBytes(size_t numberOfBytes) const {
	if(checkOverflow(m_readOffset, numberOfBytes)) {
		return false;
	}

	setReadOffset(m_readOffset + numberOfBytes);

	return true;
}

bool ByteBuffer::hasMoreLines() const {
	uint8_t currentByte = 0;

	for(size_t i = m_readOffset; i < m_data->size(); i++) {
		currentByte = (*m_data)[i];

		if(currentByte == '\n' || currentByte == '\r') {
			return true;
		}
	}

	return false;
}

bool ByteBuffer::skipToNextLine(size_t * endOfLineIndex) const {
	size_t nextLineIndex = indexOfNextLine(endOfLineIndex);

	if(nextLineIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	setReadOffset(nextLineIndex);

	return true;
}

size_t ByteBuffer::indexOfNextLine(size_t * endOfLineIndex) const {
	return indexOfNextLineFrom(m_readOffset, endOfLineIndex);
}

size_t ByteBuffer::indexOfNextLineFrom(size_t offset, size_t * endOfLineIndex) const {
	bool foundCarriageReturn = false;
	bool foundNewline = false;
	uint8_t currentByte = 0;

	while(offset < m_data->size()) {
		currentByte = (*m_data)[offset];

		if(currentByte == '\n') {
			if(foundNewline) {
				return offset;
			}
			else if(!foundCarriageReturn) {
				if(endOfLineIndex != nullptr) {
					*endOfLineIndex = offset == 0 ? 0 : offset - 1;
				}
			}

			foundNewline = true;
		}
		else if(currentByte == '\r' && !foundNewline) {
			foundCarriageReturn = true;

			if(endOfLineIndex != nullptr) {
				*endOfLineIndex = offset == 0 ? 0 : offset - 1;
			}
		}
		else if(foundCarriageReturn || foundNewline) {
			return offset;
		}

		offset++;
	}

	return std::numeric_limits<size_t>::max();
}

size_t ByteBuffer::numberOfBytesRemaining() const {
	return m_data->size() - m_readOffset;
}

std::unique_ptr<ByteBuffer> ByteBuffer::getRemainingBytes() const {
	size_t remainingBytesCount = numberOfBytesRemaining();

	if(m_readOffset + (remainingBytesCount * sizeof(uint8_t)) > m_data->size()) {
		return nullptr;
	}

	const uint8_t * rawDataStart = m_data->data() + (m_readOffset * sizeof(uint8_t));

	return std::make_unique<ByteBuffer>(std::make_unique<std::vector<uint8_t>>(rawDataStart, rawDataStart + (remainingBytesCount * sizeof(uint8_t))));
}

bool ByteBuffer::isEndOfBuffer() const {
	return m_readOffset >= m_data->size();
}

void ByteBuffer::resetReadOffset() const {
	setReadOffset(0);
}

size_t ByteBuffer::getWriteOffset() const {
	return m_writeOffset;
}

void ByteBuffer::setWriteOffset(size_t offset) const {
	m_writeOffset = offset;
}

void ByteBuffer::resetWriteOffset() const {
	setWriteOffset(0);
}

bool ByteBuffer::skipWriteBytes(size_t numberOfBytes) const {
	if(checkOverflow(m_readOffset, numberOfBytes)) {
		return false;
	}

	setWriteOffset(m_writeOffset + numberOfBytes);

	return true;
}

int8_t ByteBuffer::getByte(size_t offset, bool * error) const {
	if(offset + sizeof(int8_t) > m_data->size()) {
		if(error != nullptr) {
			*error = true;
		}

		return 0;
	}

	return static_cast<int8_t>((*m_data)[offset]);
}

std::optional<int8_t> ByteBuffer::getByte(size_t offset) const {
	bool error = false;

	int8_t value = getByte(offset, &error);

	if(error) {
		return {};
	}

	return value;
}

uint8_t ByteBuffer::getUnsignedByte(size_t offset, bool * error) const {
	if(offset + sizeof(uint8_t) > m_data->size()) {
		if(error != nullptr) {
			*error = true;
		}

		return 0;
	}

	return (*m_data)[offset];
}

std::optional<uint8_t> ByteBuffer::getUnsignedByte(size_t offset) const {
	bool error = false;

	uint8_t value = getUnsignedByte(offset, &error);

	if(error) {
		return {};
	}

	return value;
}

int16_t ByteBuffer::getShort(size_t offset, bool * error) const {
	return static_cast<int16_t>(getUnsignedShort(offset, error));
}

std::optional<int16_t> ByteBuffer::getShort(size_t offset) const {
	bool error = false;

	int16_t value = getShort(offset, &error);

	if(error) {
		return {};
	}

	return value;
}

uint16_t ByteBuffer::getUnsignedShort(size_t offset, bool * error) const {
	if(offset + sizeof(uint16_t) > m_data->size()) {
		if(error != nullptr) {
			*error = true;
		}

		return 0;
	}

	if(m_endianness == Endianness::BigEndian) {
		return fromBigEndian(static_cast<uint16_t>(
			static_cast<uint16_t>((*m_data)[offset    ]       ) << 8 |
			static_cast<uint16_t>((*m_data)[offset + 1] & 0xff)
		));
	}
	else {
		return fromBigEndian(static_cast<uint16_t>(
			static_cast<uint16_t>((*m_data)[offset + 1]       ) << 8 |
			static_cast<uint16_t>((*m_data)[offset    ] & 0xff)
		));
	}
}

std::optional<uint16_t> ByteBuffer::getUnsignedShort(size_t offset) const {
	bool error = false;

	uint16_t value = getUnsignedShort(offset, &error);

	if(error) {
		return {};
	}

	return value;
}

int32_t ByteBuffer::getInteger(size_t offset, bool * error) const {
	return static_cast<int32_t>(getUnsignedInteger(offset, error));
}

std::optional<int32_t> ByteBuffer::getInteger(size_t offset) const {
	bool error = false;

	int32_t value = getInteger(offset, &error);

	if(error) {
		return {};
	}

	return value;
}

uint32_t ByteBuffer::getUnsignedInteger(size_t offset, bool * error) const {
	if(offset + sizeof(uint32_t) > m_data->size()) {
		if(error != nullptr) {
			*error = true;
		}

		return 0;
	}

	if(m_endianness == Endianness::BigEndian) {
		return fromBigEndian(static_cast<uint32_t>(
			static_cast<uint32_t>((*m_data)[offset    ]       ) << 24 |
			static_cast<uint32_t>((*m_data)[offset + 1] & 0xff) << 16 |
			static_cast<uint32_t>((*m_data)[offset + 2] & 0xff) << 8  |
			static_cast<uint32_t>((*m_data)[offset + 3] & 0xff)
		));
	}
	else {
		return fromBigEndian(static_cast<uint32_t>(
			static_cast<uint32_t>((*m_data)[offset + 3]       ) << 24 |
			static_cast<uint32_t>((*m_data)[offset + 2] & 0xff) << 16 |
			static_cast<uint32_t>((*m_data)[offset + 1] & 0xff) << 8  |
			static_cast<uint32_t>((*m_data)[offset    ] & 0xff)
		));
	}
}

std::optional<uint32_t> ByteBuffer::getUnsignedInteger(size_t offset) const {
	bool error = false;

	uint32_t value = getUnsignedInteger(offset, &error);

	if(error) {
		return {};
	}

	return value;
}

int64_t ByteBuffer::getLong(size_t offset, bool * error) const {
	return static_cast<int64_t>(getUnsignedLong(offset, error));
}

std::optional<int64_t> ByteBuffer::getLong(size_t offset) const {
	bool error = false;

	int64_t value = getLong(offset, &error);

	if(error) {
		return {};
	}

	return value;
}

uint64_t ByteBuffer::getUnsignedLong(size_t offset, bool * error) const {
	if(offset + sizeof(uint64_t) > m_data->size()) {
		if(error != nullptr) {
			*error = true;
		}

		return 0L;
	}

	if(m_endianness == Endianness::BigEndian) {
		return fromBigEndian(static_cast<uint64_t>(
			static_cast<uint64_t>((*m_data)[offset    ]       ) << 56 |
			static_cast<uint64_t>((*m_data)[offset + 1] & 0xff) << 48 |
			static_cast<uint64_t>((*m_data)[offset + 2] & 0xff) << 40 |
			static_cast<uint64_t>((*m_data)[offset + 3] & 0xff) << 32 |
			static_cast<uint64_t>((*m_data)[offset + 4] & 0xff) << 24 |
			static_cast<uint64_t>((*m_data)[offset + 5] & 0xff) << 16 |
			static_cast<uint64_t>((*m_data)[offset + 6] & 0xff) << 8  |
			static_cast<uint64_t>((*m_data)[offset + 7] & 0xff)
		));
	}
	else {
		return fromBigEndian(static_cast<uint64_t>(
			static_cast<uint64_t>((*m_data)[offset + 7]       ) << 56 |
			static_cast<uint64_t>((*m_data)[offset + 6] & 0xff) << 48 |
			static_cast<uint64_t>((*m_data)[offset + 5] & 0xff) << 40 |
			static_cast<uint64_t>((*m_data)[offset + 4] & 0xff) << 32 |
			static_cast<uint64_t>((*m_data)[offset + 3] & 0xff) << 24 |
			static_cast<uint64_t>((*m_data)[offset + 2] & 0xff) << 16 |
			static_cast<uint64_t>((*m_data)[offset + 1] & 0xff) << 8  |
			static_cast<uint64_t>((*m_data)[offset    ] & 0xff)
		));
	}
}

std::optional<uint64_t> ByteBuffer::getUnsignedLong(size_t offset) const {
	bool error = false;

	uint64_t value = getUnsignedLong(offset, &error);

	if(error) {
		return {};
	}

	return value;
}

float ByteBuffer::getFloat(size_t offset, bool * error) const {
	return double_conversion::uint32_to_float(getUnsignedInteger(offset, error));
}

std::optional<float> ByteBuffer::getFloat(size_t offset) const {
	bool error = false;

	float value = getFloat(offset, &error);

	if(error) {
		return {};
	}

	return value;
}

double ByteBuffer::getDouble(size_t offset, bool * error) const {
	return double_conversion::uint64_to_double(getUnsignedLong(offset, error));
}

std::optional<double> ByteBuffer::getDouble(size_t offset) const {
	bool error = false;

	double value = getDouble(offset, &error);

	if(error) {
		return {};
	}

	return value;
}

std::string ByteBuffer::getString(size_t length, size_t offset, bool * error) const {
	if(offset + (length * sizeof(uint8_t)) > m_data->size()) {
		if(error != nullptr) {
			*error = true;
		}

		return std::string();
	}

	size_t actualLength = length;

	for(size_t i = 0; i < length; i++) {
		if((*m_data)[(offset + i) * sizeof(uint8_t)] == 0) {
			actualLength = i;
			break;
		}
	}

	return std::string(reinterpret_cast<const char *>(m_data->data() + (offset * sizeof(uint8_t))), actualLength);
}

std::optional<std::string> ByteBuffer::getString(size_t length, size_t offset) const {
	bool error = false;

	std::string value(getString(length, offset, &error));

	if(error) {
		return {};
	}

	return value;
}

std::string ByteBuffer::getNullTerminatedString(size_t offset, bool * error) const {
	if(isEndOfBuffer()) {
		if(error != nullptr) {
			*error = true;
		}

		return std::string();
	}

	return std::string(reinterpret_cast<const char *>(m_data->data() + (offset * sizeof(uint8_t))));
}

std::optional<std::string> ByteBuffer::getNullTerminatedString(size_t offset) const {
	bool error = false;

	std::string value(getNullTerminatedString(offset, &error));

	if(error) {
		return {};
	}

	return value;
}

std::string ByteBuffer::getLine(size_t offset, size_t * nextLineIndex, bool * error) const {
	if(offset >= m_data->size()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	size_t endOfLineIndex = std::numeric_limits<size_t>::max();
	size_t nextLineIndexInternal = indexOfNextLineFrom(offset, &endOfLineIndex);
	size_t lineLength = nextLineIndexInternal != std::numeric_limits<size_t>::max() ? endOfLineIndex - offset + 1 : m_data->size() - offset;

	if(nextLineIndex != nullptr) {
		*nextLineIndex = nextLineIndexInternal;
	}

	return std::string(reinterpret_cast<const char *>(m_data->data() + (m_readOffset * sizeof(uint8_t))), lineLength);
}

std::optional<std::string> ByteBuffer::getLine(size_t offset, size_t * nextLineIndex) const {
	bool error = false;

	std::string value(getLine(offset, nextLineIndex, &error));

	if(error) {
		return {};
	}

	return value;
}

std::unique_ptr<std::vector<uint8_t>> ByteBuffer::getBytes(size_t numberOfBytes, size_t offset) const {
	if(offset + (numberOfBytes * sizeof(uint8_t)) > m_data->size()) {
		return nullptr;
	}

	const uint8_t * rawDataStart = m_data->data() + (offset * sizeof(uint8_t));

	return std::make_unique<std::vector<uint8_t>>(rawDataStart, rawDataStart + (numberOfBytes * sizeof(uint8_t)));
}

int8_t ByteBuffer::peekByte(bool * error) const {
	bool e = false;
	int8_t value = getByte(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}

	return value;
}

std::optional<int8_t> ByteBuffer::peekByte() const {
	bool error = false;

	int8_t value = peekByte(&error);

	if(error) {
		return {};
	}

	return value;
}

uint8_t ByteBuffer::peekUnsignedByte(bool * error) const {
	bool e = false;
	uint8_t value = getUnsignedByte(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}

	return value;
}

std::optional<uint8_t> ByteBuffer::peekUnsignedByte() const {
	bool error = false;

	uint8_t value = peekUnsignedByte(&error);

	if(error) {
		return {};
	}

	return value;
}

int16_t ByteBuffer::peekShort(bool * error) const {
	bool e = false;
	int16_t value = getShort(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}

	return value;
}

std::optional<int16_t> ByteBuffer::peekShort() const {
	bool error = false;

	int16_t value = peekShort(&error);

	if(error) {
		return {};
	}

	return value;
}

uint16_t ByteBuffer::peekUnsignedShort(bool * error) const {
	bool e = false;
	uint16_t value = getUnsignedShort(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}

	return value;
}

std::optional<uint16_t> ByteBuffer::peekUnsignedShort() const {
	bool error = false;

	uint16_t value = peekUnsignedShort(&error);

	if(error) {
		return {};
	}

	return value;
}

int32_t ByteBuffer::peekInteger(bool * error) const {
	bool e = false;
	int32_t value = getInteger(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}

	return value;
}

std::optional<int32_t> ByteBuffer::peekInteger() const {
	bool error = false;

	int32_t value = peekInteger(&error);

	if(error) {
		return {};
	}

	return value;
}

uint32_t ByteBuffer::peekUnsignedInteger(bool * error) const {
	bool e = false;
	uint32_t value = getUnsignedInteger(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}

	return value;
}

std::optional<uint32_t> ByteBuffer::peekUnsignedInteger() const {
	bool error = false;

	uint32_t value = peekUnsignedInteger(&error);

	if(error) {
		return {};
	}

	return value;
}

int64_t ByteBuffer::peekLong(bool * error) const {
	bool e = false;
	int64_t value = getLong(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}

	return value;
}

std::optional<int64_t> ByteBuffer::peekLong() const {
	bool error = false;

	int64_t value = peekLong(&error);

	if(error) {
		return {};
	}

	return value;
}

uint64_t ByteBuffer::peekUnsignedLong(bool * error) const {
	bool e = false;
	uint64_t value = getUnsignedLong(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}

	return value;
}

std::optional<uint64_t> ByteBuffer::peekUnsignedLong() const {
	bool error = false;

	uint64_t value = peekUnsignedLong(&error);

	if(error) {
		return {};
	}

	return value;
}

float ByteBuffer::peekFloat(bool * error) const {
	bool e = false;
	float value = getFloat(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}

	return value;
}

std::optional<float> ByteBuffer::peekFloat() const {
	bool error = false;

	float value = peekFloat(&error);

	if(error) {
		return {};
	}

	return value;
}

double ByteBuffer::peekDouble(bool * error) const {
	bool e = false;
	double value = getDouble(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}

	return value;
}

std::optional<double> ByteBuffer::peekDouble() const {
	bool error = false;

	double value = peekDouble(&error);

	if(error) {
		return {};
	}

	return value;
}

std::string ByteBuffer::peekString(size_t length, bool * error) const {
	bool e = false;
	std::string value(getString(length, m_readOffset, &e));

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}

	return value;
}

std::optional<std::string> ByteBuffer::peekString(size_t length) const {
	bool error = false;

	std::string value(peekString(length, &error));

	if(error) {
		return {};
	}

	return value;
}

std::string ByteBuffer::peekNullTerminatedString(bool * error) const {
	bool e = false;
	std::string value(getNullTerminatedString(m_readOffset, &e));

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}

	return value;
}

std::optional<std::string> ByteBuffer::peekNullTerminatedString() const {
	bool error = false;

	std::string value(peekNullTerminatedString(&error));

	if(error) {
		return {};
	}

	return value;
}

std::string ByteBuffer::peekLine(bool * error) const {
	bool e = false;
	size_t nextLineIndex = std::numeric_limits<size_t>::max();
	std::string line(getLine(m_readOffset, &nextLineIndex, &e));

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}

	return line;
}

std::optional<std::string> ByteBuffer::peekLine() const {
	bool error = false;

	std::string value(peekLine(&error));

	if(error) {
		return {};
	}

	return value;
}

std::unique_ptr<std::vector<uint8_t>> ByteBuffer::peekBytes(size_t numberOfBytes) const {
	return getBytes(numberOfBytes, m_readOffset);
}

int8_t ByteBuffer::readByte(bool * error) const {
	bool e = false;
	int8_t value = getByte(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		m_readOffset += sizeof(int8_t);
	}

	return value;
}

std::optional<int8_t> ByteBuffer::readByte() const {
	bool error = false;

	int8_t value = readByte(&error);

	if(error) {
		return {};
	}

	return value;
}

uint8_t ByteBuffer::readUnsignedByte(bool * error) const {
	bool e = false;
	uint8_t value = getUnsignedByte(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		m_readOffset += sizeof(uint8_t);
	}

	return value;
}

std::optional<uint8_t> ByteBuffer::readUnsignedByte() const {
	bool error = false;

	uint8_t value = readUnsignedByte(&error);

	if(error) {
		return {};
	}

	return value;
}

int16_t ByteBuffer::readShort(bool * error) const {
	bool e = false;
	int16_t value = getShort(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		m_readOffset += sizeof(int16_t);
	}

	return value;
}

std::optional<int16_t> ByteBuffer::readShort() const {
	bool error = false;

	int16_t value = readShort(&error);

	if(error) {
		return {};
	}

	return value;
}

uint16_t ByteBuffer::readUnsignedShort(bool * error) const {
	bool e = false;
	uint16_t value = getUnsignedShort(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		m_readOffset += sizeof(uint16_t);
	}

	return value;
}

std::optional<uint16_t> ByteBuffer::readUnsignedShort() const {
	bool error = false;

	uint16_t value = readUnsignedShort(&error);

	if(error) {
		return {};
	}

	return value;
}

int32_t ByteBuffer::readInteger(bool * error) const {
	bool e = false;
	int32_t value = getInteger(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		m_readOffset += sizeof(int32_t);
	}

	return value;
}

std::optional<int32_t> ByteBuffer::readInteger() const {
	bool error = false;

	int32_t value = readInteger(&error);

	if(error) {
		return {};
	}

	return value;
}

uint32_t ByteBuffer::readUnsignedInteger(bool * error) const {
	bool e = false;
	uint32_t value = getUnsignedInteger(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		m_readOffset += sizeof(uint32_t);
	}

	return value;
}

std::optional<uint32_t> ByteBuffer::readUnsignedInteger() const {
	bool error = false;

	uint32_t value = readUnsignedInteger(&error);

	if(error) {
		return {};
	}

	return value;
}

int64_t ByteBuffer::readLong(bool * error) const {
	bool e = false;
	int64_t value = getLong(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		m_readOffset += sizeof(int64_t);
	}

	return value;
}

std::optional<int64_t> ByteBuffer::readLong() const {
	bool error = false;

	int64_t value = readLong(&error);

	if(error) {
		return {};
	}

	return value;
}

uint64_t ByteBuffer::readUnsignedLong(bool * error) const {
	bool e = false;
	uint64_t value = getUnsignedLong(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		m_readOffset += sizeof(uint64_t);
	}

	return value;
}

std::optional<uint64_t> ByteBuffer::readUnsignedLong() const {
	bool error = false;

	uint64_t value = readUnsignedLong(&error);

	if(error) {
		return {};
	}

	return value;
}

float ByteBuffer::readFloat(bool * error) const {
	bool e = false;
	float value = getFloat(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		m_readOffset += sizeof(uint32_t);
	}

	return value;
}

std::optional<float> ByteBuffer::readFloat() const {
	bool error = false;

	float value = readFloat(&error);

	if(error) {
		return {};
	}

	return value;
}

double ByteBuffer::readDouble(bool * error) const {
	bool e = false;
	double value = getDouble(m_readOffset, &e);

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		m_readOffset += sizeof(uint64_t);
	}

	return value;
}

std::optional<double> ByteBuffer::readDouble() const {
	bool error = false;

	double value = readDouble(&error);

	if(error) {
		return {};
	}

	return value;
}

std::string ByteBuffer::readString(size_t length, bool * error) const {
	bool e = false;
	std::string value(getString(length, m_readOffset, &e));

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		m_readOffset += length * sizeof(uint8_t);
	}

	return value;
}

std::optional<std::string> ByteBuffer::readString(size_t length) const {
	bool error = false;

	std::string value(readString(length, &error));

	if(error) {
		return {};
	}

	return value;
}

std::string ByteBuffer::readNullTerminatedString(bool * error) const {
	bool e = false;
	std::string value(getNullTerminatedString(m_readOffset, &e));

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		m_readOffset += value.length() * sizeof(uint8_t);
	}

	return value;
}

std::optional<std::string> ByteBuffer::readNullTerminatedString() const {
	bool error = false;

	std::string value(readNullTerminatedString(&error));

	if(error) {
		return {};
	}

	return value;
}

std::string ByteBuffer::readLine(bool * error) const {
	bool e = false;
	size_t nextLineIndex = std::numeric_limits<size_t>::max();
	std::string line(getLine(m_readOffset, &nextLineIndex, &e));

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		m_readOffset = nextLineIndex != std::numeric_limits<size_t>::max() ? nextLineIndex : m_data->size();
	}

	return line;
}

std::optional<std::string> ByteBuffer::readLine() const {
	bool error = false;

	std::string value(readLine(&error));

	if(error) {
		return {};
	}

	return value;
}

std::unique_ptr<std::vector<uint8_t>> ByteBuffer::readBytes(size_t numberOfBytes) const {
	std::unique_ptr<std::vector<uint8_t>> value(getBytes(numberOfBytes, m_readOffset));

	if(value != nullptr) {
		m_readOffset += numberOfBytes * sizeof(uint8_t);
	}

	return value;
}

bool ByteBuffer::putByte(int8_t value, size_t offset) {
	if(!autoResize(offset, sizeof(int8_t))) {
		return false;
	}

	(*m_data)[offset] = static_cast<uint8_t>(value);

	return true;
}

bool ByteBuffer::putUnsignedByte(uint8_t value, size_t offset) {
	if(!autoResize(offset, sizeof(uint8_t))) {
		return false;
	}

	(*m_data)[offset] = value;

	return true;
}

bool ByteBuffer::putShort(int16_t value, size_t offset) {
	return putUnsignedShort(static_cast<uint16_t>(value), offset);
}

bool ByteBuffer::putUnsignedShort(uint16_t value, size_t offset) {
	if(!autoResize(offset, sizeof(uint16_t))) {
		return false;
	}

	uint16_t bigEndianValue = toBigEndian(value);

	if(m_endianness == Endianness::BigEndian) {
		(*m_data)[offset    ] = static_cast<uint8_t>(bigEndianValue >> 8);
		(*m_data)[offset + 1] = static_cast<uint8_t>(bigEndianValue);
	}
	else {
		(*m_data)[offset + 1] = static_cast<uint8_t>(bigEndianValue >> 8);
		(*m_data)[offset    ] = static_cast<uint8_t>(bigEndianValue);
	}

	return true;
}

bool ByteBuffer::putInteger(int32_t value, size_t offset) {
	return putUnsignedInteger(static_cast<uint32_t>(value), offset);
}

bool ByteBuffer::putUnsignedInteger(uint32_t value, size_t offset) {
	if(!autoResize(offset, sizeof(uint32_t))) {
		return false;
	}

	uint32_t bigEndianValue = toBigEndian(value);

	if(m_endianness == Endianness::BigEndian) {
		(*m_data)[offset    ] = static_cast<uint8_t>(bigEndianValue >> 24);
		(*m_data)[offset + 1] = static_cast<uint8_t>(bigEndianValue >> 16);
		(*m_data)[offset + 2] = static_cast<uint8_t>(bigEndianValue >> 8);
		(*m_data)[offset + 3] = static_cast<uint8_t>(bigEndianValue);
	}
	else {
		(*m_data)[offset + 3] = static_cast<uint8_t>(bigEndianValue >> 24);
		(*m_data)[offset + 2] = static_cast<uint8_t>(bigEndianValue >> 16);
		(*m_data)[offset + 1] = static_cast<uint8_t>(bigEndianValue >> 8);
		(*m_data)[offset    ] = static_cast<uint8_t>(bigEndianValue);
	}

	return true;
}

bool ByteBuffer::putLong(int64_t value, size_t offset) {
	return putUnsignedLong(static_cast<uint64_t>(value), offset);
}

bool ByteBuffer::putUnsignedLong(uint64_t value, size_t offset) {
	if(!autoResize(offset, sizeof(uint64_t))) {
		return false;
	}

	uint64_t bigEndianValue = toBigEndian(value);

	if(m_endianness == Endianness::BigEndian) {
		(*m_data)[offset    ] = static_cast<uint8_t>(bigEndianValue >> 56);
		(*m_data)[offset + 1] = static_cast<uint8_t>(bigEndianValue >> 48);
		(*m_data)[offset + 2] = static_cast<uint8_t>(bigEndianValue >> 40);
		(*m_data)[offset + 3] = static_cast<uint8_t>(bigEndianValue >> 32);
		(*m_data)[offset + 4] = static_cast<uint8_t>(bigEndianValue >> 24);
		(*m_data)[offset + 5] = static_cast<uint8_t>(bigEndianValue >> 16);
		(*m_data)[offset + 6] = static_cast<uint8_t>(bigEndianValue >> 8);
		(*m_data)[offset + 7] = static_cast<uint8_t>(bigEndianValue);
	}
	else {
		(*m_data)[offset + 7] = static_cast<uint8_t>(bigEndianValue >> 56);
		(*m_data)[offset + 6] = static_cast<uint8_t>(bigEndianValue >> 48);
		(*m_data)[offset + 5] = static_cast<uint8_t>(bigEndianValue >> 40);
		(*m_data)[offset + 4] = static_cast<uint8_t>(bigEndianValue >> 32);
		(*m_data)[offset + 3] = static_cast<uint8_t>(bigEndianValue >> 24);
		(*m_data)[offset + 2] = static_cast<uint8_t>(bigEndianValue >> 16);
		(*m_data)[offset + 1] = static_cast<uint8_t>(bigEndianValue >> 8);
		(*m_data)[offset    ] = static_cast<uint8_t>(bigEndianValue);
	}

	return true;
}

bool ByteBuffer::putFloat(float value, size_t offset) {
	return putUnsignedInteger(double_conversion::float_to_uint32(value), offset);
}

bool ByteBuffer::putDouble(double value, size_t offset) {
	return putUnsignedLong(double_conversion::double_to_uint64(value), offset);
}

bool ByteBuffer::putString(const std::string & value, size_t offset) {
	return putBytes(reinterpret_cast<const uint8_t *>(value.data()), value.length(), offset);
}

bool ByteBuffer::putNullTerminatedString(const std::string & value, size_t offset) {
	return putBytes(reinterpret_cast<const uint8_t *>(value.c_str()), value.length() + 1, offset);
}

bool ByteBuffer::putLine(const std::string & value, size_t offset, const std::string & newLine) {
	return putString(value + newLine, offset);
}

bool ByteBuffer::putBytes(const uint8_t * data, size_t size, size_t offset) {
	if(data == nullptr || !autoResize(offset, size * sizeof(uint8_t))) {
		return false;
	}

	memcpy(m_data->data() + (offset * sizeof(uint8_t)), data, size);

	return true;
}

bool ByteBuffer::putBytes(const std::vector<uint8_t> data, size_t offset) {
	return putBytes(data.data(), data.size(), offset);
}

bool ByteBuffer::putBytes(const ByteBuffer & buffer, size_t offset) {
	return putBytes(buffer.m_data->data(), buffer.m_data->size(), offset);
}

bool ByteBuffer::insertByte(int8_t value, size_t offset) {
	if(checkOverflow(m_data->size(), sizeof(int8_t))) {
		return false;
	}

	m_data->insert(m_data->begin() + offset, static_cast<uint8_t>(value));

	return true;
}

bool ByteBuffer::insertUnsignedByte(uint8_t value, size_t offset) {
	if(checkOverflow(m_data->size(), sizeof(uint8_t))) {
		return false;
	}

	m_data->insert(m_data->begin() + offset, value);

	return true;
}

bool ByteBuffer::insertShort(int16_t value, size_t offset) {
	return insertUnsignedShort(static_cast<uint16_t>(value), offset);
}

bool ByteBuffer::insertUnsignedShort(uint16_t value, size_t offset) {
	if(checkOverflow(m_data->size(), sizeof(uint16_t))) {
		return false;
	}

	uint16_t bigEndianValue = toBigEndian(value);
	uint8_t data[2];

	if(m_endianness == Endianness::BigEndian) {
		data[0] = static_cast<uint8_t>(bigEndianValue >> 8);
		data[1] = static_cast<uint8_t>(bigEndianValue);
	}
	else {
		data[1] = static_cast<uint8_t>(bigEndianValue >> 8);
		data[0] = static_cast<uint8_t>(bigEndianValue);
	}

	m_data->insert(m_data->begin() + offset, data, data + sizeof(uint16_t));

	return true;
}

bool ByteBuffer::insertInteger(int32_t value, size_t offset) {
	return insertUnsignedInteger(static_cast<uint32_t>(value), offset);
}

bool ByteBuffer::insertUnsignedInteger(uint32_t value, size_t offset) {
	if(checkOverflow(m_data->size(), sizeof(uint32_t))) {
		return false;
	}

	uint32_t bigEndianValue = toBigEndian(value);
	uint8_t data[4];

	if(m_endianness == Endianness::BigEndian) {
		data[0] = static_cast<uint8_t>(bigEndianValue >> 24);
		data[1] = static_cast<uint8_t>(bigEndianValue >> 16);
		data[2] = static_cast<uint8_t>(bigEndianValue >> 8);
		data[3] = static_cast<uint8_t>(bigEndianValue);
	}
	else {
		data[3] = static_cast<uint8_t>(bigEndianValue >> 24);
		data[2] = static_cast<uint8_t>(bigEndianValue >> 16);
		data[1] = static_cast<uint8_t>(bigEndianValue >> 8);
		data[0] = static_cast<uint8_t>(bigEndianValue);
	}

	m_data->insert(m_data->begin() + offset, data, data + sizeof(uint32_t));

	return true;
}

bool ByteBuffer::insertLong(int64_t value, size_t offset) {
	return insertUnsignedLong(static_cast<uint64_t>(value), offset);
}

bool ByteBuffer::insertUnsignedLong(uint64_t value, size_t offset) {
	if(checkOverflow(m_data->size(), sizeof(uint64_t))) {
		return false;
	}

	uint64_t bigEndianValue = toBigEndian(value);
	uint8_t data[8];

	if(m_endianness == Endianness::BigEndian) {
		data[0] = static_cast<uint8_t>(bigEndianValue >> 56);
		data[1] = static_cast<uint8_t>(bigEndianValue >> 48);
		data[2] = static_cast<uint8_t>(bigEndianValue >> 40);
		data[3] = static_cast<uint8_t>(bigEndianValue >> 32);
		data[4] = static_cast<uint8_t>(bigEndianValue >> 24);
		data[5] = static_cast<uint8_t>(bigEndianValue >> 16);
		data[6] = static_cast<uint8_t>(bigEndianValue >> 8);
		data[7] = static_cast<uint8_t>(bigEndianValue);
	}
	else {
		data[7] = static_cast<uint8_t>(bigEndianValue >> 56);
		data[6] = static_cast<uint8_t>(bigEndianValue >> 48);
		data[5] = static_cast<uint8_t>(bigEndianValue >> 40);
		data[4] = static_cast<uint8_t>(bigEndianValue >> 32);
		data[3] = static_cast<uint8_t>(bigEndianValue >> 24);
		data[2] = static_cast<uint8_t>(bigEndianValue >> 16);
		data[1] = static_cast<uint8_t>(bigEndianValue >> 8);
		data[0] = static_cast<uint8_t>(bigEndianValue);
	}

	m_data->insert(m_data->begin() + offset, data, data + sizeof(uint64_t));

	return true;
}

bool ByteBuffer::insertFloat(float value, size_t offset) {
	return insertUnsignedInteger(double_conversion::float_to_uint32(value), offset);
}

bool ByteBuffer::insertDouble(double value, size_t offset) {
	return insertUnsignedLong(double_conversion::double_to_uint64(value), offset);
}

bool ByteBuffer::insertString(const std::string & value, size_t offset) {
	return insertBytes(reinterpret_cast<const uint8_t *>(value.data()), value.length(), offset);
}

bool ByteBuffer::insertNullTerminatedString(const std::string & value, size_t offset) {
	return insertBytes(reinterpret_cast<const uint8_t *>(value.c_str()), value.length() + 1, offset);
}

bool ByteBuffer::insertLine(const std::string & value, size_t offset, const std::string & newLine) {
	return insertString(value + newLine, offset);
}

bool ByteBuffer::insertBytes(const uint8_t * data, size_t size, size_t offset) {
	if(data == nullptr || checkOverflow(m_data->size(), size * sizeof(uint8_t))) {
		return false;
	}

	m_data->insert(m_data->begin() + offset, data, data + (size * sizeof(uint8_t)));

	return true;
}

bool ByteBuffer::insertBytes(const std::vector<uint8_t> data, size_t offset) {
	return insertBytes(data.data(), data.size(), offset);
}

bool ByteBuffer::insertBytes(const ByteBuffer & buffer, size_t offset) {
	return insertBytes(buffer.m_data->data(), buffer.m_data->size(), offset);
}

bool ByteBuffer::writeByte(int8_t value) {
	if(putByte(value, m_writeOffset)) {
		m_writeOffset += sizeof(int8_t);

		return true;
	}

	return false;
}

bool ByteBuffer::writeUnsignedByte(uint8_t value) {
	if(putUnsignedByte(value, m_writeOffset)) {
		m_writeOffset += sizeof(uint8_t);

		return true;
	}

	return false;
}

bool ByteBuffer::writeShort(int16_t value) {
	if(putShort(value, m_writeOffset)) {
		m_writeOffset += sizeof(int16_t);

		return true;
	}

	return false;
}

bool ByteBuffer::writeUnsignedShort(uint16_t value) {
	if(putUnsignedShort(value, m_writeOffset)) {
		m_writeOffset += sizeof(uint16_t);

		return true;
	}

	return false;
}

bool ByteBuffer::writeInteger(int32_t value) {
	if(putInteger(value, m_writeOffset)) {
		m_writeOffset += sizeof(int32_t);

		return true;
	}

	return false;
}

bool ByteBuffer::writeUnsignedInteger(uint32_t value) {
	if(putUnsignedInteger(value, m_writeOffset)) {
		m_writeOffset += sizeof(uint32_t);

		return true;
	}

	return false;
}

bool ByteBuffer::writeLong(int64_t value) {
	if(putLong(value, m_writeOffset)) {
		m_writeOffset += sizeof(int64_t);

		return true;
	}

	return false;
}

bool ByteBuffer::writeUnsignedLong(uint64_t value) {
	if(putUnsignedLong(value, m_writeOffset)) {
		m_writeOffset += sizeof(uint64_t);

		return true;
	}

	return false;
}

bool ByteBuffer::writeFloat(float value) {
	if(putFloat(value, m_writeOffset)) {
		m_writeOffset += sizeof(uint32_t);

		return true;
	}

	return false;
}

bool ByteBuffer::writeDouble(double value) {
	if(putDouble(value, m_writeOffset)) {
		m_writeOffset += sizeof(uint64_t);

		return true;
	}

	return false;
}

bool ByteBuffer::writeString(const std::string & value) {
	if(putString(value, m_writeOffset)) {
		m_writeOffset += value.length() * sizeof(uint8_t);

		return true;
	}

	return false;
}

bool ByteBuffer::writeNullTerminatedString(const std::string & value) {
	if(putNullTerminatedString(value, m_writeOffset)) {
		m_writeOffset += (value.length() + 1) * sizeof(uint8_t);

		return true;
	}

	return false;
}

bool ByteBuffer::writeLine(const std::string & value, const std::string & newLine) {
	return writeString(value + newLine);
}

bool ByteBuffer::writeBytes(const uint8_t * data, size_t size) {
	if(data == nullptr) {
		return false;
	}

	if(putBytes(data, size, m_writeOffset)) {
		m_writeOffset += size * sizeof(uint8_t);

		return true;
	}

	return false;
}

bool ByteBuffer::writeBytes(const std::vector<uint8_t> data) {
	if(putBytes(data, m_writeOffset)) {
		m_writeOffset += data.size() * sizeof(uint8_t);

		return true;
	}

	return false;
}

bool ByteBuffer::writeBytes(const ByteBuffer & buffer) {
	if(putBytes(buffer, m_writeOffset)) {
		m_writeOffset += buffer.getSize() * sizeof(uint8_t);

		return true;
	}

	return false;
}

std::unique_ptr<ByteBuffer> ByteBuffer::clone() const {
	std::unique_ptr<ByteBuffer> copy(std::make_unique<ByteBuffer>(*m_data, m_endianness));
	copy->m_readOffset = m_readOffset;
	copy->m_writeOffset = m_writeOffset;

	return copy;
}

std::unique_ptr<ByteBuffer> ByteBuffer::copyOfRange(size_t start, size_t end) const {
	if(start >= end || end >= m_data->size()) {
		return nullptr;
	}

	return std::make_unique<ByteBuffer>(m_data->data() + (start * sizeof(uint8_t)), end - start + 1);
}

std::unique_ptr<ByteBuffer> ByteBuffer::decompressed(CompressionMethod decompressionMethod, size_t offset, size_t size) const {
	if(offset == std::numeric_limits<size_t>::max()) {
		offset = m_readOffset;
	}

	if(size == std::numeric_limits<size_t>::max()) {
		size = m_data->size();
	}

	if(size > m_data->size() - offset) {
		size = m_data->size() - offset;
	}

	if(size == 0) {
		return nullptr;
	}

	std::unique_ptr<ByteBuffer> decompressedData(std::make_unique<ByteBuffer>());

	switch(decompressionMethod) {
		case CompressionMethod::BZip2: {
			BZip2::StreamHandle bZip2Stream(BZip2::createDecompressionStreamHandle());

			if(bZip2Stream == nullptr) {
				return nullptr;
			}

			static constexpr uint64_t OUTPUT_BUFFER_SIZE = 4096;
			uint8_t outputBuffer[OUTPUT_BUFFER_SIZE];
			int result = BZ_OK;

			bZip2Stream->next_in = reinterpret_cast<char *>(const_cast<uint8_t *>(m_data->data())) + offset;
			bZip2Stream->avail_in = size;
			bZip2Stream->next_out = reinterpret_cast<char *>(outputBuffer);
			bZip2Stream->avail_out = OUTPUT_BUFFER_SIZE;

			while(true) {
				result = BZ2_bzDecompress(bZip2Stream.get());

				if(!BZip2::isSuccess(result, "Failed to decompress BZip2 data")) {
					return nullptr;
				}

				if(!decompressedData->writeBytes(outputBuffer, OUTPUT_BUFFER_SIZE - bZip2Stream->avail_out)) {
					spdlog::error("Failed to write decompressed BZip2 data to buffer.");
					return nullptr;
				}

				bZip2Stream->next_out = reinterpret_cast<char *>(outputBuffer);
				bZip2Stream->avail_out = OUTPUT_BUFFER_SIZE;

				if(result == BZ_STREAM_END) {
					return decompressedData;
				}
			}

			break;
		}
		case CompressionMethod::LZMA:
		case CompressionMethod::XZ: {
			LZMA::StreamHandle lzmaStream(LZMA::createStreamHandle());

			if(lzmaStream == nullptr) {
				spdlog::error("Failed to initialize {} stream handle.", magic_enum::enum_name(decompressionMethod));
				return nullptr;
			}

			lzma_ret lzmaStatus = lzma_auto_decoder(lzmaStream.get(), std::numeric_limits<uint64_t>::max(), 0);

			if(!LZMA::isSuccess(lzmaStatus, "Failed to initialize LZMA decoder")) {
				return nullptr;
			}

			static constexpr uint64_t OUTPUT_BUFFER_SIZE = 4096;
			uint8_t outputBuffer[OUTPUT_BUFFER_SIZE];

			lzmaStream->next_in = m_data->data() + offset;
			lzmaStream->avail_in = size;
			lzmaStream->next_out = outputBuffer;
			lzmaStream->avail_out = OUTPUT_BUFFER_SIZE;

			while(true) {
				lzmaStatus = lzma_code(lzmaStream.get(), LZMA_FINISH);

				if(!decompressedData->writeBytes(outputBuffer, OUTPUT_BUFFER_SIZE - lzmaStream->avail_out)) {
					spdlog::error("Failed to write decompressed LZMA data to buffer.");
					return nullptr;
				}

				lzmaStream->next_out = outputBuffer;
				lzmaStream->avail_out = OUTPUT_BUFFER_SIZE;

				if(lzmaStatus == LZMA_STREAM_END) {
					return decompressedData;
				}

				if(!LZMA::isSuccess(lzmaStatus, "Failed to decompress LZMA data")) {
					return nullptr;
				}
			}

			break;
		}
		case CompressionMethod::ZLib: {
			ZLib::StreamHandle zLibStream(ZLib::createInflationStreamHandle());

			if(zLibStream == nullptr) {
				return nullptr;
			}

			static constexpr uint64_t OUTPUT_BUFFER_SIZE = 4096;
			uint8_t outputBuffer[OUTPUT_BUFFER_SIZE];
			int zLibResult = Z_OK;

			zLibStream->next_in = reinterpret_cast<Bytef *>(const_cast<uint8_t *>(m_data->data())) + offset;
			zLibStream->avail_in = size;
			zLibStream->next_out = reinterpret_cast<Bytef *>(outputBuffer);
			zLibStream->avail_out = OUTPUT_BUFFER_SIZE;

			while(true) {
				zLibResult = inflate(zLibStream.get(), Z_NO_FLUSH);

				if(!ZLib::isSuccess(zLibResult, "Failed to decompress ZLib data")) {
					return nullptr;
				}

				if(!decompressedData->writeBytes(outputBuffer, OUTPUT_BUFFER_SIZE - zLibStream->avail_out)) {
					spdlog::error("Failed to write decompressed ZLib data to buffer.");
					return nullptr;
				}

				zLibStream->next_out = reinterpret_cast<Bytef *>(outputBuffer);
				zLibStream->avail_out = OUTPUT_BUFFER_SIZE;

				if(zLibResult == Z_STREAM_END) {
					return decompressedData;
				}
			}

			break;
		}
		case CompressionMethod::ZStandard: {
			size_t uncompressedSize = ZSTD_getFrameContentSize(m_data->data() + offset, size);

			if(uncompressedSize == ZSTD_CONTENTSIZE_UNKNOWN || uncompressedSize == ZSTD_CONTENTSIZE_ERROR) {
				spdlog::error("Failed to determine decompressed Zstandard data size.");
				return nullptr;
			}

			decompressedData->resize(uncompressedSize);

			if(ZSTD_isError(ZSTD_decompress(decompressedData->getRawData(), decompressedData->getSize(), m_data->data() + offset, size))) {
				spdlog::error("Failed to decompress Zstandard data.");
				return nullptr;
			}

			return decompressedData;
		}
	}

	return nullptr;
}

std::unique_ptr<ByteBuffer> ByteBuffer::compressed(CompressionMethod compressionMethod, size_t offset, size_t size) const {
	if(offset == std::numeric_limits<size_t>::max()) {
		offset = m_readOffset;
	}

	if(size == std::numeric_limits<size_t>::max()) {
		size = m_data->size();
	}

	if(size > m_data->size() - offset) {
		size = m_data->size() - offset;
	}

	if(size == 0) {
		return nullptr;
	}

	std::unique_ptr<ByteBuffer> compressedData(std::make_unique<ByteBuffer>());

	switch(compressionMethod) {
		case CompressionMethod::BZip2: {
			BZip2::StreamHandle bZip2Stream(BZip2::createCompressionStreamHandle());

			if(bZip2Stream == nullptr) {
				return nullptr;
			}

			static constexpr uint64_t OUTPUT_BUFFER_SIZE = 4096;
			uint8_t outputBuffer[OUTPUT_BUFFER_SIZE];
			int result = BZ_OK;

			bZip2Stream->next_in = reinterpret_cast<char *>(const_cast<uint8_t *>(m_data->data())) + offset;
			bZip2Stream->avail_in = size;
			bZip2Stream->next_out = reinterpret_cast<char *>(outputBuffer);
			bZip2Stream->avail_out = OUTPUT_BUFFER_SIZE;

			while(true) {
				result = BZ2_bzCompress(bZip2Stream.get(), BZ_FINISH);

				if(!BZip2::isSuccess(result, "Failed to compress BZip2 data")) {
					return nullptr;
				}

				if(!compressedData->writeBytes(outputBuffer, OUTPUT_BUFFER_SIZE - bZip2Stream->avail_out)) {
					spdlog::error("Failed to write compressed BZip2 data to buffer.");
					return nullptr;
				}

				bZip2Stream->next_out = reinterpret_cast<char *>(outputBuffer);
				bZip2Stream->avail_out = OUTPUT_BUFFER_SIZE;

				if(result == BZ_STREAM_END) {
					return compressedData;
				}
			}

			break;
		}
		case CompressionMethod::LZMA:
		case CompressionMethod::XZ: {
			LZMA::StreamHandle lzmaStream(LZMA::createStreamHandle());

			if(lzmaStream == nullptr) {
				spdlog::error("Failed to initialize {} stream handle.", magic_enum::enum_name(compressionMethod));
				return nullptr;
			}

			lzma_ret lzmaStatus = LZMA_OK;

			if(compressionMethod == CompressionMethod::LZMA) {
				lzma_options_lzma lzmaOptions;

				if(lzma_lzma_preset(&lzmaOptions, LZMA_PRESET_DEFAULT)) {
					spdlog::error("Failed to initialize LZMA encoder options with default preset.");
					return nullptr;
				}

				lzmaStatus = lzma_alone_encoder(lzmaStream.get(), &lzmaOptions);
			}
			else if(compressionMethod == CompressionMethod::XZ) {
				// TODO: Allow LZMA XZ compression present to be configurable (ie. 0 [fastest] - 9 [slowest], LZMA_PRESET_EXTREME)
				static constexpr uint32_t DEFAULT_LZMA_COMPRESSION_PRESENT = 4;
				lzmaStatus = lzma_easy_encoder(lzmaStream.get(), DEFAULT_LZMA_COMPRESSION_PRESENT, LZMA_CHECK_CRC64);
			}

			if(!LZMA::isSuccess(lzmaStatus, "Failed to initialize easy LZMA encoder")) {
				return nullptr;
			}

			static constexpr uint64_t OUTPUT_BUFFER_SIZE = 4096;
			uint8_t outputBuffer[OUTPUT_BUFFER_SIZE];

			lzmaStream->next_in = m_data->data() + offset;
			lzmaStream->avail_in = size;
			lzmaStream->next_out = outputBuffer;
			lzmaStream->avail_out = OUTPUT_BUFFER_SIZE;

			while(true) {
				lzmaStatus = lzma_code(lzmaStream.get(), LZMA_FINISH);

				if(!compressedData->writeBytes(outputBuffer, OUTPUT_BUFFER_SIZE - lzmaStream->avail_out)) {
					spdlog::error("Failed to write compressed LZMA data to buffer.");
					return nullptr;
				}

				lzmaStream->next_out = outputBuffer;
				lzmaStream->avail_out = OUTPUT_BUFFER_SIZE;

				if(lzmaStatus == LZMA_STREAM_END) {
					return compressedData;
				}

				if(!LZMA::isSuccess(lzmaStatus, "Failed to compress LZMA data")) {
					return nullptr;
				}
			}

			break;
		}
		case CompressionMethod::ZLib: {
			ZLib::StreamHandle zLibStream(ZLib::createDeflationStreamHandle());

			if(zLibStream == nullptr) {
				return nullptr;
			}

			int zLibResult = Z_OK;
			static constexpr uint64_t OUTPUT_BUFFER_SIZE = 4096;
			uint8_t outputBuffer[OUTPUT_BUFFER_SIZE];

			zLibStream->next_in = reinterpret_cast<Bytef *>(const_cast<uint8_t *>(m_data->data())) + offset;
			zLibStream->avail_in = size;
			zLibStream->next_out = outputBuffer;
			zLibStream->avail_out = OUTPUT_BUFFER_SIZE;

			while(true) {
				zLibResult = deflate(zLibStream.get(), Z_FINISH);

				if(!ZLib::isSuccess(zLibResult, "Failed to compress ZLib data")) {
					return nullptr;
				}

				if(!compressedData->writeBytes(outputBuffer, OUTPUT_BUFFER_SIZE - zLibStream->avail_out)) {
					spdlog::error("Failed to write compressed ZLib data to buffer.");
					return nullptr;
				}

				zLibStream->next_out = outputBuffer;
				zLibStream->avail_out = OUTPUT_BUFFER_SIZE;

				if(zLibResult == Z_STREAM_END) {
					return compressedData;
				}
			}

			break;
		}
		case CompressionMethod::ZStandard: {
			size_t maximumCompressedSize = ZSTD_compressBound(size);

			compressedData->resize(maximumCompressedSize);

			// TODO: Allow Zstandard compression level to be customizable:
			size_t compressedSize = ZSTD_compress(compressedData->getRawData(), compressedData->getSize(), m_data->data() + offset, size, ZSTD_CLEVEL_DEFAULT);

			if(ZSTD_isError(compressedSize)) {
				spdlog::error("Failed to compress Zstandard data.");
				return nullptr;
			}

			compressedData->resize(compressedSize);

			return compressedData;
		}
	}

	return nullptr;
}

std::string ByteBuffer::toString() const {
	return std::string(reinterpret_cast<const char *>(m_data->data()), m_data->size());
}

std::string_view ByteBuffer::toStringView() const {
	return std::string_view(reinterpret_cast<const char *>(m_data->data()), m_data->size());
}

std::string ByteBuffer::toBinary() const {
	std::stringstream binaryStream;

	for(std::vector<uint8_t>::const_iterator it = m_data->begin(); it != m_data->end(); ++it) {
		binaryStream << std::bitset<8>(*it).to_string();
	}

	return binaryStream.str();
}

std::string ByteBuffer::toHexadecimal(bool uppercase) const {
	std::string hex(m_data->size() * 2, '\0');

	uint8_t modifier = uppercase ? 0x0 : 0x20;

	for(size_t i = 0; i < m_data->size(); i++) {
		hex[ i * 2     ] = BASE_16_CHARACTERS[(*m_data)[i] >> 4]   | modifier;
		hex[(i * 2) + 1] = BASE_16_CHARACTERS[(*m_data)[i] & 0x0f] | modifier;
	}

	return hex;
}

std::string ByteBuffer::toBase64() const {
	static constexpr char PADDING_CHARACTER = '=';

	size_t length = (((m_data->size() / 3) + ((m_data->size() % 3 != 0) ? 1 : 0))) * 4;
	std::string base64(length, '\0');
	uint32_t value = 0;
	size_t i = 0;
	size_t offset = 0;

	if(m_data->size() >= 3) {
		for(; i < m_data->size() - 2; i += 3) {
			value = static_cast<uint8_t>((*m_data)[i    ]) << 16 |
					static_cast<uint8_t>((*m_data)[i + 1]) <<  8 |
					static_cast<uint8_t>((*m_data)[i + 2]);

			base64[offset++] = BASE_64_CHARACTERS[(value >> 18) & 0x3f];
			base64[offset++] = BASE_64_CHARACTERS[(value >> 12) & 0x3f];
			base64[offset++] = BASE_64_CHARACTERS[(value >> 6)  & 0x3f];
			base64[offset++] = BASE_64_CHARACTERS[(value)       & 0x3f];
		}
	}

	switch(m_data->size() % 3) {
		case 1:
			value = static_cast<uint8_t>((*m_data)[i]) << 16;

			base64[offset++] = BASE_64_CHARACTERS[(value >> 18) & 0x3f];
			base64[offset++] = BASE_64_CHARACTERS[(value >> 12) & 0x3f];
			base64[offset++] = PADDING_CHARACTER;
			base64[offset++] = PADDING_CHARACTER;
			break;

		case 2:
			value = static_cast<uint8_t>((*m_data)[i    ]) << 16 |
					static_cast<uint8_t>((*m_data)[i + 1]) <<  8;

			base64[offset++] = BASE_64_CHARACTERS[(value >> 18) & 0x3f];
			base64[offset++] = BASE_64_CHARACTERS[(value >> 12) & 0x3f];
			base64[offset++] = BASE_64_CHARACTERS[(value >> 6)  & 0x3f];
			base64[offset++] = PADDING_CHARACTER;
			break;

	}

	return base64;
}

ByteBuffer ByteBuffer::fromBinary(const std::string & binary, bool * error) {
	if(binary.length() % 8 != 0 || binary.find_first_not_of("01") != std::string::npos) {
		if(error != nullptr) {
			*error = true;
		}

		return ByteBuffer();
	}

	size_t size = binary.length() / 8;
	ByteBuffer buffer(size);
	buffer.resize(size);

	for(size_t i = 0; i < size; i++) {
		(*buffer.m_data)[i] = static_cast<uint8_t>(std::bitset<8>(binary, i * 8, 8).to_ulong());
	}

	return buffer;
}

std::optional<ByteBuffer> ByteBuffer::fromBinary(const std::string & binary) {
	bool error = false;

	ByteBuffer byteBuffer(ByteBuffer::fromBinary(binary, &error));

	if(error) {
		return {};
	}

	return byteBuffer;
}

ByteBuffer ByteBuffer::fromHexadecimal(const std::string & hexadecimal, bool * error) {
	if(hexadecimal.length() % 2 != 0) {
		if(error != nullptr) {
			*error = true;
		}

		return ByteBuffer();
	}

	size_t size = hexadecimal.length() / 2;
	uint8_t value = 0;
	uint8_t part = 0;
	uint8_t c = 0;
	size_t offset = 0;
	ByteBuffer buffer(size);
	buffer.resize(size);

	for(size_t i = 0; i < size; i += 2) {
		for(int j = 0; j < 2; j++) {
			c = static_cast<uint8_t>(hexadecimal[i + j]);

			if(c >= '0' && c <= '9') {
				part = c - '0';
			}
			else if(c >= 'A' && c <= 'Z') {
				part = c - 'A' + 10;
			}
			else if(c >= 'a' && c <= 'z') {
				part = c - 'a' + 10;
			}
			else {
				if(error != nullptr) {
					*error = true;
				}

				return ByteBuffer();
			}

			if(j == 0) {
				value = part << 4;
			}
			else {
				value |= part;
			}
		}

		(*buffer.m_data)[offset++] = value;
	}

	return buffer;
}

std::optional<ByteBuffer> ByteBuffer::fromHexadecimal(const std::string & hexadecimal) {
	bool error = false;

	ByteBuffer byteBuffer(ByteBuffer::fromHexadecimal(hexadecimal, &error));

	if(error) {
		return {};
	}

	return byteBuffer;
}

ByteBuffer ByteBuffer::fromBase64(const std::string & base64, bool * error) {
	char c = '\0';
	uint8_t part = 0;
	uint32_t value = 0;
	uint8_t bitShift = 0;
	size_t length = (base64.length() * 3) / 4;
	ByteBuffer buffer(length);

	for(size_t i = 0; i < base64.length(); i++) {
		c = base64[i];

		if(c >= 'A' && c <= 'Z') {
			part = c - 'A';
		}
		else if(c >= 'a' && c <= 'z') {
			part = c - 'a' + 26;
		}
		else if(c >= '0' && c <= '9') {
			part = c - '0' + 52;
		}
		else if(c == '+') {
			part = 62;
		}
		else if(c == '/') {
			part = 63;
		}
		else if(c != '=' || (c == '=' && i + 1 < base64.length() && base64[i + 1] != '=')) {
			if(error != nullptr) {
				*error = true;
			}

			return ByteBuffer();
		}

		if(c != '=') {
			value = (value << 6) | part;
			bitShift += 6;

			if(bitShift >= 8) {
				bitShift -= 8;
				buffer.writeByte(value >> bitShift);
				value &= (1 << bitShift) - 1;
			}
		}
	}

	return buffer;
}

std::string ByteBuffer::binaryToHexadecimal(const std::string & binary) {
	return fromBinary(binary).value_or(EMPTY_BYTE_BUFFER).toHexadecimal();
}

std::string ByteBuffer::binaryToBase64(const std::string & binary) {
	return fromBinary(binary).value_or(EMPTY_BYTE_BUFFER).toBase64();
}

std::string ByteBuffer::hexadecimalToBinary(const std::string & hexadecimal) {
	return fromHexadecimal(hexadecimal).value_or(EMPTY_BYTE_BUFFER).toBinary();
}

std::string ByteBuffer::hexadecimalToBase64(const std::string & hexadecimal) {
	return fromHexadecimal(hexadecimal).value_or(EMPTY_BYTE_BUFFER).toBase64();
}

std::string ByteBuffer::base64ToBinary(const std::string & base64) {
	return fromBase64(base64).value_or(EMPTY_BYTE_BUFFER).toBinary();
}

std::string ByteBuffer::base64ToHexadecimal(const std::string & base64) {
	return fromBase64(base64).value_or(EMPTY_BYTE_BUFFER).toHexadecimal();
}

std::optional<ByteBuffer> ByteBuffer::fromBase64(const std::string & base64) {
	bool error = false;

	ByteBuffer byteBuffer(ByteBuffer::fromBase64(base64, &error));

	if(error) {
		return {};
	}

	return byteBuffer;
}

const ByteBuffer & ByteBuffer::emptyByteBuffer() {
	return EMPTY_BYTE_BUFFER;
}

bool ByteBuffer::writeTo(const std::string & filePath, bool overwrite, bool createParentDirectories) const {
	if(!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		return false;
	}

	if(createParentDirectories) {
		std::error_code errorCode;
		Utilities::createDirectoryStructureForFilePath(filePath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to create file destination directory structure for file path '{}': {}", filePath, errorCode.message());
			return false;
		}
	}

	std::ofstream fileStream(filePath, std::ios::binary);

	if(!fileStream.is_open()) {
		return false;
	}

	fileStream.write(reinterpret_cast<const char *>(m_data->data()), m_data->size());

	fileStream.close();

	return true;
}

std::unique_ptr<ByteBuffer> ByteBuffer::readFrom(const std::string & filePath, Endianness endianness) {
	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return nullptr;
	}

	std::ifstream fileStream(filePath, std::ios::binary | std::ios::ate);

	if(!fileStream.is_open()) {
		return nullptr;
	}

	size_t size = fileStream.tellg();

	std::unique_ptr<ByteBuffer> buffer(std::make_unique<ByteBuffer>(size, endianness));
	buffer->resize(size);

	fileStream.seekg(0, std::ios::beg);
	fileStream.read(reinterpret_cast<char *>(buffer->m_data->data()), size);
	fileStream.close();

	return buffer;
}

bool ByteBuffer::checkOverflow(size_t baseSize, size_t additionalBytes) const {
	return m_data->max_size() - baseSize < additionalBytes;
}

bool ByteBuffer::autoResize(size_t baseSize, size_t additionalBytes) {
	if(checkOverflow(baseSize, additionalBytes)) {
		return false;
	}

	size_t minimumSize = baseSize + additionalBytes;

	if(minimumSize > m_data->size()) {
		resize(minimumSize, 0);
	}

	return true;
}

ByteBuffer ByteBuffer::operator + (const ByteBuffer & buffer) const {
	return operator + (buffer.getData());
}

ByteBuffer ByteBuffer::operator + (const std::vector<uint8_t> & buffer) const {
	size_t size = m_data->size() + buffer.size();

	// check for overflow
	if(size < m_data->size()) {
		return ByteBuffer();
	}

	ByteBuffer newBuffer(size);
	newBuffer.resize(size, 0);
	memcpy(newBuffer.m_data->data(), m_data->data(), m_data->size());
	memcpy(newBuffer.m_data->data(), buffer.data(), buffer.size());

	return newBuffer;
}

void ByteBuffer::operator += (const ByteBuffer & buffer) {
	writeBytes(buffer.m_data->data(), buffer.m_data->size());
}

void ByteBuffer::operator += (const std::vector<uint8_t> & data) {
	writeBytes(data.data(), data.size());
}

uint8_t ByteBuffer::operator [] (size_t index) const {
	return (*m_data)[index];
}

bool ByteBuffer::operator == (const ByteBuffer & byteBuffer) const {
	return *m_data == *byteBuffer.m_data;
}

bool ByteBuffer::operator != (const ByteBuffer & byteBuffer) const {
	return *m_data != *byteBuffer.m_data;
}
