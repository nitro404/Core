#ifndef _SEVEN_ZIP_ARCHIVE_H_
#define _SEVEN_ZIP_ARCHIVE_H_

#include "Archive/Archive.h"

#include <SevenZip/C/7z.h>
#include <SevenZip/C/7zFile.h>

#include <functional>

class SevenZipArchive final : public Archive {
	friend class Entry;

public:
	class Entry final : public ArchiveEntry {
		friend class SevenZipArchive;

	public:
		Entry(Entry && entry) noexcept;
		const Entry & operator = (Entry && entry) noexcept;
		~Entry() override;

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
		virtual bool writeToFile(const std::string & filePath, bool overwrite = false) override;

	protected:
		virtual Archive * getParentArchive() const override;
		virtual bool setParentArchive(Archive * archive) override;

	private:
		Entry(uint64_t index, SevenZipArchive * parentArchive);

		uint64_t m_index;
		SevenZipArchive * m_parentArchive;

		Entry(const Entry &) = delete;
		const Entry & operator = (const Entry &) = delete;
	};

	SevenZipArchive(SevenZipArchive && archive) noexcept;
	const SevenZipArchive & operator = (SevenZipArchive && archive) noexcept;
	~SevenZipArchive() override;

	virtual std::string getDefaultFileExtension() const override;
	virtual std::string getFilePath() const override;
	virtual bool hasComment() const override;
	virtual std::string getComment() const override;
	virtual uint64_t getCompressedSize() const override;
	virtual size_t numberOfEntries() const override;
	virtual size_t numberOfFiles() const override;
	virtual size_t numberOfDirectories() const override;
	virtual std::vector<std::shared_ptr<ArchiveEntry>> getEntries() const override;
	virtual std::string toDebugString(bool includeDate = false) const override;

	static bool is7ZipArchive(const std::string & filePath);
	static bool is7ZipArchive(const ByteBuffer & data);
	static std::unique_ptr<SevenZipArchive> readFrom(const std::string & filePath);
	static std::unique_ptr<SevenZipArchive> createFrom(std::unique_ptr<ByteBuffer> data);

	static const std::string DEFAULT_FILE_EXTENSION;

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

	SevenZipArchive(ArchiveStreamHandle archiveStream, LookStreamHandle lookStream, ArchiveHandle archive, AllocatorHandle allocator, const std::string & filePath, std::unique_ptr<ByteBuffer> data, uint64_t compressedSize);

	const CFileInStream * getRawArchiveStreamHandle() const;
	CFileInStream * getRawArchiveStreamHandle();
	const CLookToRead2 * getRawLookStreamHandle() const;
	CLookToRead2 * getRawLookStreamHandle();
	const CSzArEx * getRawArchiveHandle() const;
	CSzArEx * getRawArchiveHandle();
	const ISzAlloc * getRawAllocatorHandle() const;
	ISzAlloc * getRawAllocatorHandle();
	ExtractionData & getCachedExtractionData();
	void updateParentArchive();

	static std::unique_ptr<SevenZipArchive> createFrom(ArchiveStreamHandle archiveStream, const std::string & filePath, std::unique_ptr<ByteBuffer> data, uint64_t compressedSize);
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
	uint64_t m_compressedSize;

	static const ISzAlloc DEFAULT_ALLOCATOR;

	SevenZipArchive(const SevenZipArchive &) = delete;
	const SevenZipArchive & operator = (const SevenZipArchive &) = delete;
};

#endif // _SEVEN_ZIP_ARCHIVE_H_
