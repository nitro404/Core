#ifndef _ARCHIVE_H_
#define _ARCHIVE_H_

#include "ArchiveEntry.h"
#include "ByteBuffer.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Archive {
	friend class ArchiveEntry;

public:
	enum class Type {
		NSIS,
		Rar,
		SevenZip,
		Tar,
		Zip
	};

	Archive(Type type);
	Archive(Archive && a) noexcept;
	Archive(const Archive & a);
	Archive & operator = (Archive && a) noexcept;
	Archive & operator = (const Archive & a);
	virtual ~Archive();

	Type getType() const;
	virtual std::string getDefaultFileExtension() const = 0;
	virtual bool isOpen() const;
	virtual bool isModifiable() const;
	virtual std::string getFilePath() const = 0;
	virtual bool hasComment() const = 0;
	virtual std::string getComment() const = 0;
	virtual uint64_t getCompressedSize() const;
	virtual uint64_t getUncompressedSize() const;
	virtual size_t numberOfEntries() const;
	virtual size_t numberOfFiles() const;
	virtual size_t numberOfDirectories() const;
	bool hasEntry(const ArchiveEntry & entry) const;
	bool hasEntry(const std::string & entryPath, bool caseSensitive = false) const;
	bool hasEntryWithName(const std::string & entryName, bool includeSubdirectories = true, bool caseSensitive = false) const;
	size_t indexOfEntry(const std::string & entryPath, bool caseSensitive = false) const;
	size_t indexOfFirstEntryWithName(const std::string & entryName, bool includeSubdirectories = true, bool caseSensitive = false) const;
	const std::shared_ptr<ArchiveEntry> getEntry(const std::string & entryPath, bool caseSensitive = false) const;
	std::shared_ptr<ArchiveEntry> getEntry(const std::string & entryPath, bool caseSensitive = false);
	std::shared_ptr<ArchiveEntry> getFirstEntryWithName(const std::string & entryName, bool includeSubdirectories = true, bool caseSensitive = false) const;
	const std::shared_ptr<ArchiveEntry> getEntry(size_t index) const;
	std::shared_ptr<ArchiveEntry> getEntry(size_t index);
	virtual std::vector<std::shared_ptr<ArchiveEntry>> getEntries() const = 0;
	size_t extractAllEntries(const std::string & directoryPath, bool overwrite = false) const;
	void updateParentArchive();
	virtual std::string toDebugString(bool includeDate = false) const = 0;

private:
	Type m_type;
};

#endif // _ARCHIVE_H_
