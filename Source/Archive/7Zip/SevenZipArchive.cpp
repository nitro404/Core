#include "SevenZipArchive.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/TimeUtilities.h"

#include <SevenZip/C/7zAlloc.h>
#include <SevenZip/C/7zCrc.h>
#include <SevenZip/C/7zTypes.h>

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

const ISzAlloc SevenZipArchive::DEFAULT_ALLOCATOR = { SzAlloc, SzFree };

static std::vector<const ByteBuffer *> s_byteBufferVirtualFileHandles;

static void createVirtualByteBufferFileHandle(CSzFile & file, const ByteBuffer & data) {
#ifdef USE_WINDOWS_FILE
	file.handle = reinterpret_cast<HANDLE>(const_cast<ByteBuffer *>(&data));
#else
	s_byteBufferVirtualFileHandles.push_back(&data);

	#if defined(USE_FOPEN)
		file.file = reinterpret_cast<FILE *>(s_byteBufferVirtualFileHandles.size() - 1);
	#else
		file.fd = static_cast<int>(s_byteBufferVirtualFileHandles.size() - 1);
	#endif
#endif
}

static const ByteBuffer * getByteBufferFromSeekInStreamInterface(const ISeekInStream * seekInStreamInterface) {
	CFileInStream * fileInStream = CONTAINER_FROM_VTBL(seekInStreamInterface, CFileInStream, vt);

#ifdef USE_WINDOWS_FILE
	return reinterpret_cast<const ByteBuffer *>(fileInStream->file.handle);
#else
	size_t byteBufferVirtualFileHandleIndex = 0;

	#if defined(USE_FOPEN)
		byteBufferVirtualFileHandleIndex = reinterpret_cast<size_t>(fileInStream->file.file);
	#else
		byteBufferVirtualFileHandleIndex = fileInStream->file.fd;
	#endif

	return s_byteBufferVirtualFileHandles[byteBufferVirtualFileHandleIndex];
#endif
}

static SRes readFromVirtualByteBufferFile(const ISeekInStream * seekInStreamInterface, void * outputBuffer, size_t * numberOfBytesToRead) {
	const ByteBuffer * data = getByteBufferFromSeekInStreamInterface(seekInStreamInterface);

	size_t numberOfBytesRead = std::min(*numberOfBytesToRead, data->numberOfBytesRemaining());
	std::memcpy(outputBuffer, data->getRawData() + data->getReadOffset(), numberOfBytesRead);
	data->skipReadBytes(numberOfBytesRead);
	*numberOfBytesToRead = numberOfBytesRead;

	return SZ_OK;
}

static SRes seekVirtualByteBufferFile(const ISeekInStream * seekInStreamInterface, Int64 * offset, ESzSeek seekType) {
	const ByteBuffer * data = getByteBufferFromSeekInStreamInterface(seekInStreamInterface);

	switch(seekType) {
		case SZ_SEEK_SET: {
			data->setReadOffset(*offset);
			break;
		}
		case SZ_SEEK_CUR: {
			data->setReadOffset(data->getReadOffset() + *offset);
			break;
		}
		case SZ_SEEK_END: {
			data->setReadOffset(data->getSize() + *offset);
			break;
		}
	}

	*offset = data->getReadOffset();

	return SZ_OK;
}

const std::string SevenZipArchive::DEFAULT_FILE_EXTENSION("7z");

SevenZipArchive::SevenZipArchive(ArchiveStreamHandle archiveStream, LookStreamHandle lookStream, ArchiveHandle archive, AllocatorHandle allocator, const std::string & filePath, std::unique_ptr<ByteBuffer> data, uint64_t compressedSize)
	: Archive(Type::SevenZip)
	, m_archiveStream(std::move(archiveStream))
	, m_lookStream(std::move(lookStream))
	, m_archive(std::move(archive))
	, m_allocator(std::move(allocator))
	, m_data(std::move(data))
	, m_filePath(filePath)
	, m_numberOfFiles(0)
	, m_numberOfDirectories(0)
	, m_compressedSize(compressedSize) {
	for(size_t i = 0; i < m_archive->NumFiles; i++) {
		m_entries.emplace_back(new Entry(i, this));

		if(m_entries[i]->isDirectory()) {
			m_numberOfDirectories++;
		}
		else {
			m_numberOfFiles++;
		}
	}
}

