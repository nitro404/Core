#include "ZipArchive.h"

#include "ZipUtilities.h"

#include <spdlog/spdlog.h>

ZipArchive::SourceBuffer::SourceBuffer(ZipSourceHandle zipSourceHandle, std::unique_ptr<ByteBuffer> data)
	: m_sourceHandle(std::move(zipSourceHandle))
	, m_data(std::move(data)) { }

ZipArchive::SourceBuffer::~SourceBuffer() = default;

zip_source * ZipArchive::SourceBuffer::getRawSourceHandle() const {
	return m_sourceHandle.get();
}

const ByteBuffer * ZipArchive::SourceBuffer::getData() const {
	return m_data.get();
}

ByteBuffer * ZipArchive::SourceBuffer::getData() {
	return m_data.get();
}

const uint8_t * ZipArchive::SourceBuffer::getRawData() const {
	if(m_data == nullptr) {
		return 0;
	}

	return m_data->getRawData();
}

size_t ZipArchive::SourceBuffer::getSize() const {
	if(m_data == nullptr) {
		return 0;
	}

	return m_data->getSize();
}

void ZipArchive::SourceBuffer::deleteSourceHandle() {
	m_sourceHandle.reset();
}

void ZipArchive::SourceBuffer::deleteDataBuffer() {
	m_data.reset();
}

zip_source * ZipArchive::SourceBuffer::releaseSourceHandle() {
	return m_sourceHandle.release();
}

ByteBuffer * ZipArchive::SourceBuffer::releaseDataBuffer() {
	return m_data.release();
}

ZipArchive::ZipSourceHandle ZipArchive::SourceBuffer::transferSourceHandle() {
	return std::move(m_sourceHandle);
}

std::unique_ptr<ByteBuffer> ZipArchive::SourceBuffer::transferDataBuffer() {
	return std::move(m_data);
}

bool ZipArchive::SourceBuffer::isOpen() const {
	return m_sourceHandle != nullptr && m_data != nullptr;
}

bool ZipArchive::SourceBuffer::close() {
	if(!isOpen()) {
		return false;
	}

	zip_stat_t zipSourceInfo;
	zip_stat_init(&zipSourceInfo);

	if(!ZipUtilities::isSuccess(zip_source_stat(m_sourceHandle.get(), &zipSourceInfo), "Failed to retrieve updated zip archive source buffer information.")) {
		return false;
	}

	if(!ZipUtilities::isSuccess(zip_source_open(m_sourceHandle.get()), "Failed to open zip archive source buffer to read updated data.")) {
		return false;
	}

	m_data->resize(zipSourceInfo.size);
	int64_t numberOfBytesRead = zip_source_read(m_sourceHandle.get(), m_data->getRawData(), m_data->getSize());

	if(!ZipUtilities::isSuccess(zip_source_close(m_sourceHandle.get()), "Failed to close zip archive source buffer after reading updated data.")) {
		return false;
	}

	if(numberOfBytesRead == -1) {
		spdlog::error("Failed to write updated zip archive source buffer data to byte buffer.");
		return false;
	}

	if(numberOfBytesRead != zipSourceInfo.size) {
		spdlog::error("Failed to write updated zip archive source buffer data to buffer, read only {} bytes from source buffer when {} bytes were expected.", numberOfBytesRead, zipSourceInfo.size);
		return false;
	}

	// source handle is no longer valid
	m_sourceHandle.reset();

	return true;
}

bool ZipArchive::SourceBuffer::reopen() {
	if(isOpen() || m_data == nullptr) {
		return false;
	}

	ZipUtilities::ZipErrorHandle zipError(ZipUtilities::createZipErrorHandle());

	m_sourceHandle = createZipSourceHandle(zip_source_buffer_create(m_data->getRawData(), m_data->getSize(), 0, zipError.get()));

	if(m_sourceHandle == nullptr) {
		spdlog::error("Failed to re-open zip archive source buffer. {}", zip_error_strerror(zipError.get()));
		return false;
	}

	return true;
}
