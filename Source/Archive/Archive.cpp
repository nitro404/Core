#include "Archive.h"

#include "Utilities/FileUtilities.h"

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

bool Archive::hasEntryWithExtension(const std::string & extension, bool includeSubdirectories, bool caseSensitive) const {
	return indexOfFirstEntryWithExtension(extension, includeSubdirectories, caseSensitive) != std::numeric_limits<size_t>::max();
}

bool Archive::hasEntryWithExtension(const std::vector<std::string> & extensions, bool includeSubdirectories, bool caseSensitive) const {
	return indexOfFirstEntryWithExtension(extensions, includeSubdirectories, caseSensitive) != std::numeric_limits<size_t>::max();
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

size_t Archive::indexOfLastEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	if(entryName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());

	for(std::vector<std::shared_ptr<ArchiveEntry>>::const_reverse_iterator i = entries.crbegin(); i != entries.crend(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(!includeSubdirectories && (*i)->isInSubdirectory()) {
			continue;
		}

		if(Utilities::areStringsEqual(Utilities::trimTrailingPathSeparator((*i)->getName()), Utilities::trimTrailingPathSeparator(entryName), caseSensitive)) {
			return entries.crend() - i - 1;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Archive::indexOfFirstEntryWithExtension(const std::string & extension, bool includeSubdirectories, bool caseSensitive) const {
	if(extension.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());

	for(std::vector<std::shared_ptr<ArchiveEntry>>::const_iterator i = entries.cbegin(); i != entries.cend(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(!includeSubdirectories && (*i)->isInSubdirectory()) {
			continue;
		}

		if(Utilities::areStringsEqual((*i)->getFileExtension(), extension, caseSensitive)) {
			return i - entries.cbegin();
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Archive::indexOfFirstEntryWithExtension(const std::vector<std::string> & extensions, bool includeSubdirectories, bool caseSensitive) const {
	if(extensions.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());

	for(std::vector<std::shared_ptr<ArchiveEntry>>::const_iterator i = entries.cbegin(); i != entries.cend(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(!includeSubdirectories && (*i)->isInSubdirectory()) {
			continue;
		}

		for(const std::string & extension : extensions) {
			if(extension.empty()) {
				continue;
			}

			if(Utilities::areStringsEqual((*i)->getFileExtension(), extension, caseSensitive)) {
				return i - entries.cbegin();
			}
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Archive::indexOfLastEntryWithExtension(const std::string & extension, bool includeSubdirectories, bool caseSensitive) const {
	if(extension.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());

	for(std::vector<std::shared_ptr<ArchiveEntry>>::const_reverse_iterator i = entries.crbegin(); i != entries.crend(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(!includeSubdirectories && (*i)->isInSubdirectory()) {
			continue;
		}

		if(Utilities::areStringsEqual((*i)->getFileExtension(), extension, caseSensitive)) {
			return entries.crend() - i - 1;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Archive::indexOfLastEntryWithExtension(const std::vector<std::string> & extensions, bool includeSubdirectories, bool caseSensitive) const {
	if(extensions.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());

	for(std::vector<std::shared_ptr<ArchiveEntry>>::const_reverse_iterator i = entries.crbegin(); i != entries.crend(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(!includeSubdirectories && (*i)->isInSubdirectory()) {
			continue;
		}

		for(const std::string & extension : extensions) {
			if(extension.empty()) {
				continue;
			}

			if(Utilities::areStringsEqual((*i)->getFileExtension(), extension, caseSensitive)) {
				return entries.crend() - i - 1;
			}
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

std::shared_ptr<ArchiveEntry> Archive::getLastEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	return getEntry(indexOfLastEntryWithName(entryName, includeSubdirectories, caseSensitive));
}

std::shared_ptr<ArchiveEntry> Archive::getFirstEntryWithExtension(const std::string & extension, bool includeSubdirectories, bool caseSensitive) const {
	return getEntry(indexOfFirstEntryWithExtension(extension, includeSubdirectories, caseSensitive));
}

std::shared_ptr<ArchiveEntry> Archive::getFirstEntryWithExtension(const std::vector<std::string> & extensions, bool includeSubdirectories, bool caseSensitive) const {
	return getEntry(indexOfFirstEntryWithExtension(extensions, includeSubdirectories, caseSensitive));
}

std::shared_ptr<ArchiveEntry> Archive::getLastEntryWithExtension(const std::string & extension, bool includeSubdirectories, bool caseSensitive) const {
	return getEntry(indexOfLastEntryWithExtension(extension, includeSubdirectories, caseSensitive));
}

std::shared_ptr<ArchiveEntry> Archive::getLastEntryWithExtension(const std::vector<std::string> & extensions, bool includeSubdirectories, bool caseSensitive) const {
	return getEntry(indexOfLastEntryWithExtension(extensions, includeSubdirectories, caseSensitive));
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

std::vector<std::shared_ptr<ArchiveEntry>> Archive::getRootEntries() const {
	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());
	std::vector<std::shared_ptr<ArchiveEntry>> rootEntries;

	for(const std::shared_ptr<ArchiveEntry> & entry : entries) {
		if(entry == nullptr) {
			continue;
		}

		if(!entry->isDirectory() && !entry->isInSubdirectory()) {
			rootEntries.push_back(entry);
		}
	}

	return rootEntries;
}

std::vector<std::shared_ptr<ArchiveEntry>> Archive::getEntriesWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	if(entryName.empty()) {
		return {};
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());
	std::vector<std::shared_ptr<ArchiveEntry>> entriesWithName;

	for(const std::shared_ptr<ArchiveEntry> & entry : entries) {
		if(entry == nullptr) {
			continue;
		}

		if(!includeSubdirectories && entry->isInSubdirectory()) {
			continue;
		}

		if(Utilities::areStringsEqual(entry->getName(), entryName, caseSensitive)) {
			entriesWithName.push_back(entry);
		}
	}

	return entriesWithName;
}

std::vector<std::shared_ptr<ArchiveEntry>> Archive::getEntriesWithExtension(const std::string & extension, bool includeSubdirectories, bool caseSensitive) const {
	if(extension.empty()) {
		return {};
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());
	std::vector<std::shared_ptr<ArchiveEntry>> entriesWithExtension;

	for(std::shared_ptr<ArchiveEntry> & entry : entries) {
		if(entry == nullptr) {
			continue;
		}

		if(!includeSubdirectories && entry->isInSubdirectory()) {
			continue;
		}

		if(Utilities::areStringsEqualIgnoreCase(entry->getFileExtension(), extension)) {
			entriesWithExtension.push_back(entry);
		}
	}

	return entriesWithExtension;
}

std::vector<std::shared_ptr<ArchiveEntry>> Archive::getEntriesWithExtensions(const std::vector<std::string> & extensions, bool includeSubdirectories, bool caseSensitive) const {
	if(extensions.empty()) {
		return {};
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());
	std::vector<std::shared_ptr<ArchiveEntry>> entriesWithExtension;

	for(std::shared_ptr<ArchiveEntry> & entry : entries) {
		if(entry == nullptr) {
			continue;
		}

		if(!includeSubdirectories && entry->isInSubdirectory()) {
			continue;
		}

		for(const std::string & extension : extensions) {
			if(extension.empty()) {
				continue;
			}

			if(Utilities::areStringsEqualIgnoreCase(entry->getFileExtension(), extension)) {
				entriesWithExtension.push_back(entry);
				break;
			}
		}
	}

	return entriesWithExtension;
}

std::vector<std::shared_ptr<ArchiveEntry>> Archive::getEntriesInDirectory(const std::string & directoryPath, bool includeSubdirectories, bool caseSensitive) const {
	if(directoryPath.empty()) {
		return getRootEntries();
	}

	std::string_view formattedDirectoryPath;

	if(directoryPath[0] == '/' || directoryPath[0] == '\\') {
		formattedDirectoryPath = directoryPath.substr(0, directoryPath.length() - 1);
	}
	else {
		formattedDirectoryPath = directoryPath;
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());
	std::vector<std::shared_ptr<ArchiveEntry>> entriesInDirectory;

	for(const std::shared_ptr<ArchiveEntry> & entry : entries) {
		if(entry == nullptr) {
			continue;
		}

		std::string entryBasePath(entry->getBasePath());

		if(includeSubdirectories) {
			if(entryBasePath.length() < formattedDirectoryPath.length()) {
				continue;
			}

			if(Utilities::areStringsEqual(entryBasePath.substr(0, formattedDirectoryPath.length()), formattedDirectoryPath, caseSensitive)) {
				entriesInDirectory.push_back(entry);
			}
		}
		else {
			if(Utilities::areStringsEqual(entryBasePath, formattedDirectoryPath, caseSensitive)) {
				entriesInDirectory.push_back(entry);
			}
		}
	}

	return entriesInDirectory;
}

std::shared_ptr<ArchiveEntry> Archive::extractFirstEntryWithExtension(const std::string & extension, const std::string & directory, bool overwrite, bool includeSubdirectories, bool caseSensitive) const {
	std::shared_ptr<ArchiveEntry> entry(getFirstEntryWithExtension(extension, includeSubdirectories, caseSensitive));

	if(entry != nullptr && entry->writeToFile(directory, overwrite)) {
		return entry;
	}

	return nullptr;
}

std::shared_ptr<ArchiveEntry> Archive::extractFirstEntryWithExtension(const std::vector<std::string> & extensions, const std::string & directory, bool overwrite, bool includeSubdirectories, bool caseSensitive) const {
	std::shared_ptr<ArchiveEntry> entry(getFirstEntryWithExtension(extensions, includeSubdirectories, caseSensitive));

	if(entry != nullptr && entry->writeToFile(directory, overwrite)) {
		return entry;
	}

	return nullptr;
}

std::shared_ptr<ArchiveEntry> Archive::extractLastEntryWithExtension(const std::string & extension, const std::string & directory, bool overwrite, bool includeSubdirectories, bool caseSensitive) const {
	std::shared_ptr<ArchiveEntry> entry(getLastEntryWithExtension(extension, includeSubdirectories, caseSensitive));

	if(entry != nullptr && entry->writeToFile(directory, overwrite)) {
		return entry;
	}

	return nullptr;
}

std::shared_ptr<ArchiveEntry> Archive::extractLastEntryWithExtension(const std::vector<std::string> & extensions, const std::string & directory, bool overwrite, bool includeSubdirectories, bool caseSensitive) const {
	std::shared_ptr<ArchiveEntry> entry(getLastEntryWithExtension(extensions, includeSubdirectories, caseSensitive));

	if(entry != nullptr && entry->writeToFile(directory, overwrite)) {
		return entry;
	}

	return nullptr;
}

std::vector<std::shared_ptr<ArchiveEntry>> Archive::extractAllEntriesWithExtension(const std::string & extension, const std::string & directory, bool overwrite, bool includeSubdirectories, bool caseSensitive) const {
	if(extension.empty()) {
		return {};
	}

	std::shared_ptr<ArchiveEntry> entry;
	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());
	std::vector<std::shared_ptr<ArchiveEntry>> entriesWithExtension(getEntriesWithExtension(extension));
	std::vector<std::shared_ptr<ArchiveEntry>> extractedFiles;

	for(const std::shared_ptr<ArchiveEntry> & entry : entries) {
		if(entry == nullptr) {
			continue;
		}

		if(!includeSubdirectories && entry->isInSubdirectory()) {
			continue;
		}

		if(Utilities::areStringsEqual(entry->getFileExtension(), extension, caseSensitive)) {
			if(entry->writeToFile(directory, overwrite)) {
				extractedFiles.push_back(entry);
			}
		}
	}

	return extractedFiles;
}

std::vector<std::shared_ptr<ArchiveEntry>> Archive::extractAllEntriesWithExtensions(const std::vector<std::string> & extensions, const std::string & directory, bool overwrite, bool includeSubdirectories, bool caseSensitive) const {
	if(extensions.empty()) {
		return {};
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());
	std::vector<std::shared_ptr<ArchiveEntry>> extractedFiles;

	for(const std::shared_ptr<ArchiveEntry> & entry : entries) {
		if(entry == nullptr) {
			continue;
		}

		if(!includeSubdirectories && entry->isInSubdirectory()) {
			continue;
		}

		for(const std::string & extension : extensions) {
			if(extension.empty()) {
				continue;
			}

			if(Utilities::areStringsEqual(entry->getFileExtension(), extension, caseSensitive)) {
				if(entry->writeToFile(directory, overwrite)) {
					extractedFiles.push_back(entry);
				}

				break;
			}
		}
	}

	return extractedFiles;
}

size_t Archive::extractAllEntries(const std::string & destionationDirectoryPath, bool includeSubdirectories, bool overwrite) {
	if(!isOpen()) {
		return 0;
	}

	std::error_code errorCode;

	if(!destionationDirectoryPath.empty()) {
		std::filesystem::path outputDirectoryPath(destionationDirectoryPath);

		if(!destionationDirectoryPath.empty() && !std::filesystem::is_directory(outputDirectoryPath)) {
			std::filesystem::create_directories(outputDirectoryPath, errorCode);

			if(errorCode) {
				spdlog::error("Cannot extract files from archive, output directory '{}' creation failed: {}", outputDirectoryPath.string(), errorCode.message());
				return false;
			}
		}
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entries(getEntries());
	size_t numberOfExtractedFileEntries = 0;

	for(const std::shared_ptr<ArchiveEntry> & entry : entries) {
		if(entry == nullptr) {
			continue;
		}
		
		if(!includeSubdirectories && entry->isInSubdirectory()) {
			continue;
		}

		std::filesystem::path currentEntryDestinationPath(Utilities::joinPaths(destionationDirectoryPath, entry->getPath()));

		if(entry->isDirectory()) {
			if(!std::filesystem::is_directory(currentEntryDestinationPath)) {
				std::filesystem::create_directories(currentEntryDestinationPath, errorCode);

				if(errorCode) {
					spdlog::error("Cannot extract files from archive, entry directory '{}' creation failed: {}", currentEntryDestinationPath.string(), errorCode.message());
					return false;
				}
			}
		}
		else if(entry->isFile()) {
			if(!overwrite && std::filesystem::is_regular_file(currentEntryDestinationPath)) {
				spdlog::warn("Skipping extraction of file from archive, destination file '{}' already exists! Did you intend to specify the overwrite flag?", currentEntryDestinationPath.string());
				continue;
			}

			if(entry->writeToDirectory(destionationDirectoryPath, overwrite)) {
				numberOfExtractedFileEntries++;

				spdlog::debug("Extracted file entry #{}/{} to: '{}'.", numberOfExtractedFileEntries, numberOfFiles(), currentEntryDestinationPath.string());
			}
		}
	}

	return numberOfExtractedFileEntries;
}

size_t Archive::extractAllEntriesInSubdirectory(const std::string & destionationDirectoryPath, const std::string & archiveSubdirectory, bool relativeToSubdirectory, bool includeSubdirectories, bool overwrite, bool caseSensitive) {
	if(!isOpen()) {
		return 0;
	}

	std::error_code errorCode;

	if(!destionationDirectoryPath.empty()) {
		std::filesystem::path outputDirectoryPath(destionationDirectoryPath);

		if(!destionationDirectoryPath.empty() && !std::filesystem::is_directory(outputDirectoryPath)) {
			std::filesystem::create_directories(outputDirectoryPath, errorCode);

			if(errorCode) {
				spdlog::error("Cannot extract files from archive subdirectory '{}', output directory '{}' creation failed: {}", archiveSubdirectory, outputDirectoryPath.string(), errorCode.message());
				return false;
			}
		}
	}

	std::vector<std::shared_ptr<ArchiveEntry>> entriesInDirectory(getEntriesInDirectory(archiveSubdirectory, includeSubdirectories, caseSensitive));
	size_t numberOfExtractedFileEntries = 0;

	for(const std::shared_ptr<ArchiveEntry> & entry : entriesInDirectory) {
		std::filesystem::path currentEntryDestinationPath(Utilities::joinPaths(destionationDirectoryPath, relativeToSubdirectory ? entry->getPath().substr(archiveSubdirectory.length()) : entry->getPath()));

		if(entry->isDirectory()) {
			if(!std::filesystem::is_directory(currentEntryDestinationPath)) {
				std::filesystem::create_directories(currentEntryDestinationPath, errorCode);

				if(errorCode) {
					spdlog::error("Cannot extract files from archive subdirectory '{}', entry directory '{}' creation failed: {}", archiveSubdirectory, currentEntryDestinationPath.string(), errorCode.message());
					return false;
				}
			}
		}
		else if(entry->isFile()) {
			if(!overwrite && std::filesystem::is_regular_file(currentEntryDestinationPath)) {
				spdlog::warn("Skipping extraction of file from archive, destination file '{}' already exists! Did you intend to specify the overwrite flag?", currentEntryDestinationPath.string());
				continue;
			}

			if(entry->writeToFile(currentEntryDestinationPath.string(), overwrite)) {
				numberOfExtractedFileEntries++;

				spdlog::debug("Extracted file entry #{}/{} to: '{}'.", numberOfExtractedFileEntries, entriesInDirectory.size(), currentEntryDestinationPath.string());
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
