#ifndef _TAR_XZ_ARCHIVE_H_
#define _TAR_XZ_ARCHIVE_H_

#include "CompressedTarArchive.h"

class TarXZArchive final : public CompressedTarArchive {
public:
	TarXZArchive(TarXZArchive && t) noexcept;
	TarXZArchive(const TarXZArchive & t);
	TarXZArchive & operator = (TarXZArchive && t) noexcept;
	TarXZArchive & operator = (const TarXZArchive & t);
	virtual ~TarXZArchive();

	virtual std::string getDefaultFileExtension() const override;

	static std::unique_ptr<TarXZArchive> readFrom(const std::string & filePath);
	static std::unique_ptr<TarXZArchive> createFrom(std::unique_ptr<ByteBuffer> data);

	static const std::string DEFAULT_FILE_EXTENSION;
	static const std::string ALTERNATE_FILE_EXTENSION;
	static const std::vector<std::string> FILE_EXTENSIONS;

private:
	TarXZArchive(const std::string & filePath);
	TarXZArchive(std::unique_ptr<TarArchive> tarArchive);
};

#endif // _TAR_XZ_ARCHIVE_H_
