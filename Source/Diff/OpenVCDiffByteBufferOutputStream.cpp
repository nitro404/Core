#include "OpenVCDiffByteBufferOutputStream.h"

OpenVCDiffByteBufferOutputStream::OpenVCDiffByteBufferOutputStream()
	: m_data(std::make_unique<ByteBuffer>()) { }

std::unique_ptr<ByteBuffer> OpenVCDiffByteBufferOutputStream::transferData() {
	return std::move(m_data);
}

OpenVCDiffByteBufferOutputStream& OpenVCDiffByteBufferOutputStream::append(const char * data, size_t size) {
	m_data->writeBytes(reinterpret_cast<const uint8_t *>(data), size);

	return *this;
}

void OpenVCDiffByteBufferOutputStream::clear() {
	m_data->clear();
}

void OpenVCDiffByteBufferOutputStream::push_back(char value) {
	m_data->writeByte(static_cast<uint8_t>(value));
}

void OpenVCDiffByteBufferOutputStream::reserve(size_t numberOfBytes) {
	m_data->reserve(numberOfBytes);
}

size_t OpenVCDiffByteBufferOutputStream::size() const {
	return m_data->getSize();
}
