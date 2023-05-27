#include "ArchiveBufferInputStream.h"

ArchiveBufferInputStream::ArchiveBufferInputStream(std::unique_ptr<ByteBuffer> data)
	: m_data(std::move(data)) { }

ArchiveBufferInputStream::~ArchiveBufferInputStream() { }

STDMETHODIMP ArchiveBufferInputStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) {
	if(m_data == nullptr) {
		return E_FAIL;
	}

	switch(seekOrigin) {
		case STREAM_SEEK_SET: {
			m_data->setReadOffset(offset);
			break;
		}
		case STREAM_SEEK_CUR: {
			m_data->setReadOffset(m_data->getReadOffset() + offset);
			break;
		}
		case STREAM_SEEK_END: {
			m_data->setReadOffset(m_data->getSize() + offset);
			break;
		}
		default: {
			return STG_E_INVALIDFUNCTION;
		}
	}

	if(newPosition != nullptr) {
		*newPosition = m_data->getReadOffset();
	}

	return S_OK;
}

STDMETHODIMP ArchiveBufferInputStream::Read(void * data, UInt32 size, UInt32 * processedSize) {
	if(m_data == nullptr) {
		return E_FAIL;
	}

	if(data == nullptr) {
		return E_INVALIDARG;
	}

	size_t numberOfBytesRead = std::min(static_cast<size_t>(size), m_data->numberOfBytesRemaining());
	std::memcpy(data, m_data->getRawData() + m_data->getReadOffset(), numberOfBytesRead);
	m_data->skipReadBytes(numberOfBytesRead);

	if(processedSize != nullptr) {
		*processedSize = static_cast<UInt32>(numberOfBytesRead);
	}

	return S_OK;
}
