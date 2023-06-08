#ifndef _TAR_ARCHIVE_H_
#define _TAR_ARCHIVE_H_

#include "Archive/Archive.h"

#include <functional>

class TarArchive : public Archive {
	friend class Entry;

public:
	class Entry final : public ArchiveEntry {
		friend class TarArchive;

	public:
		Entry(Entry && t) noexcept;
		Entry(const Entry & t);
		Entry & operator = (Entry && t) noexcept;
		Entry & operator = (const Entry & t);
		virtual ~Entry();

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

		uint32_t getFileMode() const;
		uint32_t getUserID() const;
		uint32_t getGroupID() const;
		uint8_t getFileTypeFlag() const;
		const std::string & getLinkedFileName() const;
		const std::string & getMagic() const;
		const std::string & getVersion() const;
		const std::string & getOwnerUserName() const;
		const std::string & getOwnerGroupName() const;
		uint32_t getDeviceMajorNumber() const;
		uint32_t getDeviceMinorNumber() const;
		const std::string & getFileNamePrefix() const;
		const std::array<uint8_t, 12> & getPadding() const;
		bool isUStar() const;
		std::string fileTypeToString() const;

		static std::string fileTypeToString(uint8_t fileTypeFlag);

	protected:
		virtual Archive * getParentArchive() const override;
		virtual bool setParentArchive(Archive * archive) override;

		static std::unique_ptr<Entry> parseFrom(const ByteBuffer & data);

	private:
		Entry();

		uint64_t m_index;
		std::string m_entryPath;
		uint32_t m_fileMode = 0;
		uint32_t m_userID = 0;
		uint32_t m_groupID = 0;
		uint64_t m_fileSize = 0;
		std::chrono::time_point<std::chrono::system_clock> m_lastModifiedTimestamp = {};
		uint32_t m_checksum = 0;
		uint8_t m_fileTypeFlag = 0;
		std::string m_linkedFileName;
		std::string m_magic;
		std::string m_version;
		std::string m_ownerUserName;
		std::string m_ownerGroupName;
		uint32_t m_deviceMajorNumber = 0;
		uint32_t m_deviceMinorNumber = 0;
		std::string m_fileNamePrefix;
		std::unique_ptr<std::array<uint8_t, 12>> m_padding;
		std::unique_ptr<std::vector<uint8_t>> m_data;
		TarArchive * m_parentArchive;

		static const uint16_t DEFAULT_USTAR_VERSION;

		static const uint8_t NORMAL_FILE_FLAG;
		static const uint8_t HARD_LINK_FLAG;
		static const uint8_t SYMBOLIC_LINK_FLAG;
		static const uint8_t CHARACTER_SPECIAL_FLAG;
		static const uint8_t BLOCK_SPECIAL_FLAG;
		static const uint8_t DIRECTORY_FLAG;
		static const uint8_t FIFO_FLAG;
		static const uint8_t CONTIGUOUS_FILE_FLAG;
		static const uint8_t GLOBAL_EXTENDED_HEADER_WITH_METADATA_FLAG;
		static const uint8_t EXTENDED_HEADER_WITH_METADATA_FOR_NEXT_FILE_FLAG;
	};

	TarArchive(TarArchive && t) noexcept;
	TarArchive(const TarArchive & t);
	TarArchive & operator = (TarArchive && t) noexcept;
	TarArchive & operator = (const TarArchive & t);
	virtual ~TarArchive();

	virtual std::string getDefaultFileExtension() const override;
	virtual std::string getFilePath() const override;
	virtual bool hasComment() const override;
	virtual std::string getComment() const override;
	virtual size_t numberOfEntries() const override;
	virtual size_t numberOfFiles() const override;
	virtual size_t numberOfDirectories() const override;
	virtual std::vector<std::shared_ptr<ArchiveEntry>> getEntries() const override;
	virtual std::string toDebugString(bool includeDate = false) const override;

	static std::unique_ptr<TarArchive> readFrom(const std::string & filePath);
	static std::unique_ptr<TarArchive> createFrom(std::unique_ptr<ByteBuffer> data);

	static const std::string DEFAULT_FILE_EXTENSION;

protected:
	TarArchive(const std::string & filePath = {});

	std::vector<std::shared_ptr<Entry>> m_entries;
	size_t m_numberOfFiles;
	size_t m_numberOfDirectories;
	std::string m_filePath;
};

#endif // _TAR_ARCHIVE_H_