SevenZipArchive::SevenZipArchive(SevenZipArchive && archive) noexcept
	: Archive(std::move(archive))
	, m_archiveStream(std::move(archive.m_archiveStream))
	, m_lookStream(std::move(archive.m_lookStream))
	, m_archive(std::move(archive.m_archive))
	, m_allocator(std::move(archive.m_allocator))
	, m_data(std::move(archive.m_data))
	, m_cachedExtractionData(std::move(archive.m_cachedExtractionData))
	, m_filePath(std::move(archive.m_filePath))
	, m_entries(std::move(archive.m_entries))
	, m_numberOfFiles(archive.m_numberOfFiles)
	, m_numberOfDirectories(archive.m_numberOfDirectories)
	, m_compressedSize(archive.m_compressedSize) {
	updateParentArchive();
}

const SevenZipArchive & SevenZipArchive::operator = (SevenZipArchive && archive) noexcept {
	if(this != &archive) {
		Archive::operator = (std::move(archive));

		m_archiveStream = std::move(archive.m_archiveStream);
		m_lookStream = std::move(archive.m_lookStream);
		m_archive = std::move(archive.m_archive);
		m_allocator = std::move(archive.m_allocator);
		m_data = std::move(archive.m_data);
		m_cachedExtractionData = std::move(archive.m_cachedExtractionData);
		m_filePath = std::move(archive.m_filePath);
		m_entries = std::move(archive.m_entries);
		m_numberOfFiles = archive.m_numberOfFiles;
		m_numberOfDirectories = archive.m_numberOfDirectories;
		m_compressedSize = archive.m_compressedSize;

		updateParentArchive();
	}

	return *this;
}

SevenZipArchive::~SevenZipArchive() {
	for(std::shared_ptr<SevenZipArchive::Entry> & entry : m_entries) {
		entry->clearParentArchive();
	}

	m_archive.reset();
	m_lookStream.reset();
	m_archiveStream.reset();
}

std::string SevenZipArchive::getDefaultFileExtension() const {
	return DEFAULT_FILE_EXTENSION;
}

std::string SevenZipArchive::getFilePath() const {
	return m_filePath;
}

void SevenZipArchive::setFilePath(const std::string & filePath) {
	m_filePath = filePath;
}

bool SevenZipArchive::hasComment() const {
	return false;
}

std::string SevenZipArchive::getComment() const {
	return "";
}

uint64_t SevenZipArchive::getCompressedSize() const {
	return m_compressedSize;
}

size_t SevenZipArchive::numberOfEntries() const {
	return m_entries.size();
}

size_t SevenZipArchive::numberOfFiles() const {
	return m_numberOfFiles;
}

size_t SevenZipArchive::numberOfDirectories() const {
	return m_numberOfDirectories;
}

std::string SevenZipArchive::toDebugString(bool includeDate) const {
	std::stringstream stringStream;

	stringStream << fmt::format("File Path: '{}'\n", m_filePath);
	stringStream << fmt::format("Number of Entries: {} (Files: {}, Directories: {})\n", numberOfEntries(), m_numberOfFiles, m_numberOfDirectories);
	stringStream << fmt::format("Compressed Size: {}\n", getCompressedSize());

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		if(i != m_entries.begin()) {
			stringStream << "\n";
		}

		stringStream << fmt::format("{}. '{}' Size: {}", (*i)->getIndex(), (*i)->getPath(), (*i)->getUncompressedSize());

		if(includeDate) {
			stringStream << fmt::format(" Date: {}", Utilities::timePointToString((*i)->getDate()));
		}
	}

	return stringStream.str();
}

