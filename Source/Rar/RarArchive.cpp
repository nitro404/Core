#include "RarArchive.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/TimeUtilities.h"

#include <spdlog/spdlog.h>

#include <filesystem>

RarArchive::RarArchive(ArchiveHandle archiveHandle, const std::string & filePath, std::unique_ptr<ByteBuffer> data)
	: m_archiveHandle(std::move(archiveHandle))
	, m_data(std::move(data))
	, m_filePath(filePath)
	, m_numberOfFiles(0)
	, m_numberOfDirectories(0) {
	for(size_t i = 0; i < dmc_unrar_get_file_count(m_archiveHandle.get()); i++) {
		m_entries.emplace_back(new Entry(i, this));

		if(m_entries[i]->isDirectory()) {
			m_numberOfDirectories++;
		}
		else {
			m_numberOfFiles++;
		}
	}
}

RarArchive::~RarArchive() {
	for(std::vector<std::shared_ptr<Entry>>::iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		(*i)->clearParentArchive();
	}
}

std::string RarArchive::getFilePath() const {
	return m_filePath;
}

bool RarArchive::hasComment() const {
	return dmc_unrar_get_archive_comment(m_archiveHandle.get(), nullptr, 0) != 0;
}

std::string RarArchive::getComment() const {
	size_t commentLength = dmc_unrar_get_archive_comment(m_archiveHandle.get(), nullptr, 0);

	if(commentLength == 0) {
		return {};
	}

	// Note: Dr. McCoy's Un-Rar library doesn't appear to return archive comments

	std::string comment(commentLength - 1, '\0');
	dmc_unrar_get_archive_comment(m_archiveHandle.get(), const_cast<char *>(comment.c_str()), comment.length() + 1);

	return convertComment(comment);
}

uint64_t RarArchive::getCompressedSize() const {
	uint64_t compressedSize = 0;

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.cbegin(); i != m_entries.cend(); ++i) {
		compressedSize += (*i)->getCompressedSize();
	}

	return compressedSize;
}

uint64_t RarArchive::getUncompressedSize() const {
	uint64_t uncompressedSize = 0;

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.cbegin(); i != m_entries.cend(); ++i) {
		uncompressedSize += (*i)->getUncompressedSize();
	}

	return uncompressedSize;
}

size_t RarArchive::numberOfEntries() const {
	return m_entries.size();
}

size_t RarArchive::numberOfFiles() const {
	return m_numberOfFiles;
}

size_t RarArchive::numberOfDirectories() const {
	return m_numberOfDirectories;
}

bool RarArchive::hasEntry(const Entry & entry) const {
	return entry.getParentArchive() == this &&
		   entry.getIndex() < m_entries.size() &&
		   m_entries[entry.getIndex()] != nullptr &&
		   m_entries[entry.getIndex()].get() == &entry;
}

bool RarArchive::hasEntry(const std::string & entryPath, bool caseSensitive) const {
	return indexOfEntry(entryPath, caseSensitive) != std::numeric_limits<size_t>::max();
}

bool RarArchive::hasEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	return indexOfFirstEntryWithName(entryName, includeSubdirectories, caseSensitive) != std::numeric_limits<size_t>::max();
}

size_t RarArchive::indexOfEntry(const std::string & entryPath, bool caseSensitive) const {
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

size_t RarArchive::indexOfFirstEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
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

const std::weak_ptr<RarArchive::Entry> RarArchive::getEntry(const std::string & entryPath, bool caseSensitive) const {
	return getEntry(indexOfEntry(entryPath, caseSensitive));
}

std::weak_ptr<RarArchive::Entry> RarArchive::getEntry(const std::string & entryPath, bool caseSensitive) {
	return getEntry(indexOfEntry(entryPath, caseSensitive));
}

std::weak_ptr<RarArchive::Entry> RarArchive::getFirstEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	return getEntry(indexOfFirstEntryWithName(entryName, includeSubdirectories, caseSensitive));
}

const std::weak_ptr<RarArchive::Entry> RarArchive::getEntry(size_t index) const {
	if(index >= m_entries.size()) {
		return std::weak_ptr<Entry>();
	}

	return m_entries[index];
}

std::weak_ptr<RarArchive::Entry> RarArchive::getEntry(size_t index) {
	if(index >= m_entries.size()) {
		return std::weak_ptr<Entry>();
	}

	return m_entries[index];
}

