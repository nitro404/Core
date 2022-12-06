#ifndef _RAR_ARCHIVE_H_
#define _RAR_ARCHIVE_H_

#include "ByteBuffer.h"

#include <dmc_unrar/dmc_unrar.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>

class RarArchive final {
	friend class Entry;

public:
	class Entry final {
		friend class RarArchive;

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
		bool hasComment() const;
		std::string getComment() const;
		std::chrono::time_point<std::chrono::system_clock> getDate() const;
		uint64_t getCompressedSize() const;
		uint64_t getUncompressedSize() const;
		std::unique_ptr<ByteBuffer> getData() const;
		uint32_t getCRC32() const;
		bool writeTo(const std::string & directoryPath, bool overwrite = false) const;

	private:
		Entry(uint64_t index, RarArchive * parentArchive);

		RarArchive * getParentArchive() const;
		void clearParentArchive();
		bool isParentArchiveValid() const;
		dmc_unrar_archive * getRawParentArchiveHandle() const;
		const dmc_unrar_file * getStatistics() const;

		uint64_t m_index;
		RarArchive * m_parentArchive;

		Entry(const Entry &) = delete;
		Entry(Entry &&) noexcept = delete;
		const Entry & operator = (const Entry &) = delete;
		const Entry & operator = (Entry &&) noexcept = delete;
	};

	~RarArchive();

	std::string getFilePath() const;
	bool hasComment() const;
	std::string getComment() const;
	uint64_t getCompressedSize() const;
	uint64_t getUncompressedSize() const;
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

	static std::unique_ptr<RarArchive> readFrom(const std::string & filePath);
	static std::unique_ptr<RarArchive> createFrom(std::unique_ptr<ByteBuffer> data);

private:
	using ArchiveHandle = std::unique_ptr<dmc_unrar_archive, std::function<void (dmc_unrar_archive *)>>;

	RarArchive(ArchiveHandle archiveHandle, const std::string & filePath, std::unique_ptr<ByteBuffer> data);

	const std::vector<std::shared_ptr<Entry>> & getEntries() const;
	std::vector<std::shared_ptr<Entry>> & getEntries();
	dmc_unrar_archive * getRawArchiveHandle() const;

	static bool isSuccess(dmc_unrar_return result, const std::string & errorMessage = {});
	static ArchiveHandle createArchiveHandle();
	static std::string convertComment(const std::string & comment);

	ArchiveHandle m_archiveHandle;
	std::unique_ptr<ByteBuffer> m_data;
	std::string m_filePath;
	std::vector<std::shared_ptr<Entry>> m_entries;
	size_t m_numberOfFiles;
	size_t m_numberOfDirectories;

	RarArchive(const RarArchive &) = delete;
	RarArchive(RarArchive &&) noexcept = delete;
	const RarArchive & operator = (const RarArchive &) = delete;
	const RarArchive & operator = (RarArchive &&) noexcept = delete;
};

#endif // _RAR_ARCHIVE_H_
