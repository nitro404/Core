#ifndef _BYTE_BUFFER_H_
#define _BYTE_BUFFER_H_

#include "Endianness.h"

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class ByteBuffer final {
public:
	enum class HashType {
		MD5,
		SHA1,
		SHA256,
		SHA512
	};

	enum class HashFormat {
		Hexadecimal,
		Base64
	};

	enum class CompressionMethod {
		BZip2,
		LZMA,
		XZ,
		ZLib,
		ZStandard
	};

	ByteBuffer(Endianness endianness = DEFAULT_ENDIANNESS);
	ByteBuffer(size_t initialCapacity, Endianness endianness = DEFAULT_ENDIANNESS);
	ByteBuffer(const uint8_t * data, size_t size, Endianness endianness = DEFAULT_ENDIANNESS);
	template <size_t N>
	ByteBuffer(const std::array<uint8_t, N> & data, Endianness endianness = DEFAULT_ENDIANNESS);
	ByteBuffer(const std::vector<uint8_t> & data, Endianness endianness = DEFAULT_ENDIANNESS);
	ByteBuffer(std::unique_ptr<std::vector<uint8_t>> data, Endianness endianness = DEFAULT_ENDIANNESS);
	ByteBuffer(const std::string & data, Endianness endianness = DEFAULT_ENDIANNESS);
	ByteBuffer(ByteBuffer && buffer) noexcept;
	ByteBuffer(const ByteBuffer & buffer);
	ByteBuffer & operator = (const char * data);
	template <size_t N>
	ByteBuffer & operator = (const std::array<uint8_t, N> & data);
	ByteBuffer & operator = (const std::vector<uint8_t> & data);
	ByteBuffer & operator = (const std::string & data);
	ByteBuffer & operator = (ByteBuffer && buffer) noexcept;
	ByteBuffer & operator = (const ByteBuffer & buffer);
	~ByteBuffer();

	const std::vector<uint8_t> & getData() const;
	std::vector<uint8_t> & getData();
	const uint8_t * getRawData() const;
	uint8_t * getRawData();
	void setData(const uint8_t * data, size_t size);
	template <size_t N>
	void setData(const std::array<uint8_t, N> & data);
	void setData(const std::vector<uint8_t> & data);
	void setData(const std::string & data);
	void setData(const ByteBuffer & buffer);
	bool isEmpty() const;
	bool isNotEmpty() const;
	bool isFull() const;
	size_t getSize() const;
	size_t getCapacity() const;
	void resize(size_t size, uint8_t value = 0);
	bool reserve(size_t capacity);
	void shrinkToFit();
	Endianness getEndianness() const;
	void setEndianness(Endianness endianness) const;
	void fill(uint8_t value, size_t start = 0, size_t end = std::numeric_limits<size_t>::max());
	void reverse(size_t start = 0, size_t end = std::numeric_limits<size_t>::max());
	void clear();
	std::string getMD5(HashFormat hashFormat = DEFAULT_HASH_FORMAT) const;
	std::string getSHA1(HashFormat hashFormat = DEFAULT_HASH_FORMAT) const;
	std::string getSHA256(HashFormat hashFormat = DEFAULT_HASH_FORMAT) const;
	std::string getSHA512(HashFormat hashFormat = DEFAULT_HASH_FORMAT) const;
	template <class A>
	std::string getHash(HashFormat hashFormat = DEFAULT_HASH_FORMAT) const;
	std::string getHash(HashType hashType, HashFormat hashFormat = DEFAULT_HASH_FORMAT) const;

	size_t getReadOffset() const;
	void setReadOffset(size_t offset) const;
	bool canReadBytes(size_t numberOfBytes) const;
	bool skipReadBytes(size_t numberOfBytes) const;
	size_t numberOfBytesRemaining() const;
	std::unique_ptr<ByteBuffer> getRemainingBytes() const;
	bool isEndOfBuffer() const;
	void resetReadOffset() const;

	size_t getWriteOffset() const;
	void setWriteOffset(size_t offset) const;
	void resetWriteOffset() const;
	bool skipWriteBytes(size_t numberOfBytes) const;

	int8_t getByte(size_t offset, bool * error) const;
	std::optional<int8_t> getByte(size_t offset) const;
	uint8_t getUnsignedByte(size_t offset, bool * error) const;
	std::optional<uint8_t> getUnsignedByte(size_t offset) const;
	int16_t getShort(size_t offset, bool * error) const;
	std::optional<int16_t> getShort(size_t offset) const;
	uint16_t getUnsignedShort(size_t offset, bool * error) const;
	std::optional<uint16_t> getUnsignedShort(size_t offset) const;
	int32_t getInteger(size_t offset, bool * error) const;
	std::optional<int32_t> getInteger(size_t offset) const;
	uint32_t getUnsignedInteger(size_t offset, bool * error) const;
	std::optional<uint32_t> getUnsignedInteger(size_t offset) const;
	int64_t getLong(size_t offset, bool * error) const;
	std::optional<int64_t> getLong(size_t offset) const;
	uint64_t getUnsignedLong(size_t offset, bool * error) const;
	std::optional<uint64_t> getUnsignedLong(size_t offset) const;
	float getFloat(size_t offset, bool * error) const;
	std::optional<float> getFloat(size_t offset) const;
	double getDouble(size_t offset, bool * error) const;
	std::optional<double> getDouble(size_t offset) const;
	std::string getString(size_t length, size_t offset, bool * error) const;
	std::optional<std::string> getString(size_t length, size_t offset) const;
	std::string getNullTerminatedString(size_t offset, bool * error) const;
	std::optional<std::string> getNullTerminatedString(size_t offset) const;
	template <size_t N>
	std::array<uint8_t, N> getBytes(size_t offset, bool * error) const;
	template <size_t N>
	std::optional<std::array<uint8_t, N>> getBytes(size_t offset) const;
	std::vector<uint8_t> getBytes(size_t numberOfBytes, size_t offset, bool * error) const;
	std::optional<std::vector<uint8_t>> getBytes(size_t numberOfBytes, size_t offset) const;

	int8_t readByte(bool * error) const;
	std::optional<int8_t> readByte() const;
	uint8_t readUnsignedByte(bool * error) const;
	std::optional<uint8_t> readUnsignedByte() const;
	int16_t readShort(bool * error) const;
	std::optional<int16_t> readShort() const;
	uint16_t readUnsignedShort(bool * error) const;
	std::optional<uint16_t> readUnsignedShort() const;
	int32_t readInteger(bool * error) const;
	std::optional<int32_t> readInteger() const;
	uint32_t readUnsignedInteger(bool * error) const;
	std::optional<uint32_t> readUnsignedInteger() const;
	int64_t readLong(bool * error) const;
	std::optional<int64_t> readLong() const;
	uint64_t readUnsignedLong(bool * error) const;
	std::optional<uint64_t> readUnsignedLong() const;
	float readFloat(bool * error) const;
	std::optional<float> readFloat() const;
	double readDouble(bool * error) const;
	std::optional<double> readDouble() const;
	std::string readString(size_t length, bool * error) const;
	std::optional<std::string> readString(size_t length) const;
	std::string readNullTerminatedString(bool * error) const;
	std::optional<std::string> readNullTerminatedString() const;
	template <size_t N>
	std::array<uint8_t, N> readBytes(bool * error) const;
	template <size_t N>
	std::optional<std::array<uint8_t, N>> readBytes() const;
	std::vector<uint8_t> readBytes(size_t numberOfBytes, bool * error) const;
	std::optional<std::vector<uint8_t>> readBytes(size_t numberOfBytes) const;

	bool putByte(int8_t value, size_t offset);
	bool putUnsignedByte(uint8_t value, size_t offset);
	bool putShort(int16_t value, size_t offset);
	bool putUnsignedShort(uint16_t value, size_t offset);
	bool putInteger(int32_t value, size_t offset);
	bool putUnsignedInteger(uint32_t value, size_t offset);
	bool putLong(int64_t value, size_t offset);
	bool putUnsignedLong(uint64_t value, size_t offset);
	bool putFloat(float value, size_t offset);
	bool putDouble(double value, size_t offset);
	bool putString(const std::string & value, size_t offset);
	bool putNullTerminatedString(const std::string & value, size_t offset);
	bool putBytes(const uint8_t * data, size_t size, size_t offset);
	template <size_t N>
	bool putBytes(const std::array<uint8_t, N> data, size_t offset);
	bool putBytes(const std::vector<uint8_t> data, size_t offset);
	bool putBytes(const ByteBuffer & buffer, size_t offset);

	bool insertByte(int8_t value, size_t offset);
	bool insertUnsignedByte(uint8_t value, size_t offset);
	bool insertShort(int16_t value, size_t offset);
	bool insertUnsignedShort(uint16_t value, size_t offset);
	bool insertInteger(int32_t value, size_t offset);
	bool insertUnsignedInteger(uint32_t value, size_t offset);
	bool insertLong(int64_t value, size_t offset);
	bool insertUnsignedLong(uint64_t value, size_t offset);
	bool insertFloat(float value, size_t offset);
	bool insertDouble(double value, size_t offset);
	bool insertString(const std::string & value, size_t offset);
	bool insertNullTerminatedString(const std::string & value, size_t offset);
	bool insertBytes(const uint8_t * data, size_t size, size_t offset);
	template <size_t N>
	bool insertBytes(const std::array<uint8_t, N> data, size_t offset);
	bool insertBytes(const std::vector<uint8_t> data, size_t offset);
	bool insertBytes(const ByteBuffer & buffer, size_t offset);

	bool writeByte(int8_t value);
	bool writeUnsignedByte(uint8_t value);
	bool writeShort(int16_t value);
	bool writeUnsignedShort(uint16_t value);
	bool writeInteger(int32_t value);
	bool writeUnsignedInteger(uint32_t value);
	bool writeLong(int64_t value);
	bool writeUnsignedLong(uint64_t value);
	bool writeFloat(float value);
	bool writeDouble(double value);
	bool writeString(const std::string & value);
	bool writeNullTerminatedString(const std::string & value);
	bool writeBytes(const uint8_t * data, size_t size);
	template <size_t N>
	bool writeBytes(const std::array<uint8_t, N> data);
	bool writeBytes(const std::vector<uint8_t> data);
	bool writeBytes(const ByteBuffer & buffer);

	std::unique_ptr<ByteBuffer> clone() const;
	std::unique_ptr<ByteBuffer> copyOfRange(size_t start, size_t end) const;
	std::unique_ptr<ByteBuffer> decompressed(CompressionMethod compressionMethod, size_t offset = std::numeric_limits<size_t>::max(), size_t size = std::numeric_limits<size_t>::max()) const;
	std::unique_ptr<ByteBuffer> compressed(CompressionMethod compressionMethod, size_t offset = std::numeric_limits<size_t>::max(), size_t size = std::numeric_limits<size_t>::max()) const;
	std::string toString() const;
	std::string_view toStringView() const;
	std::string toBinary() const;
	std::string toHexadecimal(bool uppercase = false) const;
	std::string toBase64() const;

	static ByteBuffer fromBinary(const std::string & binary, bool * error);
	static std::optional<ByteBuffer> fromBinary(const std::string & binary);
	static ByteBuffer fromHexadecimal(const std::string & hexadecimal, bool * error);
	static std::optional<ByteBuffer> fromHexadecimal(const std::string & hexadecimal);
	static ByteBuffer fromBase64(const std::string & base64, bool * error);
	static std::optional<ByteBuffer> fromBase64(const std::string & base64);
	static std::string binaryToHexadecimal(const std::string & binary);
	static std::string binaryToBase64(const std::string & binary);
	static std::string hexadecimalToBinary(const std::string & hexadecimal);
	static std::string hexadecimalToBase64(const std::string & hexadecimal);
	static std::string base64ToBinary(const std::string & base64);
	static std::string base64ToHexadecimal(const std::string & base64);
	static const ByteBuffer & emptyByteBuffer();

	bool writeTo(const std::string & filePath, bool overwrite = false, bool createParentDirectories = true) const;
	static std::unique_ptr<ByteBuffer> readFrom(const std::string & filePath, Endianness endianness = DEFAULT_ENDIANNESS);

	ByteBuffer operator + (const ByteBuffer & buffer) const;
	ByteBuffer operator + (const std::vector<uint8_t> & buffer) const;
	void operator += (const ByteBuffer & buffer);
	void operator += (const std::vector<uint8_t> & data);
	uint8_t operator [] (size_t index) const;

	bool operator == (const ByteBuffer & byteBuffer) const;
	bool operator != (const ByteBuffer & byteBuffer) const;

	static const Endianness DEFAULT_ENDIANNESS;
	static const HashFormat DEFAULT_HASH_FORMAT;
	static const ByteBuffer EMPTY_BYTE_BUFFER;

private:
	bool checkOverflow(size_t baseSize, size_t additionalBytes) const;
	bool autoResize(size_t baseSize, size_t additionalBytes);

	std::unique_ptr<std::vector<uint8_t>> m_data;
	mutable Endianness m_endianness;
	mutable size_t m_readOffset;
	mutable size_t m_writeOffset;
};

