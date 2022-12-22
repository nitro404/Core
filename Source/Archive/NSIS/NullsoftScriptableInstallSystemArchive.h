#ifndef _NULLSOFT_SCRIPTABLE_INSTALL_SYSTEM_ARCHIVE_H_
#define _NULLSOFT_SCRIPTABLE_INSTALL_SYSTEM_ARCHIVE_H_

#include "ByteBuffer.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

class NullsoftScriptableInstallSystemArchive final {
	friend class Entry;

public:
	class Entry final {
		friend class NullsoftScriptableInstallSystemArchive;

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
		Entry(uint64_t index, NullsoftScriptableInstallSystemArchive * parentArchive);

		NullsoftScriptableInstallSystemArchive * getParentArchive() const;
		void clearParentArchive();
		bool isParentArchiveValid() const;

		uint64_t m_index;
		std::string m_path;
		NullsoftScriptableInstallSystemArchive * m_parentArchive;

		Entry(const Entry &) = delete;
		Entry(Entry &&) noexcept = delete;
		const Entry & operator = (const Entry &) = delete;
		const Entry & operator = (Entry &&) noexcept = delete;
	};

	~NullsoftScriptableInstallSystemArchive();

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

	static std::unique_ptr<NullsoftScriptableInstallSystemArchive> readFrom(const std::string & filePath);

private:
	NullsoftScriptableInstallSystemArchive(const std::string & filePath);

	const std::vector<std::shared_ptr<Entry>> & getEntries() const;
	std::vector<std::shared_ptr<Entry>> & getEntries();

	std::string m_filePath;
	std::vector<std::shared_ptr<Entry>> m_entries;
	size_t m_numberOfFiles;
	size_t m_numberOfDirectories;

	NullsoftScriptableInstallSystemArchive(const NullsoftScriptableInstallSystemArchive &) = delete;
	NullsoftScriptableInstallSystemArchive(NullsoftScriptableInstallSystemArchive &&) noexcept = delete;
	const NullsoftScriptableInstallSystemArchive & operator = (const NullsoftScriptableInstallSystemArchive &) = delete;
	const NullsoftScriptableInstallSystemArchive & operator = (NullsoftScriptableInstallSystemArchive &&) noexcept = delete;
};

#endif // _NULLSOFT_SCRIPTABLE_INSTALL_SYSTEM_ARCHIVE_H_
