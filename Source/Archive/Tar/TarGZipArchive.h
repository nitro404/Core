#ifndef _TAR_GZIP_ARCHIVE_H_
#define _TAR_GZIP_ARCHIVE_H_

#include "CompressedTarArchive.h"

class TarGZipArchive final : public CompressedTarArchive {
public:
	TarGZipArchive(TarGZipArchive && t) noexcept;
	TarGZipArchive(const TarGZipArchive & t);
	TarGZipArchive & operator = (TarGZipArchive && t) noexcept;
	TarGZipArchive & operator = (const TarGZipArchive & t);
	~TarGZipArchive() override;

	virtual std::string getDefaultFileExtension() const override;

	static std::unique_ptr<TarGZipArchive> readFrom(const std::string & filePath);
	static std::unique_ptr<TarGZipArchive> createFrom(std::unique_ptr<ByteBuffer> data);

	static const std::string DEFAULT_FILE_EXTENSION;
	static const std::string ALTERNATE_FILE_EXTENSION;
	static const std::vector<std::string> FILE_EXTENSIONS;

private:
	TarGZipArchive(const std::string & filePath);
	TarGZipArchive(std::unique_ptr<TarArchive> tarArchive);
};

#endif // _TAR_GZIP_ARCHIVE_H_
