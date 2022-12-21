#ifndef _COMPRESSED_TAR_ARCHIVE_H_
#define _COMPRESSED_TAR_ARCHIVE_H_

#include "TarArchive.h"

class CompressedTarArchive : public TarArchive {
public:
	CompressedTarArchive(CompressedTarArchive && t) noexcept;
	CompressedTarArchive(const CompressedTarArchive & t);
	CompressedTarArchive & operator = (CompressedTarArchive && t) noexcept;
	CompressedTarArchive & operator = (const CompressedTarArchive & t);
	virtual ~CompressedTarArchive();

	virtual uint64_t getCompressedSize() const override;

	ByteBuffer::CompressionMethod getCompressionMethod() const;

protected:
	CompressedTarArchive(const std::string & filePath, ByteBuffer::CompressionMethod compressionMethod);

protected:
	uint64_t m_compressedSize;

private:
	ByteBuffer::CompressionMethod m_compressionMethod;
};

#endif // _COMPRESSED_TAR_ARCHIVE_H_
