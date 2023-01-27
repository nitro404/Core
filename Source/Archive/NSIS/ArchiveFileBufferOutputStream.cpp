#include "ArchiveFileBufferOutputStream.h"

ArchiveFileBufferOutputStream::ArchiveFileBufferOutputStream(ByteBuffer * data)
	: m_data(data) { }

ArchiveFileBufferOutputStream::~ArchiveFileBufferOutputStream() { }

ByteBuffer * ArchiveFileBufferOutputStream::getData() {
	return m_data;
}

const ByteBuffer * ArchiveFileBufferOutputStream::getData() const {
	return m_data;
}

STDMETHODIMP ArchiveFileBufferOutputStream::Write(const void * data, UInt32 size, UInt32 * processedSize) {
	if(m_data == nullptr || !m_data->writeBytes(reinterpret_cast<const uint8_t *>(data), size)) {
		return E_FAIL;
	}

	if(processedSize) {
		*processedSize = size;
	}

	return S_OK;
}

STDMETHODIMP ArchiveFileBufferOutputStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) {
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

STDMETHODIMP ArchiveFileBufferOutputStream::SetSize(UInt64 newSize) {
	if(m_data == nullptr) {
		return E_FAIL;
	}

	m_data->reserve(newSize);

	return S_OK;
}
