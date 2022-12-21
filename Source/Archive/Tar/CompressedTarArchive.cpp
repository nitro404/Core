#include "CompressedTarArchive.h"

CompressedTarArchive::CompressedTarArchive(const std::string & filePath, ByteBuffer::CompressionMethod compressionMethod)
	: TarArchive(filePath)
	, m_compressedSize(0)
	, m_compressionMethod(compressionMethod) { }

CompressedTarArchive::CompressedTarArchive(CompressedTarArchive && t) noexcept
	: TarArchive(t)
	, m_compressedSize(t.m_compressedSize)
	, m_compressionMethod(t.m_compressionMethod) { }

CompressedTarArchive::CompressedTarArchive(const CompressedTarArchive & t)
	: TarArchive(t)
	, m_compressedSize(t.m_compressedSize)
	, m_compressionMethod(t.m_compressionMethod) { }

CompressedTarArchive & CompressedTarArchive::operator = (CompressedTarArchive && t) noexcept {
	if(this != &t) {
		TarArchive::operator = (t);

		m_compressedSize = t.m_compressedSize;
		m_compressionMethod = t.m_compressionMethod;
	}

	return *this;
}

CompressedTarArchive & CompressedTarArchive::operator = (const CompressedTarArchive & t) {
	TarArchive::operator = (t);

	m_compressedSize = t.m_compressedSize;
	m_compressionMethod = t.m_compressionMethod;

	return *this;
}

CompressedTarArchive::~CompressedTarArchive() { }

uint64_t CompressedTarArchive::getCompressedSize() const {
	return m_compressedSize;
}

ByteBuffer::CompressionMethod CompressedTarArchive::getCompressionMethod() const {
	return m_compressionMethod;
}
