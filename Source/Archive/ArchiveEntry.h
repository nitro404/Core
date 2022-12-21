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
	ArchiveEntry(ArchiveEntry && e) noexcept;
	ArchiveEntry(const ArchiveEntry & e);
	ArchiveEntry & operator = (ArchiveEntry && e) noexcept;
	ArchiveEntry & operator = (const ArchiveEntry & e);
	virtual ~ArchiveEntry();

	virtual bool isFile() const;
	virtual bool isDirectory() const = 0;
	bool isInSubdirectory() const;
	static bool isInSubdirectory(std::string_view path);
	std::string getName() const;
	virtual std::string getPath() const = 0;
	std::vector<std::shared_ptr<ArchiveEntry>> getChildren(bool includeSubdirectories = true, bool caseSensitive = false) const;
	virtual uint64_t getIndex() const = 0;
	virtual bool hasComment() const;
	virtual std::string getComment() const = 0;
	virtual std::chrono::time_point<std::chrono::system_clock> getDate() const = 0;
	virtual uint64_t getCompressedSize() const = 0;
	virtual uint64_t getUncompressedSize() const = 0;
	virtual std::unique_ptr<ByteBuffer> getData() const = 0;
	virtual uint32_t getCRC32() const = 0;
	virtual bool writeTo(const std::string & directoryPath, bool overwrite = false) const = 0;

protected:
	virtual bool isParentArchiveValid() const;
	virtual Archive * getParentArchive() const = 0;
	virtual bool setParentArchive(Archive * archive) = 0;
	void clearParentArchive();
};

#endif // _ARCHIVE_ENTRTY_H_
