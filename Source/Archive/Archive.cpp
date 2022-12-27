#include "Archive.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

#include <filesystem>

#include <spdlog/spdlog.h>

Archive::Archive(Type type)
	: m_type(type) { }

Archive::Archive(Archive && a) noexcept
	: m_type(a.m_type) { }

Archive::Archive(const Archive & a)
	: m_type(a.m_type) { }

Archive & Archive::operator = (Archive && a) noexcept {
	if(this != &a) {
		m_type = a.m_type;
	}

	return *this;
}

Archive & Archive::operator = (const Archive & a) {
	m_type = a.m_type;

	return *this;
}

Archive::~Archive() { }

Archive::Type Archive::getType() const {
	return m_type;
}

bool Archive::isOpen() const {
	return true;
}

bool Archive::isModifiable() const {
	return false;
}

uint64_t Archive::getCompressedSize() const {
	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());
	uint64_t compressedSize = 0;

	for(std::vector<std::shared_ptr<ArchiveEntry>>::const_iterator i = entries.cbegin(); i != entries.cend(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		compressedSize += (*i)->getCompressedSize();
	}

	return compressedSize;
}

uint64_t Archive::getUncompressedSize() const {
	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());
	uint64_t uncompressedSize = 0;

	for(std::vector<std::shared_ptr<ArchiveEntry>>::const_iterator i = entries.cbegin(); i != entries.cend(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		uncompressedSize += (*i)->getUncompressedSize();
	}

	return uncompressedSize;
}

size_t Archive::numberOfEntries() const {
	return getEntries().size();
}

size_t Archive::numberOfFiles() const {
	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());

	size_t fileCount = 0;

	for(std::vector<std::shared_ptr<ArchiveEntry>>::const_iterator i = entries.cbegin(); i != entries.cend(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if((*i)->isFile()) {
			fileCount++;
		}
	}

	return fileCount;
}

size_t Archive::numberOfDirectories() const {
	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());

	size_t directoryCount = 0;

	for(std::vector<std::shared_ptr<ArchiveEntry>>::const_iterator i = entries.begin(); i != entries.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if((*i)->isDirectory()) {
			directoryCount++;
		}
	}

	return directoryCount;
}

bool Archive::hasEntry(const ArchiveEntry & entry) const {
	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());

	return entry.getParentArchive() == this &&
		   entry.getIndex() < entries.size() &&
		   entries[entry.getIndex()] != nullptr &&
		   entries[entry.getIndex()].get() == &entry;
}

bool Archive::hasEntry(const std::string & entryPath, bool caseSensitive) const {
	return indexOfEntry(entryPath, caseSensitive) != std::numeric_limits<size_t>::max();
}

bool Archive::hasEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	return indexOfFirstEntryWithName(entryName, includeSubdirectories, caseSensitive) != std::numeric_limits<size_t>::max();
}

size_t Archive::indexOfEntry(const std::string & entryPath, bool caseSensitive) const {
	if(entryPath.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());

	for(std::vector<std::shared_ptr<ArchiveEntry>>::const_iterator i = entries.begin(); i != entries.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(Utilities::areStringsEqual(Utilities::trimTrailingPathSeparator((*i)->getPath()), Utilities::trimTrailingPathSeparator(entryPath), caseSensitive)) {
			return i - entries.begin();
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Archive::indexOfFirstEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	if(entryName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());

	for(std::vector<std::shared_ptr<ArchiveEntry>>::const_iterator i = entries.begin(); i != entries.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(!includeSubdirectories && (*i)->isInSubdirectory()) {
			continue;
		}

		if(Utilities::areStringsEqual(Utilities::trimTrailingPathSeparator((*i)->getName()), Utilities::trimTrailingPathSeparator(entryName), caseSensitive)) {
			return i - entries.begin();
		}
	}

	return std::numeric_limits<size_t>::max();
}

const std::shared_ptr<ArchiveEntry> Archive::getEntry(const std::string & entryPath, bool caseSensitive) const {
	return getEntry(indexOfEntry(entryPath, caseSensitive));
}

std::shared_ptr<ArchiveEntry> Archive::getEntry(const std::string & entryPath, bool caseSensitive) {
	return getEntry(indexOfEntry(entryPath, caseSensitive));
}

std::shared_ptr<ArchiveEntry> Archive::getFirstEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	return getEntry(indexOfFirstEntryWithName(entryName, includeSubdirectories, caseSensitive));
}

const std::shared_ptr<ArchiveEntry> Archive::getEntry(size_t index) const {
	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());

	if(index >= entries.size()) {
		return std::shared_ptr<ArchiveEntry>();
	}

	return entries[index];
}

std::shared_ptr<ArchiveEntry> Archive::getEntry(size_t index) {
	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());

	if(index >= entries.size()) {
		return std::shared_ptr<ArchiveEntry>();
	}

	return entries[index];
}

size_t Archive::extractAllEntries(const std::string & directoryPath, bool overwrite) const {
	if(!isOpen()) {
		return 0;
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());

	std::error_code errorCode;

	if(!directoryPath.empty()) {
		std::filesystem::path outputDirectoryPath(directoryPath);

		if(!directoryPath.empty() && !std::filesystem::is_directory(outputDirectoryPath)) {
			std::filesystem::create_directories(outputDirectoryPath, errorCode);

			if(errorCode) {
				spdlog::error("Cannot extract files from archive, output directory '{}' creation failed: {}", outputDirectoryPath.string(), errorCode.message());
				return false;
			}
		}
	}

	size_t numberOfExtractedFileEntries = 0;

	for(std::vector<std::shared_ptr<ArchiveEntry>>::const_iterator i = entries.begin(); i != entries.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		std::filesystem::path currentEntryDestinationPath(Utilities::joinPaths(directoryPath, (*i)->getPath()));

		if((*i)->isDirectory()) {
			if(!std::filesystem::is_directory(currentEntryDestinationPath)) {
				std::filesystem::create_directories(currentEntryDestinationPath, errorCode);

				if(errorCode) {
					spdlog::error("Cannot extract files from archive, entry directory '{}' creation failed: {}", currentEntryDestinationPath.string(), errorCode.message());
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

				spdlog::debug("Extracted file entry #{}/{} to: '{}'.", numberOfExtractedFileEntries, numberOfFiles(), currentEntryDestinationPath.string());
			}
		}
	}

	return numberOfExtractedFileEntries;
}

void Archive::updateParentArchive() {
	std::shared_ptr<ArchiveEntry> currentArchiveEntry;

	for(size_t i = 0; i < numberOfEntries(); i++) {
		currentArchiveEntry = getEntry(i);

		if(currentArchiveEntry == nullptr) {
			continue;
		}

		currentArchiveEntry->setParentArchive(this);
	}
}
