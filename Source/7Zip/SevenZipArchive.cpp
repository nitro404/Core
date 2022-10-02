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

SevenZipArchive::SevenZipArchive(ArchiveStreamHandle archiveStream, LookStreamHandle lookStream, ArchiveHandle archive, AllocatorHandle allocator, const std::string & filePath)
	: m_archiveStream(std::move(archiveStream))
	, m_lookStream(std::move(lookStream))
	, m_archive(std::move(archive))
	, m_allocator(std::move(allocator))
	, m_filePath(filePath)
	, m_numberOfFiles(0)
	, m_numberOfDirectories(0) {
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
	for(std::vector<std::shared_ptr<Entry>>::iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		(*i)->clearParentArchive();
	}
}

std::string SevenZipArchive::getFilePath() const {
	return m_filePath;
}

uint64_t SevenZipArchive::getInflatedSize() const {
	uint64_t inflatedSize = 0;

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.cbegin(); i != m_entries.cend(); ++i) {
		inflatedSize += (*i)->getInflatedSize();
	}

	return inflatedSize;
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

bool SevenZipArchive::hasEntry(const Entry & entry) const {
	return entry.getParentArchive() == this &&
		   entry.getIndex() < m_entries.size() &&
		   m_entries[entry.getIndex()] != nullptr &&
		   m_entries[entry.getIndex()].get() == &entry;
}

bool SevenZipArchive::hasEntry(const std::string & entryPath, bool caseSensitive) const {
	return indexOfEntry(entryPath, caseSensitive) != std::numeric_limits<size_t>::max();
}

bool SevenZipArchive::hasEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	return indexOfFirstEntryWithName(entryName, includeSubdirectories, caseSensitive) != std::numeric_limits<size_t>::max();
}

size_t SevenZipArchive::indexOfEntry(const std::string & entryPath, bool caseSensitive) const {
	if(entryPath.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(Utilities::areStringsEqual(Utilities::trimTrailingPathSeparator((*i)->getPath()), Utilities::trimTrailingPathSeparator(entryPath), caseSensitive)) {
			return i - m_entries.begin();
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t SevenZipArchive::indexOfFirstEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	if(entryName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(!includeSubdirectories && (*i)->isInSubdirectory()) {
			continue;
		}

		if(Utilities::areStringsEqual(Utilities::trimTrailingPathSeparator((*i)->getName()), Utilities::trimTrailingPathSeparator(entryName), caseSensitive)) {
			return i - m_entries.begin();
		}
	}

	return std::numeric_limits<size_t>::max();
}

const std::weak_ptr<SevenZipArchive::Entry> SevenZipArchive::getEntry(const std::string & entryPath, bool caseSensitive) const {
	return getEntry(indexOfEntry(entryPath, caseSensitive));
}

std::weak_ptr<SevenZipArchive::Entry> SevenZipArchive::getEntry(const std::string & entryPath, bool caseSensitive) {
	return getEntry(indexOfEntry(entryPath, caseSensitive));
}

std::weak_ptr<SevenZipArchive::Entry> SevenZipArchive::getFirstEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	return getEntry(indexOfFirstEntryWithName(entryName, includeSubdirectories, caseSensitive));
}

const std::weak_ptr<SevenZipArchive::Entry> SevenZipArchive::getEntry(size_t index) const {
	if(index >= m_entries.size()) {
		return std::weak_ptr<Entry>();
	}

	return m_entries[index];
}

std::weak_ptr<SevenZipArchive::Entry> SevenZipArchive::getEntry(size_t index) {
	if(index >= m_entries.size()) {
		return std::weak_ptr<Entry>();
	}

	return m_entries[index];
}

size_t SevenZipArchive::extractAllEntries(const std::string & directoryPath, bool overwrite) const {
	std::error_code errorCode;

	if(!directoryPath.empty()) {
		std::filesystem::path outputDirectoryPath(directoryPath);

		if(!std::filesystem::is_directory(outputDirectoryPath)) {
			std::filesystem::create_directories(outputDirectoryPath, errorCode);

			if(errorCode) {
				spdlog::error("Cannot extract files from 7-Zip archive, output directory '{}' creation failed: {}", outputDirectoryPath.string(), errorCode.message());
				return false;
			}
		}
	}

	size_t numberOfExtractedFileEntries = 0;

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		std::filesystem::path currentEntryDestinationPath(Utilities::joinPaths(directoryPath, (*i)->getPath()));

		if((*i)->isDirectory()) {
			if(!std::filesystem::is_directory(currentEntryDestinationPath)) {
				std::filesystem::create_directories(currentEntryDestinationPath, errorCode);

				if(errorCode) {
					spdlog::error("Cannot extract files from 7-Zip archive, entry directory '{}' creation failed: {}", currentEntryDestinationPath.string(), errorCode.message());
					return false;
				}
			}
		}
		else if((*i)->isFile()) {
			if(!overwrite && std::filesystem::is_regular_file(currentEntryDestinationPath)) {
				spdlog::warn("Skipping extraction of file from archive, destination file '{}' already exists! Did you intend to specify the overwrite flag?", currentEntryDestinationPath.string());
				continue;
			}

			if((*i)->writeTo(directoryPath, overwrite)) {
				numberOfExtractedFileEntries++;

				spdlog::debug("Extracted 7-Zip file entry #{}/{} to: '{}'.", numberOfExtractedFileEntries, m_numberOfFiles, currentEntryDestinationPath.string());
			}
		}
	}

	return numberOfExtractedFileEntries;
}

std::string SevenZipArchive::toDebugString(bool includeDate) const {
	std::stringstream stringStream;

	stringStream << fmt::format("File Path: '{}'\n", m_filePath);
	stringStream << fmt::format("Number of Entries: {} (Files: {}, Directories: {})\n", numberOfEntries(), m_numberOfFiles, m_numberOfDirectories);

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		stringStream << fmt::format("{}. '{}' Size: {}", (*i)->getIndex(), (*i)->getPath(), (*i)->getInflatedSize());

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

	return std::unique_ptr<SevenZipArchive>(new SevenZipArchive(std::move(archiveStream), std::move(lookStream), std::move(archive), std::move(allocator), filePath));
}

const std::vector<std::shared_ptr<SevenZipArchive::Entry>> & SevenZipArchive::getEntries() const {
	return m_entries;
}

std::vector<std::shared_ptr<SevenZipArchive::Entry>> & SevenZipArchive::getEntries() {
	return m_entries;
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