bool SevenZipArchive::is7ZipArchive(const std::string & filePath) {
	static constexpr size_t MAX_7ZIP_MAGIC_NUMBER_LENGTH = 6;

	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	std::ifstream fileStream(filePath, std::ios::binary);

	if(!fileStream.is_open()) {
		return false;
	}

	ByteBuffer buffer(MAX_7ZIP_MAGIC_NUMBER_LENGTH, Endianness::LittleEndian);
	buffer.resize(MAX_7ZIP_MAGIC_NUMBER_LENGTH);

	fileStream.read(reinterpret_cast<char *>(buffer.getData().data()), MAX_7ZIP_MAGIC_NUMBER_LENGTH);

	const bool endOfFile = fileStream.eof();

	fileStream.close();

	if(endOfFile) {
		return false;
	}

	return is7ZipArchive(buffer);
}

bool SevenZipArchive::is7ZipArchive(const ByteBuffer & data) {
	static const std::array<uint8_t, 6> SEVEN_ZIP_MAGIC_NUMBER({ 0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C }); // 7z

	if(data.getSize() < SEVEN_ZIP_MAGIC_NUMBER.size()) {
		return false;
	}

	return std::memcmp(data.getRawData(), SEVEN_ZIP_MAGIC_NUMBER.data(), SEVEN_ZIP_MAGIC_NUMBER.size()) == 0;
}

std::unique_ptr<SevenZipArchive> SevenZipArchive::readFrom(const std::string & filePath) {
	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::error("Failed to read 7-Zip archive from non-existent file: '{}'!", filePath);
		return nullptr;
	}

	ArchiveStreamHandle archiveStream(createArchiveStreamHandle());

	if(InFile_Open(&archiveStream->file, filePath.c_str()) != 0) {
		spdlog::error("Failed to open 7-Zip archive file: '{}'!", filePath);
		return nullptr;
	}

	FileInStream_CreateVTable(archiveStream.get());
	archiveStream->wres = 0;

	std::error_code errorCode;
	return createFrom(std::move(archiveStream), filePath, nullptr, std::filesystem::file_size(std::filesystem::path(filePath), errorCode));
}

std::unique_ptr<SevenZipArchive> SevenZipArchive::createFrom(std::unique_ptr<ByteBuffer> data) {
	std::unique_ptr<CFileInStream> archiveStream(std::make_unique<CFileInStream>());
	createVirtualByteBufferFileHandle(archiveStream->file, *data);
	archiveStream->wres = 0;
	archiveStream->vt.Read = readFromVirtualByteBufferFile;
	archiveStream->vt.Seek = seekVirtualByteBufferFile;

	uint64_t compressedSize = data->getSize();
	return createFrom(std::move(archiveStream), Utilities::emptyString, std::move(data), compressedSize);
}

std::unique_ptr<SevenZipArchive> SevenZipArchive::createFrom(ArchiveStreamHandle archiveStream, const std::string & filePath, std::unique_ptr<ByteBuffer> data, uint64_t compressedSize) {
	AllocatorHandle allocator(createAllocatorHandle(DEFAULT_ALLOCATOR));

	LookStreamHandle lookStream(createLookStreamHandle(*allocator));

	if(lookStream == nullptr) {
		spdlog::error("Failed to allocate 7-Zip look stream buffer.");
		return nullptr;
	}

	lookStream->realStream = &archiveStream->vt;
	LookToRead2_Init(lookStream.get());

	CrcGenerateTable();

	ArchiveHandle archive(createArchiveHandle(*allocator));

	ISzAlloc temporaryAllocator = DEFAULT_ALLOCATOR;

	if(SzArEx_Open(archive.get(), &lookStream->vt, allocator.get(), &temporaryAllocator) != SZ_OK) {
		spdlog::error("Failed to open 7-Zip archive!");
		return nullptr;
	}

	return std::unique_ptr<SevenZipArchive>(new SevenZipArchive(std::move(archiveStream), std::move(lookStream), std::move(archive), std::move(allocator), filePath, std::move(data), compressedSize));
}

