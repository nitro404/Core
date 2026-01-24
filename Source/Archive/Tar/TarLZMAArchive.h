#ifndef _TAR_LZMA_ARCHIVE_H_
#define _TAR_LZMA_ARCHIVE_H_

#include "CompressedTarArchive.h"

class TarLZMAArchive final : public CompressedTarArchive {
public:
	TarLZMAArchive(TarLZMAArchive && t) noexcept;
	TarLZMAArchive(const TarLZMAArchive & t);
	TarLZMAArchive & operator = (TarLZMAArchive && t) noexcept;
	TarLZMAArchive & operator = (const TarLZMAArchive & t);
	~TarLZMAArchive() override;

	virtual std::string getDefaultFileExtension() const override;

	static std::unique_ptr<TarLZMAArchive> readFrom(const std::string & filePath);
	static std::unique_ptr<TarLZMAArchive> createFrom(std::unique_ptr<ByteBuffer> data);

	static const std::string DEFAULT_FILE_EXTENSION;
	static const std::string ALTERNATE_FILE_EXTENSION;
	static const std::vector<std::string> FILE_EXTENSIONS;

private:
	TarLZMAArchive(const std::string & filePath);
	TarLZMAArchive(std::unique_ptr<TarArchive> tarArchive);
};

#endif // _TAR_LZMA_ARCHIVE_H_
