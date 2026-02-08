#ifndef _NULLSOFT_SCRIPTABLE_INSTALL_SYSTEM_ARCHIVE_H_
#define _NULLSOFT_SCRIPTABLE_INSTALL_SYSTEM_ARCHIVE_H_

#include "Archive/Archive.h"

#include <SevenZip/CPP/7zip/Archive/IArchive.h>
#include <SevenZip/CPP/Common/MyCom.h>

class NullsoftScriptableInstallSystemArchive final : public Archive {
	friend class Entry;

public:
	class Entry final : public ArchiveEntry {
		friend class NullsoftScriptableInstallSystemArchive;

	public:
		Entry(Entry && entry) noexcept;
		const Entry & operator = (Entry && entry) noexcept;
		~Entry() override;

		// ArchiveEntry Virtuasls
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

		CMyComPtr<IInArchive> & getParentArchiveHandle();
		const CMyComPtr<IInArchive> & getParentArchiveHandle() const;

	protected:
		virtual Archive * getParentArchive() const override;
		virtual bool setParentArchive(Archive * archive) override;

	private:
		Entry(uint64_t index, NullsoftScriptableInstallSystemArchive * parentArchive);

		uint64_t m_index;
		NullsoftScriptableInstallSystemArchive * m_parentArchive;

		Entry(const Entry &) = delete;
		const Entry & operator = (const Entry &) = delete;
	};

	NullsoftScriptableInstallSystemArchive(NullsoftScriptableInstallSystemArchive && archive) noexcept;
	const NullsoftScriptableInstallSystemArchive & operator = (NullsoftScriptableInstallSystemArchive && archive) noexcept;
	~NullsoftScriptableInstallSystemArchive() override;

	// Archive Virtuals
	virtual std::string getDefaultFileExtension() const override;
	virtual std::string getFilePath() const override;
	virtual bool hasComment() const override;
	virtual std::string getComment() const override;
	virtual size_t numberOfEntries() const override;
	virtual size_t numberOfFiles() const override;
	virtual size_t numberOfDirectories() const override;
	virtual std::vector<std::shared_ptr<ArchiveEntry>> getEntries() const override;
	virtual std::string toDebugString(bool includeDate = false) const override;

	CMyComPtr<IInArchive> & getArchiveHandle();
	const CMyComPtr<IInArchive> & getArchiveHandle() const;

	static bool isNSISArchive(const std::string & filePath);
	static bool isNSISArchive(const ByteBuffer & data);
	static std::unique_ptr<NullsoftScriptableInstallSystemArchive> readFrom(const std::string & filePath);
	static std::unique_ptr<NullsoftScriptableInstallSystemArchive> createFrom(std::unique_ptr<ByteBuffer> data);

	static std::string getExtractionOperationErrorMessage(Int32 result);

	static const std::string DEFAULT_FILE_EXTENSION;

private:
	NullsoftScriptableInstallSystemArchive(CMyComPtr<IInArchive> archiveHandle);

	void updateParentArchive();
	static std::unique_ptr<NullsoftScriptableInstallSystemArchive> createFrom(CMyComPtr<IInStream> inputStream);

	std::string m_filePath;
	std::vector<std::shared_ptr<Entry>> m_entries;
	size_t m_numberOfFiles;
	size_t m_numberOfDirectories;
	CMyComPtr<IInArchive> m_archiveHandle;

	NullsoftScriptableInstallSystemArchive(const NullsoftScriptableInstallSystemArchive &) = delete;
	const NullsoftScriptableInstallSystemArchive & operator = (const NullsoftScriptableInstallSystemArchive &) = delete;
};

#endif // _NULLSOFT_SCRIPTABLE_INSTALL_SYSTEM_ARCHIVE_H_