template <size_t N>
ByteBuffer::ByteBuffer(const std::array<uint8_t, N> & data, Endianness endianness)
	: m_data(std::make_unique<std::vector<uint8_t>(data))
	, m_endianness(endianness)
	, m_readOffset(0)
	, m_writeOffset(data.size()) { }

template <size_t N>
ByteBuffer & ByteBuffer::operator = (const std::array<uint8_t, N> & data) {
	m_data = std::make_unique<std::vector<uint8_t>(data);
	m_readOffset = 0;
	m_writeOffset = 0;

	return *this;
}

template <size_t N>
void ByteBuffer::setData(const std::array<uint8_t, N> & data) {
	m_data = std::make_unique<std::vector<uint8_t>(data);
	m_readOffset = 0;
	m_writeOffset = 0;
}

template <class A>
std::string ByteBuffer::getHash(HashFormat hashFormat) const {
	if(isEmpty()) {
		return {};
	}

	A hash;
	hash.Update(m_data->data(), m_data->size());
	ByteBuffer digest(hash.DigestSize());
	digest.resize(hash.DigestSize());
	hash.Final(digest.getRawData());

	switch(hashFormat) {
		case HashFormat::Hexadecimal: {
			return digest.toHexadecimal();
		}
		case HashFormat::Base64: {
			return digest.toBase64();
		}
	}

	return {};
}

