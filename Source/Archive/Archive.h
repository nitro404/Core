#ifndef _ARCHIVE_H_
#define _ARCHIVE_H_

#include "ArchiveEntry.h"
#include "ByteBuffer.h"
#include "Utilities/StringUtilities.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class ArchiveFactoryRegistry;

class Archive {
	friend class ArchiveEntry;
	friend class ArchiveFactoryRegistry;

public:
	enum class Type : uint8_t {
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
	bool hasEntryWithExtension(const std::string & extension, bool includeSubdirectories = true, bool caseSensitive = false) const;
	bool hasEntryWithExtension(const std::vector<std::string> & extensions, bool includeSubdirectories = true, bool caseSensitive = false) const;
	size_t indexOfEntry(const std::string & entryPath, bool caseSensitive = false) const;
	size_t indexOfFirstEntryWithName(const std::string & entryName, bool includeSubdirectories = true, bool caseSensitive = false) const;
	size_t indexOfLastEntryWithName(const std::string & entryName, bool includeSubdirectories = true, bool caseSensitive = false) const;
	size_t indexOfFirstEntryWithExtension(const std::string & extension, bool includeSubdirectories = true, bool caseSensitive = false) const;
	size_t indexOfFirstEntryWithExtension(const std::vector<std::string> & extensions, bool includeSubdirectories = true, bool caseSensitive = false) const;
	size_t indexOfLastEntryWithExtension(const std::string & extension, bool includeSubdirectories = true, bool caseSensitive = false) const;
	size_t indexOfLastEntryWithExtension(const std::vector<std::string> & extensions, bool includeSubdirectories = true, bool caseSensitive = false) const;
	const std::shared_ptr<ArchiveEntry> getEntry(const std::string & entryPath, bool caseSensitive = false) const;
	std::shared_ptr<ArchiveEntry> getEntry(const std::string & entryPath, bool caseSensitive = false);
	std::shared_ptr<ArchiveEntry> getFirstEntryWithName(const std::string & entryName, bool includeSubdirectories = true, bool caseSensitive = false) const;
	std::shared_ptr<ArchiveEntry> getLastEntryWithName(const std::string & entryName, bool includeSubdirectories = true, bool caseSensitive = false) const;
	std::shared_ptr<ArchiveEntry> getFirstEntryWithExtension(const std::string & extension, bool includeSubdirectories = true, bool caseSensitive = false) const;
	std::shared_ptr<ArchiveEntry> getFirstEntryWithExtension(const std::vector<std::string> & extensions, bool includeSubdirectories = true, bool caseSensitive = false) const;
	std::shared_ptr<ArchiveEntry> getLastEntryWithExtension(const std::string & extension, bool includeSubdirectories = true, bool caseSensitive = false) const;
	std::shared_ptr<ArchiveEntry> getLastEntryWithExtension(const std::vector<std::string> & extensions, bool includeSubdirectories = true, bool caseSensitive = false) const;
	const std::shared_ptr<ArchiveEntry> getEntry(size_t index) const;
	std::shared_ptr<ArchiveEntry> getEntry(size_t index);
	virtual std::vector<std::shared_ptr<ArchiveEntry>> getEntries() const = 0;
	std::vector<std::shared_ptr<ArchiveEntry>> getRootEntries() const;
	std::vector<std::shared_ptr<ArchiveEntry>> getEntriesWithName(const std::string & entryName, bool includeSubdirectories = true, bool caseSensitive = false) const;
	std::vector<std::shared_ptr<ArchiveEntry>> getEntriesWithExtension(const std::string & extension, bool includeSubdirectories = true, bool caseSensitive = false) const;
	std::vector<std::shared_ptr<ArchiveEntry>> getEntriesWithExtensions(const std::vector<std::string> & extensions, bool includeSubdirectories = true, bool caseSensitive = false) const;
	std::vector<std::shared_ptr<ArchiveEntry>> getEntriesInDirectory(const std::string & directoryPath, bool includeSubdirectories = true, bool caseSensitive = false) const;
	std::shared_ptr<ArchiveEntry> extractFirstEntryWithExtension(const std::string & extension, const std::string & directory, bool overwrite = false, bool includeSubdirectories = true, bool caseSensitive = false) const;
	std::shared_ptr<ArchiveEntry> extractFirstEntryWithExtension(const std::vector<std::string> & extension, const std::string & directory, bool overwrite = false, bool includeSubdirectories = true, bool caseSensitive = false) const;
	std::shared_ptr<ArchiveEntry> extractLastEntryWithExtension(const std::string & extension, const std::string & directory, bool overwrite = false, bool includeSubdirectories = true, bool caseSensitive = false) const;
	std::shared_ptr<ArchiveEntry> extractLastEntryWithExtension(const std::vector<std::string> & extension, const std::string & directory, bool overwrite = false, bool includeSubdirectories = true, bool caseSensitive = false) const;
	std::vector<std::shared_ptr<ArchiveEntry>> extractAllEntriesWithExtension(const std::string & extension, const std::string & directory, bool overwrite = false, bool includeSubdirectories = true, bool caseSensitive = false) const;
	std::vector<std::shared_ptr<ArchiveEntry>> extractAllEntriesWithExtensions(const std::vector<std::string> & extension, const std::string & directory, bool overwrite = false, bool includeSubdirectories = true, bool caseSensitive = false) const;
	size_t extractAllEntries(const std::string & destinationDirectoryPath, bool includeSubdirectories = true, bool overwrite = false);
	size_t extractAllEntriesInSubdirectory(const std::string & destionationDirectoryPath, const std::string & archiveSubdirectory, bool relativeToSubdirectory = true, bool includeSubdirectories = true, bool overwrite = false, bool caseSensitive = false);
	void updateParentArchive();
	virtual std::string toDebugString(bool includeDate = false) const = 0;

protected:
	virtual void setFilePath(const std::string & filePath) = 0;

private:
	Type m_type;
};

#endif // _ARCHIVE_H_