size_t RarArchive::extractAllEntries(const std::string & directoryPath, bool overwrite) const {
	std::error_code errorCode;

	if(!directoryPath.empty()) {
		std::filesystem::path outputDirectoryPath(directoryPath);

		if(!std::filesystem::is_directory(outputDirectoryPath)) {
			std::filesystem::create_directories(outputDirectoryPath, errorCode);

			if(errorCode) {
				spdlog::error("Cannot extract files from Rar archive, output directory '{}' creation failed: {}", outputDirectoryPath.string(), errorCode.message());
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
					spdlog::error("Cannot extract files from Rar archive, entry directory '{}' creation failed: {}", currentEntryDestinationPath.string(), errorCode.message());
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

				spdlog::debug("Extracted Rar file entry #{}/{} to: '{}'.", numberOfExtractedFileEntries, m_numberOfFiles, currentEntryDestinationPath.string());
			}
		}
	}

	return numberOfExtractedFileEntries;
}

std::string RarArchive::toDebugString(bool includeDate) const {
	std::stringstream stringStream;

	stringStream << fmt::format("File Path: '{}'\n", m_filePath);
	stringStream << fmt::format("Number of Entries: {} (Files: {}, Directories: {})\n", numberOfEntries(), m_numberOfFiles, m_numberOfDirectories);
	stringStream << fmt::format("Compressed Size: {}\n", getCompressedSize());
	stringStream << fmt::format("Uncompressed Size: {}", getUncompressedSize());

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		stringStream << "\n";

		stringStream << fmt::format("{}. '{}' Size: {}", (*i)->getIndex(), (*i)->getPath(), (*i)->getUncompressedSize());

		if(includeDate) {
			stringStream << fmt::format(" Date: {}", Utilities::timePointToString((*i)->getDate()));
		}
	}

	return stringStream.str();
}

std::unique_ptr<RarArchive> RarArchive::readFrom(const std::string & filePath) {
	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::error("Failed to read Rar archive from non-existent file: '{}'!", filePath);
		return nullptr;
	}

	ArchiveHandle archiveHandle(createArchiveHandle());

	if(archiveHandle == nullptr || !isSuccess(dmc_unrar_archive_open_path(archiveHandle.get(), filePath.c_str()), fmt::format("Failed to open Rar archive file: '{}'.", filePath))) {
		return nullptr;
	}

	return std::unique_ptr<RarArchive>(new RarArchive(std::move(archiveHandle), filePath, nullptr));
}

std::unique_ptr<RarArchive> RarArchive::createFrom(std::unique_ptr<ByteBuffer> data) {
	if(data == nullptr) {
		return nullptr;
	}

	ArchiveHandle archiveHandle(createArchiveHandle());

	if(archiveHandle == nullptr || !isSuccess(dmc_unrar_archive_open_mem(archiveHandle.get(), data->getRawData(), data->getSize()), "Failed to create Rar archive from binary data.")) {
		return nullptr;
	}

	return std::unique_ptr<RarArchive>(new RarArchive(std::move(archiveHandle), Utilities::emptyString, std::move(data)));
}

const std::vector<std::shared_ptr<RarArchive::Entry>> & RarArchive::getEntries() const {
	return m_entries;
}

std::vector<std::shared_ptr<RarArchive::Entry>> & RarArchive::getEntries() {
	return m_entries;
}

dmc_unrar_archive * RarArchive::getRawArchiveHandle() const {
	return m_archiveHandle.get();
}

bool RarArchive::isSuccess(dmc_unrar_return result, const std::string & errorMessage) {
	if(result == DMC_UNRAR_OK) {
		return true;
	}

	if(!errorMessage.empty()) {
		spdlog::error("{} Error: {}.", errorMessage, dmc_unrar_strerror(result));
	}

	return false;
}

RarArchive::ArchiveHandle RarArchive::createArchiveHandle() {
	dmc_unrar_archive * archiveHandle = new dmc_unrar_archive();

	if(!isSuccess(dmc_unrar_archive_init(archiveHandle), "Failed to initialize Rar archive.")) {
		delete archiveHandle;

		return false;
	}

	return ArchiveHandle(archiveHandle, [](dmc_unrar_archive * archiveHandle) {
		dmc_unrar_archive_close(archiveHandle);

		delete archiveHandle;
	});
}

std::string RarArchive::convertComment(const std::string & comment) {
	const dmc_unrar_unicode_encoding encoding = dmc_unrar_unicode_detect_encoding(comment.c_str(), comment.length() + 1);

	switch(encoding) {
		case DMC_UNRAR_UNICODE_ENCODING_UTF8:
		case DMC_UNRAR_UNICODE_ENCODING_UNKNOWN: {
			return comment;
		}

		case DMC_UNRAR_UNICODE_ENCODING_UTF16LE: {
			std::string utf8Comment(dmc_unrar_unicode_convert_utf16le_to_utf8(comment.c_str(), comment.length() + 1, nullptr, 0) - 1, '\0');
			dmc_unrar_unicode_convert_utf16le_to_utf8(comment.c_str(), comment.length() + 1, const_cast<char *>(utf8Comment.c_str()), utf8Comment.size() + 1);

			return utf8Comment;
		}
	}

	return {};
}
