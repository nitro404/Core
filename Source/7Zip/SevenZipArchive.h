#ifndef _SEVEN_ZIP_ARCHIVE_H_
#define _SEVEN_ZIP_ARCHIVE_H_

#include "ByteBuffer.h"

#define NOMINMAX
#include <7Zip/C/7z.h>
#include <7Zip/C/7zFile.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>

class SevenZipArchive final {
	friend class Entry;

public:
	class Entry final {
		friend class SevenZipArchive;

	public:
		~Entry();

		bool isFile() const;
		bool isDirectory() const;
		bool isInSubdirectory() const;
		static bool isInSubdirectory(std::string_view path);
		std::string getName() const;
		std::string getPath() const;
		std::vector<std::weak_ptr<Entry>> getChildren(bool includeSubdirectories = true, bool caseSensitive = false) const;
		uint64_t getIndex() const;
		std::chrono::time_point<std::chrono::system_clock> getDate() const;
		uint64_t getInflatedSize() const;
		std::unique_ptr<ByteBuffer> getData() const;
		uint32_t getCRC32() const;
		bool writeTo(const std::string & directoryPath, bool overwrite = false) const;

	private:
		Entry(uint64_t index, SevenZipArchive * parentArchive);

		SevenZipArchive * getParentArchive() const;
		void clearParentArchive();
		bool isParentArchiveValid() const;

		uint64_t m_index;
		SevenZipArchive * m_parentArchive;

		Entry(const Entry &) = delete;
		Entry(Entry &&) noexcept = delete;
		const Entry & operator = (const Entry &) = delete;
		const Entry & operator = (Entry &&) noexcept = delete;
	};

	~SevenZipArchive();

	std::string getFilePath() const;
	uint64_t getInflatedSize() const;
	size_t numberOfEntries() const;
	size_t numberOfFiles() const;
	size_t numberOfDirectories() const;
	bool hasEntry(const Entry & entry) const;
	bool hasEntry(const std::string & entryPath, bool caseSensitive = false) const;
	bool hasEntryWithName(const std::string & entryName, bool includeSubdirectories = true, bool caseSensitive = false) const;
	size_t indexOfEntry(const std::string & entryPath, bool caseSensitive = false) const;
	size_t indexOfFirstEntryWithName(const std::string & entryName, bool includeSubdirectories = true, bool caseSensitive = false) const;
	const std::weak_ptr<Entry> getEntry(const std::string & entryPath, bool caseSensitive = false) const;
	std::weak_ptr<Entry> getEntry(const std::string & entryPath, bool caseSensitive = false);
	std::weak_ptr<Entry> getFirstEntryWithName(const std::string & entryName, bool includeSubdirectories = true, bool caseSensitive = false) const;
	const std::weak_ptr<Entry> getEntry(size_t index) const;
	std::weak_ptr<Entry> getEntry(size_t index);
	size_t extractAllEntries(const std::string & directoryPath, bool overwrite = false) const;
	std::string toDebugString(bool includeDate = false) const;

	static std::unique_ptr<SevenZipArchive> readFrom(const std::string & filePath);
	static std::unique_ptr<SevenZipArchive> createFrom(std::unique_ptr<ByteBuffer> data);

private:
	using ArchiveStreamHandle = std::unique_ptr<CFileInStream, std::function<void (CFileInStream *)>>;
	using LookStreamHandle = std::unique_ptr<CLookToRead2, std::function<void (CLookToRead2 *)>>;
	using ArchiveHandle = std::unique_ptr<CSzArEx, std::function<void (CSzArEx *)>>;
	using AllocatorHandle = std::unique_ptr<ISzAlloc>;

	struct ExtractionData {
		uint32_t blockIndex = std::numeric_limits<uint32_t>::max();
		uint8_t * outputBuffer = 0;
		size_t outputBufferSize = 0;
	};

	SevenZipArchive(ArchiveStreamHandle archiveStream, LookStreamHandle lookStream, ArchiveHandle archive, AllocatorHandle allocator, const std::string & filePath, std::unique_ptr<ByteBuffer> data);

	const std::vector<std::shared_ptr<Entry>> & getEntries() const;
	std::vector<std::shared_ptr<Entry>> & getEntries();
	const CFileInStream * getRawArchiveStreamHandle() const;
	CFileInStream * getRawArchiveStreamHandle();
	const CLookToRead2 * getRawLookStreamHandle() const;
	CLookToRead2 * getRawLookStreamHandle();
	const CSzArEx * getRawArchiveHandle() const;
	CSzArEx * getRawArchiveHandle();
	const ISzAlloc * getRawAllocatorHandle() const;
	ISzAlloc * getRawAllocatorHandle();
	ExtractionData & getCachedExtractionData();

	static std::unique_ptr<SevenZipArchive> createFrom(ArchiveStreamHandle archiveStream, const std::string & filePath, std::unique_ptr<ByteBuffer> data);
	static std::chrono::time_point<std::chrono::system_clock> getTimePointFromNTFSFileTime(const CNtfsFileTime & ntfsFileTime);
	static ArchiveStreamHandle createArchiveStreamHandle();
	static LookStreamHandle createLookStreamHandle(ISzAlloc & allocator);
	static ArchiveHandle createArchiveHandle(ISzAlloc & allocator);
	static AllocatorHandle createAllocatorHandle(const ISzAlloc & allocator);

	ArchiveStreamHandle m_archiveStream;
	LookStreamHandle m_lookStream;
	ArchiveHandle m_archive;
	AllocatorHandle m_allocator;
	std::unique_ptr<ByteBuffer> m_data;
	ExtractionData m_cachedExtractionData;
	std::string m_filePath;
	std::vector<std::shared_ptr<Entry>> m_entries;
	size_t m_numberOfFiles;
	size_t m_numberOfDirectories;

	static const ISzAlloc DEFAULT_ALLOCATOR;

	SevenZipArchive(const SevenZipArchive &) = delete;
	SevenZipArchive(SevenZipArchive &&) noexcept = delete;
	const SevenZipArchive & operator = (const SevenZipArchive &) = delete;
	const SevenZipArchive & operator = (SevenZipArchive &&) noexcept = delete;
};

#endif // _SEVEN_ZIP_ARCHIVE_H_
