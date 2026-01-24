#ifndef _TAR_ZSTANDARD_ARCHIVE_H_
#define _TAR_ZSTANDARD_ARCHIVE_H_

#include "CompressedTarArchive.h"

class TarZStandardArchive final : public CompressedTarArchive {
public:
	TarZStandardArchive(TarZStandardArchive && t) noexcept;
	TarZStandardArchive(const TarZStandardArchive & t);
	TarZStandardArchive & operator = (TarZStandardArchive && t) noexcept;
	TarZStandardArchive & operator = (const TarZStandardArchive & t);
	~TarZStandardArchive() override;

	virtual std::string getDefaultFileExtension() const override;

	static std::unique_ptr<TarZStandardArchive> readFrom(const std::string & filePath);
	static std::unique_ptr<TarZStandardArchive> createFrom(std::unique_ptr<ByteBuffer> data);

	static const std::string DEFAULT_FILE_EXTENSION;
	static const std::string ALTERNATE_FILE_EXTENSION;
	static const std::vector<std::string> FILE_EXTENSIONS;

private:
	TarZStandardArchive(const std::string & filePath);
	TarZStandardArchive(std::unique_ptr<TarArchive> tarArchive);
};

#endif // _TAR_ZSTANDARD_ARCHIVE_H_
