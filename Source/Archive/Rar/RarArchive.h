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
		Entry(uint64_t index, RarArchive * parentArchive);

		dmc_unrar_archive * getRawParentArchiveHandle() const;
		const dmc_unrar_file * getStatistics() const;

		uint64_t m_index;
		RarArchive * m_parentArchive;

		Entry(const Entry &) = delete;
		Entry(Entry &&) noexcept = delete;
		const Entry & operator = (const Entry &) = delete;
		const Entry & operator = (Entry &&) noexcept = delete;
	};

	virtual ~RarArchive();

	virtual std::string getDefaultFileExtension() const override;
	virtual std::string getFilePath() const override;
	virtual bool hasComment() const override;
	virtual std::string getComment() const override;
	virtual size_t numberOfEntries() const override;
	virtual size_t numberOfFiles() const override;
	virtual size_t numberOfDirectories() const override;
	virtual std::vector<std::shared_ptr<ArchiveEntry>> getEntries() const override;
	virtual std::string toDebugString(bool includeDate = false) const override;

	static std::unique_ptr<RarArchive> readFrom(const std::string & filePath);
	static std::unique_ptr<RarArchive> createFrom(std::unique_ptr<ByteBuffer> data);

	static const std::string DEFAULT_FILE_EXTENSION;

private:
	using ArchiveHandle = std::unique_ptr<dmc_unrar_archive, std::function<void (dmc_unrar_archive *)>>;

	RarArchive(ArchiveHandle archiveHandle, const std::string & filePath, std::unique_ptr<ByteBuffer> data);

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
