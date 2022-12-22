#ifndef _ZIP_ARCHIVE_H_
#define _ZIP_ARCHIVE_H_

#include "Archive/Archive.h"
#include "BitmaskOperators.h"

#include <magic_enum.hpp>
#include <zip.h>

#include <functional>

class ZipArchive final : public Archive {
	friend class Entry;

private:
	using ZipArchiveHandle = std::unique_ptr<zip, std::function<void (zip *)>>;
	using ZipSourceHandle = std::unique_ptr<zip_source, std::function<void (zip_source *)>>;

public:
	enum class CompressionMethod : int8_t {
		Default = -1,
		Store = 0,
		Shrink = 1,
		Reduce1 = 2,
		Reduce2 = 3,
		Reduce3 = 4,
		Reduce4 = 5,
		Implode = 6,
		Deflate = 8,
		Deflate64 = 9,
		PKWareImplode = 10,
		BZip2 = 12,
		LZMA = 14,
		IBMTerse = 18,
		IBMLZ77 = 19,
		LZMA2 = 33,
		ZStandard = 93,
		XZ = 95,
		JPEG = 96,
		WavPack = 97,
		PPMdVersion1Revision1 = 98
	};

	enum class CompressionType : uint8_t {
		Compress = 1,
		Decompress = 1 << 1,
		Both = Compress | Decompress
	};

	enum class EncryptionMethod : uint16_t {
		None = 0,
		TraditionalPKWare = 1,
		AES128 = 257,
		AES192 = 258,
		AES256 = 259
	};

	enum class EncryptionType : uint8_t {
		Encrypt = 1,
		Decrypt = 1 << 1,
		Both = Encrypt | Decrypt
	};

	class Entry final : public ArchiveEntry {
		friend class ZipArchive;

	public:
		virtual ~Entry();

		bool setName(const std::string & name);
		virtual std::string getPath() const override;
		bool move(const std::string & newBasePath, bool overwrite = false);
		virtual uint64_t getIndex() const override;
		virtual std::chrono::time_point<std::chrono::system_clock> getDate() const override;
		virtual bool hasComment() const override;
		virtual std::string getComment() const override;
		bool setComment(const std::string & comment);
		bool clearComment();
		virtual uint64_t getCompressedSize() const override;
		virtual uint64_t getUncompressedSize() const override;
		bool hasUnsavedData() const;
		virtual std::unique_ptr<ByteBuffer> getData() const override;
		bool isCompressed() const;
		CompressionMethod getCompressionMethod() const;
		bool setCompressionMethod(CompressionMethod compressionMethod);
		bool isEncrypted() const;
		EncryptionMethod getEncryptionMethod() const;
		bool setEncryptionMethod(EncryptionMethod encryptionMethod);
		virtual uint32_t getCRC32() const override;
		virtual bool writeTo(const std::string & directoryPath, bool overwrite = false) const override;

	protected:
		virtual bool isParentArchiveValid() const override;
		virtual Archive * getParentArchive() const override;
		virtual bool setParentArchive(Archive * archive) override;

	private:
		using ZipFileHandle = std::unique_ptr<struct zip_file, std::function<void (struct zip_file *)>>;

		Entry(const std::string & path, uint64_t index, std::unique_ptr<ByteBuffer> data, std::chrono::time_point<std::chrono::system_clock> date, CompressionMethod compressionMethod, EncryptionMethod encryptionMethod, uint64_t compressedSize, uint64_t uncompressedSize, uint32_t crc32, ZipArchive * parentArchive);

		bool setIndex(uint64_t index);
		void clearUnsavedData();
		static ZipFileHandle createZipFileHandle(zip_file * zipFileHandle);

		ZipArchive * m_parentArchive;
		std::string m_path;
		uint64_t m_index;
		std::unique_ptr<ByteBuffer> m_unsavedData;
		std::chrono::time_point<std::chrono::system_clock> m_date;
		CompressionMethod m_compressionMethod;
		EncryptionMethod m_encryptionMethod;
		uint64_t m_compressedSize;
		uint64_t m_uncompressedSize;
		uint32_t m_crc32;

		Entry(const Entry &) = delete;
		Entry(Entry &&) noexcept = delete;
		const Entry & operator = (const Entry &) = delete;
		const Entry & operator = (Entry &&) noexcept = delete;
	};

	virtual ~ZipArchive();

