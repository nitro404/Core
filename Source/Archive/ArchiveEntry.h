#ifndef _ARCHIVE_ENTRTY_H_
#define _ARCHIVE_ENTRTY_H_

#include "ByteBuffer.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Archive;

class ArchiveEntry {
	friend class Archive;

public:
	ArchiveEntry();
	virtual ~ArchiveEntry();

	virtual bool isFile() const;
	virtual bool isDirectory() const;
	bool isInSubdirectory() const;
	static bool isInSubdirectory(std::string_view path);
	std::string getName() const;
	virtual std::string getPath() const = 0;
	std::string getBasePath() const;
	bool hasFileExtension(std::string_view fileExtension) const;
	std::string getFileExtension(bool useLastPeriod = true) const;
	std::vector<std::shared_ptr<ArchiveEntry>> getChildren(bool includeSubdirectories = true, bool caseSensitive = false) const;
	virtual uint64_t getIndex() const = 0;
	virtual bool hasComment() const;
	virtual std::string getComment() const = 0;
	virtual std::chrono::time_point<std::chrono::system_clock> getDate() const = 0;
	virtual uint64_t getCompressedSize() const = 0;
	virtual uint64_t getUncompressedSize() const = 0;
	virtual std::unique_ptr<ByteBuffer> getData() const = 0;
	virtual uint32_t getCRC32() const = 0;
	bool writeToDirectory(const std::string & directoryPath, bool overwrite = false);
	virtual bool writeToFile(const std::string & filePath, bool overwrite = false) = 0;

	static bool isDirectory(std::string_view path);

protected:
	virtual bool isParentArchiveValid() const;
	virtual Archive * getParentArchive() const = 0;
	virtual bool setParentArchive(Archive * archive) = 0;
	void clearParentArchive();
};

#endif // _ARCHIVE_ENTRTY_H_
