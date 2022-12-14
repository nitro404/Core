#include "SevenZipArchive.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/TimeUtilities.h"

#include <7Zip/C/7zAlloc.h>
#include <7Zip/C/7zCrc.h>
#include <7Zip/C/7zTypes.h>

#include <spdlog/spdlog.h>

#include <filesystem>

const ISzAlloc SevenZipArchive::DEFAULT_ALLOCATOR = { SzAlloc, SzFree };

static void createVirtualByteBufferFileHandle(CSzFile & file, const ByteBuffer & data) {
#ifdef USE_WINDOWS_FILE
	file.handle = reinterpret_cast<HANDLE>(const_cast<ByteBuffer *>(&data));
#elif defined(USE_FOPEN)
	file.file = reinterpret_cast<FILE *>(const_cast<ByteBuffer *>(&data));
#else
	file.fd = reinterpret_cast<int>(const_cast<ByteBuffer *>(&data));
#endif
}

static const ByteBuffer * getByteBufferFromSeekInStreamInterface(const ISeekInStream * seekInStreamInterface) {
	CFileInStream * fileInStream = CONTAINER_FROM_VTBL(seekInStreamInterface, CFileInStream, vt);

#ifdef USE_WINDOWS_FILE
	return reinterpret_cast<const ByteBuffer *>(fileInStream->file.handle);
#elif defined(USE_FOPEN)
	return reinterpret_cast<const ByteBuffer *>(fileInStream->file.file);
#else
	return reinterpret_cast<const ByteBuffer *>(fileInStream->file.fd);
#endif
}

static SRes readFromVirtualByteBufferFile(const ISeekInStream * seekInStreamInterface, void * outputBuffer, size_t * numberOfBytesToRead) {
	const ByteBuffer * data = getByteBufferFromSeekInStreamInterface(seekInStreamInterface);

	size_t numberOfBytesRead = std::min(*numberOfBytesToRead, data->getRemainingBytes());
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

SevenZipArchive::SevenZipArchive(ArchiveStreamHandle archiveStream, LookStreamHandle lookStream, ArchiveHandle archive, AllocatorHandle allocator, const std::string & filePath, std::unique_ptr<ByteBuffer> data)
	: Archive(Type::SevenZip)
	, m_archiveStream(std::move(archiveStream))
	, m_lookStream(std::move(lookStream))
	, m_archive(std::move(archive))
	, m_allocator(std::move(allocator))
	, m_filePath(filePath)
	, m_numberOfFiles(0)
	, m_numberOfDirectories(0)
	, m_data(std::move(data)) {
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

SevenZipArchive::~SevenZipArchive() {
	for(std::vector<std::shared_ptr<SevenZipArchive::Entry>>::iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		(*i)->clearParentArchive();
	}
}

std::string SevenZipArchive::getDefaultFileExtension() const {
	return DEFAULT_FILE_EXTENSION;
}

std::string SevenZipArchive::getFilePath() const {
	return m_filePath;
}

bool SevenZipArchive::hasComment() const {
	return false;
}

std::string SevenZipArchive::getComment() const {
	return "";
}

uint64_t SevenZipArchive::getCompressedSize() const {
	// Note: 7-Zip does not report the compressed size of archive file entries
	return 0;
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

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		stringStream << fmt::format("{}. '{}' Size: {}", (*i)->getIndex(), (*i)->getPath(), (*i)->getUncompressedSize());

		if(includeDate) {
			stringStream << fmt::format(" Date: {}", Utilities::timePointToString((*i)->getDate()));
		}

		stringStream << "\n";
	}

	return stringStream.str();
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

	return createFrom(std::move(archiveStream), filePath, nullptr);
}

std::unique_ptr<SevenZipArchive> SevenZipArchive::createFrom(std::unique_ptr<ByteBuffer> data) {
	std::unique_ptr<CFileInStream> archiveStream(std::make_unique<CFileInStream>());
	createVirtualByteBufferFileHandle(archiveStream->file, *data);
	archiveStream->wres = 0;
	archiveStream->vt.Read = readFromVirtualByteBufferFile;
	archiveStream->vt.Seek = seekVirtualByteBufferFile;

	return createFrom(std::move(archiveStream), Utilities::emptyString, std::move(data));
}

std::unique_ptr<SevenZipArchive> SevenZipArchive::createFrom(ArchiveStreamHandle archiveStream, const std::string & filePath, std::unique_ptr<ByteBuffer> data) {
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

	return std::unique_ptr<SevenZipArchive>(new SevenZipArchive(std::move(archiveStream), std::move(lookStream), std::move(archive), std::move(allocator), filePath, std::move(data)));
}

std::vector<std::shared_ptr<ArchiveEntry>> SevenZipArchive::getEntries() const {
	std::vector<std::shared_ptr<ArchiveEntry>> entires(m_entries.size());
	std::copy(std::begin(m_entries), std::end(m_entries), std::begin(entires));

	return entires;
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

std::chrono::time_point<std::chrono::system_clock> SevenZipArchive::getTimePointFromNTFSFileTime(const CNtfsFileTime & ntfsFileTime) {
	FILETIME fileTime;
	fileTime.dwLowDateTime = static_cast<DWORD>(ntfsFileTime.Low);
	fileTime.dwHighDateTime = static_cast<DWORD>(ntfsFileTime.High);

	return std::chrono::system_clock::from_time_t(time_t{0}) + std::chrono::milliseconds(((fileTime.dwLowDateTime | (static_cast<UInt64>(fileTime.dwHighDateTime) << 32)) / 10000ULL) - 11644473600000ULL);
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

	return LookStreamHandle(lookStream, [allocator](CLookToRead2 * lookStreamHandle) {
		if(lookStreamHandle != nullptr) {
			ISzAlloc_Free(&allocator, lookStreamHandle->buf);
			delete lookStreamHandle;
		}
	});
}

SevenZipArchive::ArchiveHandle SevenZipArchive::createArchiveHandle(ISzAlloc & allocator) {
	CSzArEx * archiveHandle = new CSzArEx();
	SzArEx_Init(archiveHandle);

	return ArchiveHandle(archiveHandle, [allocator](CSzArEx * archiveHandle) {
		if(archiveHandle != nullptr) {
			SzArEx_Free(archiveHandle, &allocator);
			delete archiveHandle;
		}
	});
}

SevenZipArchive::AllocatorHandle SevenZipArchive::createAllocatorHandle(const ISzAlloc & allocator) {
	return SevenZipArchive::AllocatorHandle(new ISzAlloc(allocator));
}