	virtual std::string getDefaultFileExtension() const override;
	virtual bool isOpen() const override;
	virtual bool isModifiable() const override;
	virtual std::string getFilePath() const override;
	bool hasPassword() const;
	const std::string & getPassword() const;
	bool setPassword(const std::string & password);
	bool clearPassword();
	bool isCompressed() const;
	CompressionMethod getCompressionMethod() const;
	bool setCompressionMethod(CompressionMethod compressionMethod);
	bool isEncrypted() const;
	EncryptionMethod getEncryptionMethod() const;
	bool setEncryptionMethod(EncryptionMethod encryptionMethod);
	static bool isCompressionMethodSupported(CompressionMethod compressionMethod, CompressionType compressionType = CompressionType::Both);
	static bool isEncryptionMethodSupported(EncryptionMethod encryptionMethod, EncryptionType encryptionType = EncryptionType::Both);
	std::chrono::time_point<std::chrono::system_clock> getDate() const;
	virtual bool hasComment() const override;
	virtual std::string getComment() const override;
	bool setComment(const std::string & comment);
	bool clearComment();
	virtual uint64_t getCompressedSize() const override;
	const ByteBuffer * getData() const;
	virtual size_t numberOfEntries() const override;
	virtual size_t numberOfFiles() const override;
	virtual size_t numberOfDirectories() const override;
	std::shared_ptr<Entry> addFile(const std::string & filePath, const std::string & entryDirectoryPath = {}, bool overwrite = true);
	std::shared_ptr<Entry> addData(std::unique_ptr<ByteBuffer> data, const std::string & entryFilePath, bool overwrite = true);
	std::shared_ptr<Entry> addDirectory(const std::string & entryDirectoryPath);
	static std::string formatDirectoryPath(const std::string & directoryPath);
	size_t removeEntry(Entry & entry);
	size_t removeEntry(size_t index);
	size_t removeEntry(const std::string & entryPath, bool caseSensitive = false);
	size_t removeFirstEntryWithName(const std::string & entryName, bool includeSubdirectories = true, bool caseSensitive = false);
	size_t removeAllEntries();
	bool isModified() const;
	static std::unique_ptr<ZipArchive> createNew(const std::string & filePath = {}, bool overwrite = false);
	static std::unique_ptr<ZipArchive> createFrom(std::unique_ptr<ByteBuffer> buffer, const std::string & password = {}, bool verifyConsistency = false, const std::string & filePath = {});
	static std::unique_ptr<ZipArchive> readFrom(const std::string & filePath, const std::string & password = {}, bool verifyConsistency = false);
	bool close();
	bool reopen(bool verifyConsistency = false);
	bool save();
	virtual std::vector<std::shared_ptr<ArchiveEntry>> getEntries() const override;
	virtual std::string toDebugString(bool includeDate = false) const override;

	static const std::string DEFAULT_FILE_EXTENSION;
	static const EncryptionMethod DEFAULT_ENCRYPTION_METHOD;

private:
	class SourceBuffer final {
		friend class ZipArchive;

	public:
		~SourceBuffer();

		const ByteBuffer * getData() const;
		ByteBuffer * getData();
		const uint8_t * getRawData() const;
		size_t getSize() const;
		void deleteSourceHandle();
		void deleteDataBuffer();
		zip_source * releaseSourceHandle();
		ByteBuffer * releaseDataBuffer();
		ZipSourceHandle transferSourceHandle();
		std::unique_ptr<ByteBuffer> transferDataBuffer();
		bool isOpen() const;
		bool close();
		bool reopen();

	private:
		SourceBuffer(ZipSourceHandle zipSourceHandle, std::unique_ptr<ByteBuffer> data);

		zip_source * getRawSourceHandle() const;

		ZipSourceHandle m_sourceHandle;
		std::unique_ptr<ByteBuffer> m_data;

		SourceBuffer(const SourceBuffer &) = delete;
		SourceBuffer(SourceBuffer &&) noexcept = delete;
		const SourceBuffer & operator = (const SourceBuffer &) = delete;
		const SourceBuffer & operator = (SourceBuffer &&) noexcept = delete;
	};

	ZipArchive(ZipArchiveHandle zipArchiveHandle, std::unique_ptr<SourceBuffer> zipSourceBuffer = nullptr, const std::string & filePath = {}, const std::string & password = {});
	ZipArchive(ZipArchiveHandle zipArchiveHandle, const std::string & filePath, const std::string & password = {});

	std::unique_ptr<Entry> createEntryFromIndex(size_t index, std::unique_ptr<ByteBuffer> data = nullptr);
	void setModified();
	void reset();
	bool initialize();
	bool populateInfo();
	bool populateEntries();
	bool populateDefaultMethods();
	bool addEntry(std::unique_ptr<Entry> entry);
	size_t removeEntry(Entry & entry, bool removeChildren);
	zip * getRawArchiveHandle() const;
	zip_source * getRawSourceHandle() const;
	static ZipArchiveHandle createZipArchiveHandle(zip * zipArchiveHandle);
	static ZipSourceHandle createZipSourceHandle(zip_source * zipSourceHandle);
	std::unique_ptr<SourceBuffer> createEmptyZipFileSourceBuffer();
	std::unique_ptr<SourceBuffer> createZipFileSourceBuffer(std::unique_ptr<ByteBuffer> data);
	static std::unique_ptr<SourceBuffer> createEmptyZipArchiveSourceBuffer();
	static std::unique_ptr<SourceBuffer> createZipArchiveSourceBuffer(std::unique_ptr<ByteBuffer> data);

	ZipArchiveHandle m_archiveHandle;
	std::unique_ptr<SourceBuffer> m_sourceBuffer;
	std::string m_filePath;
	std::string m_password;
	std::chrono::time_point<std::chrono::system_clock> m_date;
	CompressionMethod m_compressionMethod;
	EncryptionMethod m_encryptionMethod;
	uint64_t m_compressedSize;
	std::vector<std::shared_ptr<Entry>> m_entries;
	size_t m_numberOfFiles;
	size_t m_numberOfDirectories;
	bool m_modified;

	ZipArchive(const ZipArchive &) = delete;
	ZipArchive(ZipArchive &&) noexcept = delete;
	const ZipArchive & operator = (const ZipArchive &) = delete;
	const ZipArchive & operator = (ZipArchive &&) noexcept = delete;
};

template <>
struct magic_enum::customize::enum_range<ZipArchive::CompressionMethod> {
	static constexpr int min = -1;
	static constexpr int max = 100;
};

template <>
struct magic_enum::customize::enum_range<ZipArchive::EncryptionMethod> {
	static constexpr int min = 0;
	static constexpr int max = 260;
};

template<>
struct BitmaskOperators<ZipArchive::CompressionType> {
	static const bool enabled = true;
};

template<>
struct BitmaskOperators<ZipArchive::EncryptionType> {
	static const bool enabled = true;
};

#endif // _ZIP_ARCHIVE_H_