template <size_t N>
std::array<uint8_t, N> ByteBuffer::getBytes(size_t offset, bool * error) const {
	if(offset + (N * sizeof(uint8_t)) > m_data->size()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	std::array<uint8_t, N> bytes;
	std::vector<uint8_t>::const_iterator dataStart(m_data->begin() + (offset * sizeof(uint8_t)));
	std::copy(dataStart, dataStart + (N * sizeof(uint8_t)), bytes.begin());

	return bytes;
}

template <size_t N>
std::optional<std::array<uint8_t, N>> ByteBuffer::getBytes(size_t offset) const {
	bool error = false;

	std::array<uint8_t, N> value(getBytes<N>(offset, &error));

	if(error) {
		return {};
	}

	return value;
}

template <size_t N>
std::array<uint8_t, N> ByteBuffer::readBytes(bool * error) const {
	bool e = false;
	std::array<uint8_t, N> value(getBytes<N>(m_readOffset, &e));

	if(e) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		m_readOffset += N * sizeof(uint8_t);
	}

	return value;
}

template <size_t N>
std::optional<std::array<uint8_t, N>> ByteBuffer::readBytes() const {
	bool error = false;

	std::vector<uint8_t, N> value(readBytes<N>(&error));

	if(error) {
		return {};
	}

	return value;
}

template <size_t N>
bool ByteBuffer::putBytes(const std::array<uint8_t, N> data, size_t offset) {
	return putBytes(data.data(), data.size(), offset);
}

template <size_t N>
bool ByteBuffer::insertBytes(const std::array<uint8_t, N> data, size_t offset) {
	return insertBytes(data.data(), data.size(), offset);
}

template <size_t N>
bool ByteBuffer::writeBytes(const std::array<uint8_t, N> data) {
	if(putBytes<N>(data, m_writeOffset)) {
		m_writeOffset += data.size() * sizeof(uint8_t);

		return true;
	}

	return false;
}

#endif // _BYTE_BUFFER_H_
