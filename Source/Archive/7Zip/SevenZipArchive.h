#ifndef _SEVEN_ZIP_ARCHIVE_H_
#define _SEVEN_ZIP_ARCHIVE_H_

#include "Archive/Archive.h"
#include "ByteBuffer.h"

#include <7Zip/C/7z.h>
#include <7Zip/C/7zFile.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>

class SevenZipArchive final : public Archive {
	friend class Entry;

public:
	class Entry final : public ArchiveEntry {
		friend class SevenZipArchive;

	public:
		virtual ~Entry();

		virtual bool isFile() const override;
		virtual bool isDirectory() const override;
		virtual std::string getPath() const override;
		virtual uint64_t getIndex() const override;
		virtual bool hasComment() const override;
		virtual std::string getComment() const override;
		virtual std::chrono::time_point<std::chrono::system_clock> getDate() const override;
		virtual uint64_t getCompressedSize() const override;
		virtual uint64_t getUncompressedSize() const override;
		virtual std::unique_ptr<ByteBuffer> getData() const override;
		virtual uint32_t getCRC32() const override;
		virtual bool writeTo(const std::string & directoryPath, bool overwrite = false) const override;

	protected:
		virtual Archive * getParentArchive() const override;
		virtual void clearParentArchive() override;

	private:
		Entry(uint64_t index, SevenZipArchive * parentArchive);

		uint64_t m_index;
		SevenZipArchive * m_parentArchive;

		Entry(const Entry &) = delete;
		Entry(Entry &&) noexcept = delete;
		const Entry & operator = (const Entry &) = delete;
		const Entry & operator = (Entry &&) noexcept = delete;
	};

	virtual ~SevenZipArchive();

	virtual std::string getFilePath() const override;
	virtual bool hasComment() const override;
	virtual std::string getComment() const override;
	virtual uint64_t getCompressedSize() const override;
	virtual size_t numberOfEntries() const override;
	virtual size_t numberOfFiles() const override;
	virtual size_t numberOfDirectories() const override;
	virtual std::string toDebugString(bool includeDate = false) const override;

	static std::unique_ptr<SevenZipArchive> readFrom(const std::string & filePath);
	static std::unique_ptr<SevenZipArchive> createFrom(std::unique_ptr<ByteBuffer> data);

protected:
	virtual std::vector<std::shared_ptr<ArchiveEntry>> getEntries() const override;

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
