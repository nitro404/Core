#ifndef _NULLSOFT_SCRIPTABLE_INSTALL_SYSTEM_ARCHIVE_H_
#define _NULLSOFT_SCRIPTABLE_INSTALL_SYSTEM_ARCHIVE_H_

#include "Archive/Archive.h"

class NullsoftScriptableInstallSystemArchive final : public Archive {
	friend class Entry;

public:
	class Entry final : public ArchiveEntry {
		friend class NullsoftScriptableInstallSystemArchive;

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
		virtual bool setParentArchive(Archive * archive) override;

	private:
		Entry(uint64_t index, NullsoftScriptableInstallSystemArchive * parentArchive);

		uint64_t m_index;
		std::string m_path;
		NullsoftScriptableInstallSystemArchive * m_parentArchive;

		Entry(const Entry &) = delete;
		Entry(Entry &&) noexcept = delete;
		const Entry & operator = (const Entry &) = delete;
		const Entry & operator = (Entry &&) noexcept = delete;
	};

	virtual ~NullsoftScriptableInstallSystemArchive();

	virtual std::string getDefaultFileExtension() const override;
	virtual std::string getFilePath() const override;
	virtual bool hasComment() const override;
	virtual std::string getComment() const override;
	virtual size_t numberOfEntries() const override;
	virtual size_t numberOfFiles() const override;
	virtual size_t numberOfDirectories() const override;
	virtual std::vector<std::shared_ptr<ArchiveEntry>> getEntries() const override;
	virtual std::string toDebugString(bool includeDate = false) const override;

	static std::unique_ptr<NullsoftScriptableInstallSystemArchive> readFrom(const std::string & filePath);
	static std::unique_ptr<NullsoftScriptableInstallSystemArchive> createFrom(std::unique_ptr<ByteBuffer> data);

	static const std::string DEFAULT_FILE_EXTENSION;

private:
	NullsoftScriptableInstallSystemArchive();

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
