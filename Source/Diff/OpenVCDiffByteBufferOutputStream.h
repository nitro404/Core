#ifndef _OPEN_VCDIFF_BYTE_BUFFER_OUTPUT_STREAM_H_
#define _OPEN_VCDIFF_BYTE_BUFFER_OUTPUT_STREAM_H_

#include "ByteBuffer.h"

#include <cstdint>
#include <memory>

class OpenVCDiffByteBufferOutputStream final {
public:
	OpenVCDiffByteBufferOutputStream();
	std::unique_ptr<ByteBuffer> transferData();
	OpenVCDiffByteBufferOutputStream& append(const char * data, size_t size);
	void clear();
	void push_back(char value);
	void reserve(size_t numberOfBytes);
	size_t size() const;

private:
	std::unique_ptr<ByteBuffer> m_data;
};

#endif // _OPEN_VCDIFF_BYTE_BUFFER_OUTPUT_STREAM_H_
