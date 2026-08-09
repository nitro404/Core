#ifndef _RAR_ARCHIVE_H_
#define _RAR_ARCHIVE_H_

#include "Archive/Archive.h"

#include <dmc_unrar/dmc_unrar.h>

#include <functional>

class RarArchive final : public Archive {
	friend class Entry;

public:
	class Entry final : public ArchiveEntry {
		friend class RarArchive;

	public:
		Entry(Entry && entry) noexcept;
		const Entry & operator = (Entry && entry) noexcept;
		~Entry() override;

		// ArchiveEntry Virtuals
		bool isFile() const override;
		bool isDirectory() const override;
		std::string getPath() const override;
		uint64_t getIndex() const override;
		bool hasComment() const override;
		std::string getComment() const override;
		std::chrono::time_point<std::chrono::system_clock> getDate() const override;
		uint64_t getCompressedSize() const override;
		uint64_t getUncompressedSize() const override;
		std::unique_ptr<ByteBuffer> getData() const override;
		uint32_t getCRC32() const override;
		bool writeToFile(const std::string & filePath, bool overwrite = false) override;

	protected:
		// ArchiveEntry Virtuals
		Archive * getParentArchive() const override;
		bool setParentArchive(Archive * archive) override;

	private:
		Entry(uint64_t index, RarArchive * parentArchive);

		dmc_unrar_archive * getRawParentArchiveHandle() const;
		const dmc_unrar_file * getStatistics() const;

		uint64_t m_index;
		RarArchive * m_parentArchive;

		Entry(const Entry &) = delete;
		const Entry & operator = (const Entry &) = delete;
	};

	RarArchive(RarArchive && archive) noexcept;
	const RarArchive & operator = (RarArchive && archive) noexcept;
	~RarArchive() override;

	// Archive Virtuals
	std::string getDefaultFileExtension() const override;
	std::string getFilePath() const override;
	bool hasComment() const override;
	std::string getComment() const override;
	size_t numberOfEntries() const override;
	size_t numberOfFiles() const override;
	size_t numberOfDirectories() const override;
	std::vector<std::shared_ptr<ArchiveEntry>> getEntries() const override;
	std::string toDebugString(bool includeDate = false) const override;

	static bool isRarArchive(const std::string & filePath);
	static bool isRarArchive(const ByteBuffer & data);
	static std::unique_ptr<RarArchive> readFrom(const std::string & filePath);
	static std::unique_ptr<RarArchive> createFrom(std::unique_ptr<ByteBuffer> data);

	static const std::string DEFAULT_FILE_EXTENSION;

protected:
	// Archive Virtuals
	void setFilePath(const std::string & filePath) override;

private:
	using ArchiveHandle = std::unique_ptr<dmc_unrar_archive, std::function<void (dmc_unrar_archive *)>>;

	RarArchive(ArchiveHandle archiveHandle, const std::string & filePath, std::unique_ptr<ByteBuffer> data);

	dmc_unrar_archive * getRawArchiveHandle() const;
	void updateParentArchive();

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
	const RarArchive & operator = (const RarArchive &) = delete;
};

#endif // _RAR_ARCHIVE_H_
