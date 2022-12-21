#ifndef _TAR_BZIP2_ARCHIVE_H_
#define _TAR_BZIP2_ARCHIVE_H_

#include "CompressedTarArchive.h"

class TarBZip2Archive final : public CompressedTarArchive {
public:
	TarBZip2Archive(TarBZip2Archive && t) noexcept;
	TarBZip2Archive(const TarBZip2Archive & t);
	TarBZip2Archive & operator = (TarBZip2Archive && t) noexcept;
	TarBZip2Archive & operator = (const TarBZip2Archive & t);
	virtual ~TarBZip2Archive();

	virtual std::string getDefaultFileExtension() const override;

	static std::unique_ptr<TarBZip2Archive> readFrom(const std::string & filePath);
	static std::unique_ptr<TarBZip2Archive> createFrom(std::unique_ptr<ByteBuffer> data);

	static const std::string DEFAULT_FILE_EXTENSION;
	static const std::string ALTERNATE_FILE_EXTENSION;
	static const std::vector<std::string> FILE_EXTENSIONS;

private:
	TarBZip2Archive(const std::string & filePath);
	TarBZip2Archive(std::unique_ptr<TarArchive> tarArchive);
};

#endif // _TAR_BZIP2_ARCHIVE_H_