std::vector<std::shared_ptr<ArchiveEntry>> SevenZipArchive::getEntries() const {
	std::vector<std::shared_ptr<ArchiveEntry>> entries(m_entries.size());
	std::copy(std::begin(m_entries), std::end(m_entries), std::begin(entries));

	return entries;
}

const CFileInStream * SevenZipArchive::getRawArchiveStreamHandle() const {
	return m_archiveStream.get();
}

CFileInStream * SevenZipArchive::getRawArchiveStreamHandle() {
	return m_archiveStream.get();
}

const CLookToRead2 * SevenZipArchive::getRawLookStreamHandle() const {
	return m_lookStream.get();
}

CLookToRead2 * SevenZipArchive::getRawLookStreamHandle() {
	return m_lookStream.get();
}

const CSzArEx * SevenZipArchive::getRawArchiveHandle() const {
	return m_archive.get();
}

CSzArEx * SevenZipArchive::getRawArchiveHandle() {
	return m_archive.get();
}

const ISzAlloc * SevenZipArchive::getRawAllocatorHandle() const {
	return m_allocator.get();
}

ISzAlloc * SevenZipArchive::getRawAllocatorHandle() {
	return m_allocator.get();
}

SevenZipArchive::ExtractionData & SevenZipArchive::getCachedExtractionData() {
	return m_cachedExtractionData;
}

SevenZipArchive::ArchiveStreamHandle SevenZipArchive::createArchiveStreamHandle() {
	return ArchiveStreamHandle(new CFileInStream(), [](CFileInStream * archiveStreamHandle) {
		if(archiveStreamHandle != nullptr) {
			File_Close(&archiveStreamHandle->file);
			delete archiveStreamHandle;
		}
	});
}

SevenZipArchive::LookStreamHandle SevenZipArchive::createLookStreamHandle(ISzAlloc & allocator) {
	static constexpr size_t LOOK_STREAM_BUFFER_SIZE = 1 << 18;

	CLookToRead2 * lookStream = new CLookToRead2();
	LookToRead2_CreateVTable(lookStream, False);
	lookStream->buf = static_cast<Byte *>(ISzAlloc_Alloc(&allocator, LOOK_STREAM_BUFFER_SIZE));

	if(!lookStream->buf) {
		delete lookStream;
		return nullptr;
	}

	lookStream->bufSize = LOOK_STREAM_BUFFER_SIZE;

	return LookStreamHandle(lookStream, [&allocator](CLookToRead2 * lookStreamHandle) {
		if(lookStreamHandle != nullptr) {
			ISzAlloc_Free(&allocator, lookStreamHandle->buf);
			delete lookStreamHandle;
		}
	});
}

SevenZipArchive::ArchiveHandle SevenZipArchive::createArchiveHandle(ISzAlloc & allocator) {
	CSzArEx * archiveHandle = new CSzArEx();
	SzArEx_Init(archiveHandle);

	return ArchiveHandle(archiveHandle, [&allocator](CSzArEx * archiveHandle) {
		if(archiveHandle != nullptr) {
			SzArEx_Free(archiveHandle, &allocator);
			delete archiveHandle;
		}
	});
}

SevenZipArchive::AllocatorHandle SevenZipArchive::createAllocatorHandle(const ISzAlloc & allocator) {
	return SevenZipArchive::AllocatorHandle(new ISzAlloc(allocator));
}

void SevenZipArchive::updateParentArchive() {
	for(std::shared_ptr<SevenZipArchive::Entry> & entry : m_entries) {
		entry->clearParentArchive();
	}
}
