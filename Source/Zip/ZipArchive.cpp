#include "ZipArchive.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/TimeUtilities.h"
#include "ZipUtilities.h"

#include <fmt/core.h>
#include <magic_enum.hpp>

#include <filesystem>
#include <sstream>

const ZipArchive::EncryptionMethod ZipArchive::DEFAULT_ENCRYPTION_METHOD = EncryptionMethod::AES256;

ZipArchive::ZipArchive(ZipArchiveHandle zipArchiveHandle, std::unique_ptr<SourceBuffer> zipSourceBuffer, const std::string & filePath, const std::string & password)
	: m_archiveHandle(std::move(zipArchiveHandle))
	, m_sourceBuffer(std::move(zipSourceBuffer))
	, m_filePath(filePath)
	, m_password(password)
	, m_compressionMethod(CompressionMethod::Default)
	, m_encryptionMethod(EncryptionMethod::None)
	, m_compressedSize(0)
	, m_numberOfFiles(0)
	, m_numberOfDirectories(0)
	, m_modified(false) { }

ZipArchive::ZipArchive(ZipArchiveHandle zipArchiveHandle, const std::string & filePath, const std::string & password)
	: m_archiveHandle(std::move(zipArchiveHandle))
	, m_filePath(filePath)
	, m_password(password)
	, m_compressionMethod(CompressionMethod::Default)
	, m_encryptionMethod(EncryptionMethod::None)
	, m_compressedSize(0)
	, m_numberOfFiles(0)
	, m_numberOfDirectories(0)
	, m_modified(false) { }

ZipArchive::~ZipArchive() = default;

bool ZipArchive::hasPassword() const {
	return !m_password.empty();
}

const std::string & ZipArchive::getPassword() const {
	return m_password;
}

bool ZipArchive::setPassword(const std::string & password) {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to set the password.\n");
		return false;
	}

	if(!ZipUtilities::isSuccess(zip_set_default_password(m_archiveHandle.get(), m_password.c_str()), "Failed to set zip archive password.")) {
		return nullptr;
	}

	m_modified = true;

	if(password.empty()) {
		setEncryptionMethod(EncryptionMethod::None);
	}
	else if(m_password.empty() && !password.empty()) {
		setEncryptionMethod(DEFAULT_ENCRYPTION_METHOD);
	}

	m_password = password;

	return true;
}

bool ZipArchive::clearPassword() {
	return setPassword(Utilities::emptyString);
}

bool ZipArchive::isCompressed() const {
	return m_compressionMethod != CompressionMethod::Store;
}

ZipArchive::CompressionMethod ZipArchive::getCompressionMethod() const {
	return m_compressionMethod;
}

bool ZipArchive::setCompressionMethod(CompressionMethod compressionMethod) {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to set the compression method.\n");
		return false;
	}

	if(!ZipArchive::isCompressionMethodSupported(compressionMethod)) {
		fmt::print("Failed to change zip archive compression method to unsupported method: '{}'. \n", magic_enum::enum_name(compressionMethod));
		return false;
	}

	if(m_compressionMethod == compressionMethod) {
		return true;
	}

	m_modified = true;

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		if(*i == nullptr || (*i)->isDirectory()) {
			continue;
		}

		if((*i)->setCompressionMethod(compressionMethod)) {
			return false;
		}
	}

	m_compressionMethod = compressionMethod;

	return true;
}

bool ZipArchive::isEncrypted() const {
	return m_encryptionMethod != EncryptionMethod::None;
}

ZipArchive::EncryptionMethod ZipArchive::getEncryptionMethod() const {
	return m_encryptionMethod;
}

bool ZipArchive::setEncryptionMethod(EncryptionMethod encryptionMethod) {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to set encryption method.\n");
		return false;
	}

	if(!isEncryptionMethodSupported(encryptionMethod)) {
		fmt::print("Failed to change zip archive encryption method to unsupported method: '{}'. \n", magic_enum::enum_name(encryptionMethod));
		return false;
	}

	if(m_encryptionMethod == encryptionMethod) {
		return true;
	}

	m_modified = true;

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		if(*i == nullptr || (*i)->isDirectory()) {
			continue;
		}

		if((*i)->setEncryptionMethod(encryptionMethod)) {
			return false;
		}
	}

	m_encryptionMethod = encryptionMethod;

	return true;
}

bool ZipArchive::isCompressionMethodSupported(CompressionMethod compressionMethod, CompressionType compressionType) {
	if(Any(compressionType & CompressionType::Compress) && !zip_compression_method_supported(magic_enum::enum_integer(compressionMethod), 0)) {
		return false;
	}

	if(Any(compressionType & CompressionType::Decompress) && !zip_compression_method_supported(magic_enum::enum_integer(compressionMethod), 1)) {
		return false;
	}

	return true;
}

bool ZipArchive::isEncryptionMethodSupported(EncryptionMethod encryptionMethod, EncryptionType encryptionType) {
	if(Any(encryptionType & EncryptionType::Encrypt) && !zip_encryption_method_supported(magic_enum::enum_integer(encryptionMethod), 0)) {
		return false;
	}

	if(Any(encryptionType & EncryptionType::Decrypt) && !zip_encryption_method_supported(magic_enum::enum_integer(encryptionMethod), 1)) {
		return false;
	}

	return true;
}

std::chrono::time_point<std::chrono::system_clock> ZipArchive::getDate() const {
	return m_date;
}

bool ZipArchive::hasComment() const {
	if(!isOpen()) {
		return false;
	}

	int commentLength = 0;
	return zip_get_archive_comment(m_archiveHandle.get(), &commentLength, ZIP_FL_ENC_GUESS) != nullptr && commentLength != 0;
}

std::string ZipArchive::getComment() const {
	if(!isOpen()) {
		return {};
	}

	int commentLength = 0;
	const char * comment = zip_get_archive_comment(m_archiveHandle.get(), &commentLength, ZIP_FL_ENC_GUESS);

	if(comment == nullptr || commentLength <= 0) {
		return {};
	}

	return std::string(comment, commentLength);
}

bool ZipArchive::setComment(const std::string & comment) {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to set a comment.\n");
		return false;
	}

	if(ZipUtilities::isSuccess(zip_set_archive_comment(m_archiveHandle.get(), comment.data(), comment.length()), "Failed to set zip archive comment.")) {
		m_modified = true;

		return true;
	}

	return false;
}

bool ZipArchive::clearComment() {
	return setComment(Utilities::emptyString);
}

uint64_t ZipArchive::getCompressedSize() const {
	return m_compressedSize;
}

const ByteBuffer * ZipArchive::getData() const {
	if(m_sourceBuffer == nullptr) {
		return nullptr;
	}

	return m_sourceBuffer->getData();
}

size_t ZipArchive::numberOfEntries() const {
	if(!isOpen()) {
		return 0;
	}

	return zip_get_num_entries(m_archiveHandle.get(), 0);
}

size_t ZipArchive::numberOfFiles() const {
	return m_numberOfFiles;
}

size_t ZipArchive::numberOfDirectories() const {
	return m_numberOfDirectories;
}

bool ZipArchive::hasEntry(const Entry & entry) const {
	return entry.getParentArchive() == this &&
		   entry.getIndex() < m_entries.size() &&
		   m_entries[entry.getIndex()] != nullptr &&
		   m_entries[entry.getIndex()].get() == &entry;
}

bool ZipArchive::hasEntry(const std::string & entryPath, bool caseSensitive) const {
	return indexOfEntry(entryPath, caseSensitive) != std::numeric_limits<size_t>::max();
}

bool ZipArchive::hasEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	return indexOfFirstEntryWithName(entryName, includeSubdirectories, caseSensitive) != std::numeric_limits<size_t>::max();
}

size_t ZipArchive::indexOfEntry(const std::string & entryPath, bool caseSensitive) const {
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

size_t ZipArchive::indexOfFirstEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
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

const std::weak_ptr<ZipArchive::Entry> ZipArchive::getEntry(const std::string & entryPath, bool caseSensitive) const {
	return getEntry(indexOfEntry(entryPath, caseSensitive));
}

std::weak_ptr<ZipArchive::Entry> ZipArchive::getEntry(const std::string & entryPath, bool caseSensitive) {
	return getEntry(indexOfEntry(entryPath, caseSensitive));
}

std::weak_ptr<ZipArchive::Entry> ZipArchive::getFirstEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	return getEntry(indexOfFirstEntryWithName(entryName, includeSubdirectories, caseSensitive));
}

const std::weak_ptr<ZipArchive::Entry> ZipArchive::getEntry(size_t index) const {
	if(index >= m_entries.size()) {
		return std::weak_ptr<Entry>();
	}

	return m_entries[index];
}

std::weak_ptr<ZipArchive::Entry> ZipArchive::getEntry(size_t index) {
	if(index >= m_entries.size()) {
		return std::weak_ptr<Entry>();
	}

	return m_entries[index];
}

size_t ZipArchive::extractAllEntries(const std::string & directoryPath, bool overwrite) const {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to extract all entries.\n");
		return 0;
	}

	std::error_code errorCode;

	if(!directoryPath.empty()) {
		std::filesystem::path outputDirectoryPath(directoryPath);

		if(!std::filesystem::is_directory(outputDirectoryPath)) {
			std::filesystem::create_directories(outputDirectoryPath, errorCode);

			if(errorCode) {
				fmt::print("Cannot extract files from zip archive, output directory '{}' creation failed: {}\n", outputDirectoryPath.string(), errorCode.message());
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
					fmt::print("Cannot extract files from zip archive, entry directory '{}' creation failed: {}\n", currentEntryDestinationPath.string(), errorCode.message());
					return false;
				}
			}
		}
		else if((*i)->isFile()) {
			if(!overwrite && std::filesystem::is_regular_file(currentEntryDestinationPath)) {
				fmt::print("Skipping extraction of file from archive, destination file '{}' already exists! Did you intend to specify the overwrite flag?\n", currentEntryDestinationPath.string());
				continue;
			}

			if((*i)->writeTo(directoryPath, overwrite)) {
				numberOfExtractedFileEntries++;

#if _DEBUG
				fmt::print("Extracted zip file entry #{}/{} to: '{}'.\n", numberOfExtractedFileEntries, m_numberOfFiles, currentEntryDestinationPath.string());
#endif
			}
		}
	}

	return numberOfExtractedFileEntries;
}

std::weak_ptr<ZipArchive::Entry> ZipArchive::addFile(const std::string & filePath, const std::string & entryDirectoryPath, bool overwrite) {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to add a file.\n");
		return std::weak_ptr<ZipArchive::Entry>();
	}

	return addData(ByteBuffer::readFrom(filePath), Utilities::joinPaths(entryDirectoryPath, Utilities::getFileName(filePath)), overwrite);
}

std::weak_ptr<ZipArchive::Entry> ZipArchive::addData(std::unique_ptr<ByteBuffer> data, const std::string & entryFilePath, bool overwrite) {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to add data.\n");
		return std::weak_ptr<ZipArchive::Entry>();
	}

	if(entryFilePath.empty()) {
		fmt::print("Cannot add file data without a destination zip entry file path specified.\n");
		return std::weak_ptr<ZipArchive::Entry>();
	}

	std::weak_ptr<ZipArchive::Entry> entryBaseDirectory;

	if(entryFilePath.find_first_of("/\\") != std::string::npos) {
		entryBaseDirectory = addDirectory(std::string(Utilities::getFilePath(entryFilePath)));
	}

	std::unique_ptr<SourceBuffer> zipFileDataSourceBuffer(createZipFileSourceBuffer(std::move(data)));

	if(zipFileDataSourceBuffer == nullptr) {
		fmt::print("Failed to create zip file data source buffer when adding file to zip archive: '{}'.\n", entryFilePath);
		return std::weak_ptr<ZipArchive::Entry>();
	}

	uint64_t zipFlags = ZIP_FL_ENC_GUESS;

	if(overwrite) {
		zipFlags |= ZIP_FL_OVERWRITE;
	}

	std::string formattedEntryFilePath(Utilities::joinPaths(entryBaseDirectory.expired() ? "" : entryBaseDirectory.lock()->getPath(), Utilities::getFileName(entryFilePath)));

	int64_t newZipFileIndex = zip_file_add(m_archiveHandle.get(), formattedEntryFilePath.c_str(), zipFileDataSourceBuffer->getRawSourceHandle(), zipFlags);

	if(newZipFileIndex == -1) {
		fmt::print("Failed to add zip file to archive: '{}'.\n", entryFilePath);
		return std::weak_ptr<ZipArchive::Entry>();
	}

	m_modified = true;

	// zip file source buffer handle ownership is taken over by the zip archive
	zipFileDataSourceBuffer->releaseSourceHandle();

	std::unique_ptr<Entry> newFileEntry(createEntryFromIndex(newZipFileIndex, std::move(zipFileDataSourceBuffer->transferDataBuffer())));

	// zip file source buffer container is now empty and no longer needed
	zipFileDataSourceBuffer.reset();

	if(newFileEntry == nullptr) {
		fmt::print("Failed to create zip file entry for file: '{}'.\n", entryFilePath);
		return std::weak_ptr<ZipArchive::Entry>();
	}

	if(!newFileEntry->setCompressionMethod(m_compressionMethod)) {
		return std::weak_ptr<ZipArchive::Entry>();
	}

	if(!m_password.empty()) {
		if(!newFileEntry->setEncryptionMethod(m_encryptionMethod)) {
			return std::weak_ptr<ZipArchive::Entry>();
		}
	}

	if(!addEntry(std::move(newFileEntry))) {
		fmt::print("Failed add new zip file entry to archive for file: '{}'.\n", entryFilePath);
		return std::weak_ptr<ZipArchive::Entry>();
	}

	return getEntry(newZipFileIndex);
}

std::weak_ptr<ZipArchive::Entry> ZipArchive::addDirectory(const std::string & entryDirectoryPath) {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to add a directory.\n");
		return std::weak_ptr<ZipArchive::Entry>();
	}

	if(entryDirectoryPath.empty()) {
		return std::weak_ptr<ZipArchive::Entry>();
	}

	std::string formattedEntryDirectoryPath(formatDirectoryPath(entryDirectoryPath));

	if(Utilities::startsWithPathSeparator(formattedEntryDirectoryPath)) {
		fmt::print("Cannot add invalid directory path, must not start with path separator: '{}'.\n", entryDirectoryPath);
		return std::weak_ptr<ZipArchive::Entry>();
	}

	std::weak_ptr<ZipArchive::Entry> existingEntryDirectory(getEntry(formattedEntryDirectoryPath, false));

	if(!existingEntryDirectory.expired()) {
		return existingEntryDirectory;
	}

	int64_t newDirectoryIndex = -1;
	std::string curentDirectoryPath;
	std::vector<std::string_view> pathParts(Utilities::splitPath(formattedEntryDirectoryPath));

	for(std::vector<std::string_view>::const_iterator i = pathParts.begin(); i != pathParts.end(); ++i) {
		curentDirectoryPath = Utilities::addTrailingPathSeparator(Utilities::joinPaths(curentDirectoryPath, *i));

		if(hasEntry(curentDirectoryPath, false)) {
			continue;
		}

		newDirectoryIndex = zip_dir_add(m_archiveHandle.get(), curentDirectoryPath.c_str(), ZIP_FL_ENC_GUESS);

		if(newDirectoryIndex == -1) {
			fmt::print("Failed to add zip directory to archive: '{}'.\n", formattedEntryDirectoryPath);
			return std::weak_ptr<ZipArchive::Entry>();
		}

		m_modified = true;

		std::unique_ptr<Entry> newDirectoryEntry(createEntryFromIndex(newDirectoryIndex));

		if(newDirectoryEntry != nullptr) {
			if(!addEntry(std::move(newDirectoryEntry))) {
				fmt::print("Failed add new zip directory entry to archive for directory: '{}'.\n", formattedEntryDirectoryPath);
			}
		}
		else {
			fmt::print("Failed to create zip directory entry for directory: '{}'.\n", formattedEntryDirectoryPath);
		}
	}

	return getEntry(formattedEntryDirectoryPath, false);
}

std::string ZipArchive::formatDirectoryPath(const std::string & directoryPath) {
	if(directoryPath.empty()) {
		return {};
	}

	std::string formattedDirectoryPath(Utilities::addTrailingPathSeparator(Utilities::replaceAll(Utilities::replaceAll(Utilities::trimString(directoryPath), "\\", "/"), "//", "/")));

	if(!formattedDirectoryPath.empty() && formattedDirectoryPath[0] == '/') {
		return {};
	}

	return formattedDirectoryPath;
}

size_t ZipArchive::removeEntry(Entry & entry) {
	return removeEntry(entry, true);
}

size_t ZipArchive::removeEntry(Entry & entry, bool removeChildren) {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to remove an entry.\n");
		return 0;
	}

	if(!hasEntry(entry)) {
		return 0;
	}

	bool entryRemoved = false;
	size_t numberOfEntriesRemoved = 0;

	if(entry.isDirectory() && removeChildren) {
		std::vector<std::weak_ptr<Entry>> directoryEntries(entry.getChildren(false, false));

		for(std::vector<std::weak_ptr<Entry>>::iterator i = directoryEntries.begin(); i != directoryEntries.end(); ++i) {
			numberOfEntriesRemoved += removeEntry(*(*i).lock(), removeChildren);
		}
	}

	if(ZipUtilities::isSuccess(zip_delete(m_archiveHandle.get(), entry.getIndex()), fmt::format("Failed to delete zip archive entry '{}'.", entry.getPath()))) {
		if(entry.isFile()) {
			m_numberOfFiles--;
		}
		else if(entry.isDirectory()) {
			m_numberOfDirectories--;
		}

		m_modified = true;
		numberOfEntriesRemoved++;
		entry.clearParentArchive();
		m_entries[entry.getIndex()] = nullptr;
	}

	return numberOfEntriesRemoved;
}

size_t ZipArchive::removeEntry(size_t index) {
	if(index >= m_entries.size() || m_entries[index] == nullptr) {
		return 0;
	}

	return removeEntry(*m_entries[index]);
}

size_t ZipArchive::removeEntry(const std::string & entryPath, bool caseSensitive) {
	return removeEntry(indexOfEntry(entryPath, caseSensitive));
}

size_t ZipArchive::removeFirstEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) {
	return removeEntry(indexOfFirstEntryWithName(entryName, includeSubdirectories, caseSensitive));
}

size_t ZipArchive::removeAllEntries() {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to remove all entries.\n");
		return 0;
	}

	size_t numberOfEntriesRemoved = 0;

	for(std::vector<std::shared_ptr<Entry>>::iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(ZipUtilities::isSuccess(zip_delete(m_archiveHandle.get(), (*i)->getIndex()), fmt::format("Failed to delete zip archive entry '{}'.", (*i)->getPath()))) {
			numberOfEntriesRemoved++;
			(*i)->clearParentArchive();
		}
	}


	m_modified = true;
	m_entries.clear();
	m_numberOfFiles = 0;
	m_numberOfDirectories = 0;

	return numberOfEntriesRemoved;
}

bool ZipArchive::isModified() const {
	return m_modified;
}

void ZipArchive::setModified() {
	m_modified = true;
}

std::unique_ptr<ZipArchive> ZipArchive::createNew(const std::string & filePath, bool overwrite) {
	if(!overwrite && !filePath.empty() && std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		fmt::print("Zip archive '{}' already exists, please enable overwrite flag to create.\n", filePath);
		return nullptr;
	}

	uint64_t zipFlags = ZIP_CREATE | ZIP_TRUNCATE;

	if(filePath.empty()) {
		std::unique_ptr<SourceBuffer> zipSourceBuffer(createEmptyZipArchiveSourceBuffer());

		ZipUtilities::ZipErrorHandle zipError(ZipUtilities::createZipErrorHandle());

		ZipArchiveHandle zipArchiveHandle(createZipArchiveHandle(zip_open_from_source(zipSourceBuffer->getRawSourceHandle(), zipFlags, zipError.get())));

		if(zipArchiveHandle == nullptr) {
			fmt::print("Failed to create new zip archive. {}\n", zip_error_strerror(zipError.get()));
			return nullptr;
		}

		return std::unique_ptr<ZipArchive>(new ZipArchive(std::move(zipArchiveHandle), std::move(zipSourceBuffer), filePath));
	}
	else {
		int zipErrorCode = 0;
		ZipArchiveHandle zipArchiveHandle(createZipArchiveHandle(zip_open(filePath.c_str(), zipFlags, &zipErrorCode)));

		if(!ZipUtilities::isSuccess(zipErrorCode, fmt::format("Failed to create new zip archive file: '{}'.", filePath))) {
			return nullptr;
		}

		if(zipArchiveHandle == nullptr) {
			fmt::print("Failed to create new zip archive file: '{}'.\n", filePath);
			return nullptr;
		}

		return std::unique_ptr<ZipArchive>(new ZipArchive(std::move(zipArchiveHandle), filePath));
	}
}

std::unique_ptr<ZipArchive> ZipArchive::createFrom(std::unique_ptr<ByteBuffer> data, const std::string & password, bool verifyConsistency, const std::string & filePath) {
	std::unique_ptr<SourceBuffer> zipSourceBuffer(createZipArchiveSourceBuffer(std::move(data)));

	uint64_t zipFlags = ZIP_CREATE;

	if(verifyConsistency) {
		zipFlags |= ZIP_CHECKCONS;
	}

	ZipUtilities::ZipErrorHandle zipError(ZipUtilities::createZipErrorHandle());

	ZipArchiveHandle zipArchiveHandle(createZipArchiveHandle(zip_open_from_source(zipSourceBuffer->getRawSourceHandle(), zipFlags, zipError.get())));

	if(zipArchiveHandle == nullptr) {
		fmt::print("Failed to open zip archive from source buffer. {}\n", zip_error_strerror(zipError.get()));
		return nullptr;
	}

	std::unique_ptr<ZipArchive> zipArchive(new ZipArchive(std::move(zipArchiveHandle), std::move(zipSourceBuffer), filePath, password));

	if(!password.empty() && !ZipUtilities::isSuccess(zip_set_default_password(zipArchive->getRawArchiveHandle(), password.c_str()), "Failed to set zip archive password.")) {
		return nullptr;
	}

	if(!zipArchive->initialize()) {
		return nullptr;
	}

	return zipArchive;
}

std::unique_ptr<ZipArchive> ZipArchive::readFrom(const std::string & filePath, const std::string & password, bool verifyConsistency) {
	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		fmt::print("Failed to read zip archive from non-existent file: '{}'!\n", filePath);
		return nullptr;
	}

	uint64_t zipFlags = ZIP_CREATE;

	if(verifyConsistency) {
		zipFlags |= ZIP_CHECKCONS;
	}

	int zipErrorCode = 0;
	ZipArchiveHandle zipArchiveHandle(createZipArchiveHandle(zip_open(filePath.c_str(), zipFlags, &zipErrorCode)));

	if(!ZipUtilities::isSuccess(zipErrorCode, fmt::format("Failed to open zip archive file: '{}'.", filePath))) {
		return nullptr;
	}

	if(zipArchiveHandle == nullptr) {
		fmt::print("Failed to open zip archive file: '{}'.\n", filePath);
		return nullptr;
	}

	std::unique_ptr<ZipArchive> zipArchive(new ZipArchive(std::move(zipArchiveHandle), filePath, password));

	if(!password.empty() && !ZipUtilities::isSuccess(zip_set_default_password(zipArchive->getRawArchiveHandle(), password.c_str()), "Failed to set zip archive password.")) {
		return nullptr;
	}

	if(!zipArchive->initialize()) {
		return nullptr;
	}

	return zipArchive;
}

bool ZipArchive::isOpen() const {
	return m_archiveHandle != nullptr;
}

bool ZipArchive::reopen(bool verifyConsistency) {
	if(isOpen()) {
		return false;
	}

	reset();

	uint64_t zipFlags = ZIP_CREATE;

	if(verifyConsistency) {
		zipFlags |= ZIP_CHECKCONS;
	}

	if(m_sourceBuffer != nullptr) {
		if(!m_sourceBuffer->reopen()) {
			fmt::print("Failed to re-open zip archive source buffer.\n");
			return false;
		}

		ZipUtilities::ZipErrorHandle zipError(ZipUtilities::createZipErrorHandle());

		m_archiveHandle = createZipArchiveHandle(zip_open_from_source(m_sourceBuffer->getRawSourceHandle(), zipFlags, zipError.get()));

		if(m_archiveHandle == nullptr) {
			fmt::print("Failed to re-open zip archive from source buffer. {}\n", zip_error_strerror(zipError.get()));
			return false;
		}
	}
	else if(!m_filePath.empty()) {
		if(!std::filesystem::is_regular_file(std::filesystem::path(m_filePath))) {
			fmt::print("Failed to re-open zip archive from non-existent file: '{}'!\n", m_filePath);
			return nullptr;
		}

		int zipErrorCode = 0;
		m_archiveHandle = createZipArchiveHandle(zip_open(m_filePath.c_str(), zipFlags, &zipErrorCode));

		if(!ZipUtilities::isSuccess(zipErrorCode, fmt::format("Failed to re-open zip archive file: '{}'.", m_filePath))) {
			return nullptr;
		}

		if(m_archiveHandle == nullptr) {
			fmt::print("Failed to re-open zip archive from file: '{}'.\n", m_filePath);
			return nullptr;
		}
	}
	else {
		fmt::print("Failed to re-open zip archive, both file path and source buffer are empty.\n");
		return false;
	}

	if(!m_password.empty() && !ZipUtilities::isSuccess(zip_set_default_password(m_archiveHandle.get(), m_password.c_str()), "Failed to set zip archive password when re-opening zip archive.")) {
		return false;
	}

	if(!initialize()) {
		return false;
	}

	return true;
}

bool ZipArchive::close() {
	if(!isOpen()) {
		return false;
	}

	if(!ZipUtilities::isSuccess(zip_close(m_archiveHandle.get()), "Failed to close zip archive.")) {
		return false;
	}

	m_modified = false;

	// closing an archive also frees the allocated data, so it no longer needs to be discarded
	m_archiveHandle.release();

	if(m_sourceBuffer != nullptr) {
		if(!m_sourceBuffer->close()) {
			fmt::print("Failed to close zip archive source buffer.\n");
			return false;
		}

		if(!m_filePath.empty()) {
			if(!m_sourceBuffer->getData()->writeTo(m_filePath, true)) {
				fmt::print("Failed to write zip archive data buffer to file: '{}'.\n", m_filePath);
			}
			else {
#if _DEBUG
				fmt::print("Successfully wrote updated zip archive data buffer to file: '{}'.\n", m_filePath);
#endif
			}
		}
	}
	else if(!m_filePath.empty()) {
#if _DEBUG
		fmt::print("Successfully wrote updated zip archive to file: '{}'.\n", m_filePath);
#endif
	}

	for(std::vector<std::shared_ptr<Entry>>::iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		(*i)->clearUnsavedData();
	}

	return true;
}

bool ZipArchive::save() {
	return close() && reopen();
}

std::string ZipArchive::toDebugString() const {
	std::stringstream stringStream;

	stringStream << fmt::format("File Path: '{}'\n", m_filePath);
	stringStream << fmt::format("Password: '{}'\n", m_password);
	stringStream << fmt::format("Compressed Size: {} bytes\n", m_compressedSize);
	stringStream << fmt::format("Compression Method: {}\n", magic_enum::enum_name(m_compressionMethod));
	stringStream << fmt::format("Encryption Method: {}\n", magic_enum::enum_name(m_encryptionMethod));
	stringStream << fmt::format("Date Modified: {}\n", Utilities::timePointToString(m_date));
	stringStream << fmt::format("Number of Entries: {} (Files: {}, Directories: {})\n", numberOfEntries(), m_numberOfFiles, m_numberOfDirectories);

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		stringStream << fmt::format("{}. '{}' Size: {} CRC32: {} Compression: {} Encryption: {}\n", (*i)->getIndex(), (*i)->getPath(), (*i)->getInflatedSize(), (*i)->getCRC32(), magic_enum::enum_name((*i)->getCompressionMethod()), magic_enum::enum_name((*i)->getEncryptionMethod()));
	}

	return stringStream.str();
}

std::unique_ptr<ZipArchive::Entry> ZipArchive::createEntryFromIndex(size_t index, std::unique_ptr<ByteBuffer> data) {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to create an entry from an index.\n");
		return nullptr;
	}

	size_t entryCount = numberOfEntries();

	if(index >= entryCount) {
		fmt::print("Cannot create zip entry from out of bounds index value: {}!\n", index);
		return nullptr;
	}

	zip_stat_t zipEntryInfo;
	zip_stat_init(&zipEntryInfo);

	if(!ZipUtilities::isSuccess(zip_stat_index(m_archiveHandle.get(), index, ZIP_FL_ENC_GUESS, &zipEntryInfo), fmt::format("Failed to obtain zip file entry #{} statistics.", index + 1))) {
		return nullptr;
	}

	std::optional<CompressionMethod> optionalCompressionMethod(magic_enum::enum_cast<CompressionMethod>(zipEntryInfo.comp_method));

	if(!optionalCompressionMethod.has_value()) {
		fmt::print("Failed to determine zip file entry '{}' compression method.\n", zipEntryInfo.name);
		return nullptr;
	}

	std::optional<EncryptionMethod> optionalEncryptionMethod(magic_enum::enum_cast<EncryptionMethod>(zipEntryInfo.encryption_method));

	if(!optionalEncryptionMethod.has_value()) {
		fmt::print("Failed to determine zip file entry '{}' encryption method.\n", zipEntryInfo.name);
		return nullptr;
	}

	return std::unique_ptr<Entry>(new Entry(zipEntryInfo.name, zipEntryInfo.index, std::move(data), std::chrono::system_clock::from_time_t(zipEntryInfo.mtime), optionalCompressionMethod.value(), optionalEncryptionMethod.value(), zipEntryInfo.comp_size, zipEntryInfo.size, zipEntryInfo.crc, this));
}

void ZipArchive::reset() {
	m_date = {};
	m_compressionMethod = CompressionMethod::Default;
	m_encryptionMethod = EncryptionMethod::None;
	m_compressedSize = 0;

	m_entries.clear();
}

bool ZipArchive::initialize() {
	return populateInfo() && populateEntries() && populateDefaultMethods();
}

bool ZipArchive::populateInfo() {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to populate information.\n");
		return false;
	}

	zip_stat_t zipArchiveInfo;

	if(!ZipUtilities::getZipArchiveInfo(m_archiveHandle.get(), &zipArchiveInfo)) {
		return false;
	}

	m_date = std::chrono::system_clock::from_time_t(zipArchiveInfo.mtime);
	m_compressedSize = zipArchiveInfo.size; // comp_size is 0 when loaded from file

	return true;
}

bool ZipArchive::populateEntries() {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to populate entries.\n");
		return false;
	}

	size_t fileCount = 0;
	size_t directoryCount = 0;
	size_t entryCount = numberOfEntries();
	std::vector<std::shared_ptr<Entry>> entries;

	for(size_t i = 0; i < entryCount; i++) {
		entries.emplace_back(std::shared_ptr<Entry>(createEntryFromIndex(i).release()));

		if(entries[i] != nullptr) {
			if(entries[i]->isFile()) {
				fileCount++;
			}
			else if(entries[i]->isDirectory()) {
				directoryCount++;
			}
		}
	}

	m_entries = std::move(entries);
	m_numberOfFiles = fileCount;
	m_numberOfDirectories = directoryCount;

	return true;
}

bool ZipArchive::populateDefaultMethods() {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to populate default compression and encryption methods.\n");
		return false;
	}

	bool methodsSet = false;

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if((*i)->isFile()) {
			m_compressionMethod = (*i)->getCompressionMethod();
			m_encryptionMethod = (*i)->getEncryptionMethod();

			methodsSet = true;

			break;
		}
	}

	if(!methodsSet && !m_password.empty()) {
		m_encryptionMethod = DEFAULT_ENCRYPTION_METHOD;
	}

	if(!isCompressionMethodSupported(m_compressionMethod)) {
		fmt::print("Zip archive compression method '{}' is not supported.\n", magic_enum::enum_name(m_compressionMethod));
		return false;
	}

	if(!isEncryptionMethodSupported(m_encryptionMethod)) {
		fmt::print("Zip archive encryption method '{}' is not supported.\n", magic_enum::enum_name(m_encryptionMethod));
		return false;
	}

#if _DEBUG
	fmt::print("Using default compression method '{}' and default encryption method '{}'.\n", magic_enum::enum_name(m_compressionMethod), magic_enum::enum_name(m_encryptionMethod));
#endif

	return true;
}

bool ZipArchive::addEntry(std::unique_ptr<Entry> entry) {
	if(!isOpen()) {
		fmt::print("Zip archive must be open to add an entry.\n");
		return false;
	}

	if(entry == nullptr ||
	   entry->getParentArchive() != this ||
	   entry->getIndex() > m_entries.size()) {
		return false;
	}

	if(entry->isFile()) {
		m_numberOfFiles++;
	}
	else if(entry->isDirectory()) {
		m_numberOfDirectories++;
	}

	uint64_t entryIndex = entry->getIndex();

	m_entries.emplace(m_entries.begin() + entryIndex, std::shared_ptr<Entry>(entry.release()));

	if(m_entries.size() > entryIndex + 1) {
		for(std::vector<std::shared_ptr<Entry>>::iterator i = m_entries.begin() + entryIndex + 1; i != m_entries.end(); ++i) {
			if(*i == nullptr) {
				continue;
			}

			(*i)->setIndex((*i)->getIndex() + 1);
		}
	}

	return true;
}

const std::vector<std::shared_ptr<ZipArchive::Entry>> & ZipArchive::getEntries() const {
	return m_entries;
}

std::vector<std::shared_ptr<ZipArchive::Entry>> & ZipArchive::getEntries() {
	return m_entries;
}

zip * ZipArchive::getRawArchiveHandle() const {
	return m_archiveHandle.get();
}

zip_source * ZipArchive::getRawSourceHandle() const {
	if(m_sourceBuffer == nullptr) {
		return nullptr;
	}

	return m_sourceBuffer->getRawSourceHandle();
}

ZipArchive::ZipArchiveHandle ZipArchive::createZipArchiveHandle(zip * zipArchiveHandle) {
	return ZipArchiveHandle(zipArchiveHandle, [](zip * zipArchiveHandle) {
		if(zipArchiveHandle != nullptr) {
			zip_discard(zipArchiveHandle);
		}
	});
}

ZipArchive::ZipSourceHandle ZipArchive::createZipSourceHandle(zip_source * zipSourceHandle) {
	if(zipSourceHandle != nullptr) {
		zip_source_keep(zipSourceHandle);
	}

	return ZipSourceHandle(zipSourceHandle, [](zip_source * zipSourceHandle) {
		if(zipSourceHandle != nullptr) {
			zip_source_free(zipSourceHandle);
		}
	});
}

std::unique_ptr<ZipArchive::SourceBuffer> ZipArchive::createEmptyZipFileSourceBuffer() {
	return createZipFileSourceBuffer(std::make_unique<ByteBuffer>());
}

std::unique_ptr<ZipArchive::SourceBuffer> ZipArchive::createZipFileSourceBuffer(std::unique_ptr<ByteBuffer> data) {
	if(data == nullptr) {
		fmt::print("Cannot create zip file source buffer from null data!\n");
		return nullptr;
	}

	ZipUtilities::ZipErrorHandle zipError(ZipUtilities::createZipErrorHandle());

	ZipSourceHandle zipSourceHandle(createZipSourceHandle(zip_source_buffer(m_archiveHandle.get(), data->getRawData(), data->getSize(), 0)));

	if(zipSourceHandle == nullptr) {
		fmt::print("Failed to create zip file source buffer.\n");
		return nullptr;
	}

	return std::unique_ptr<SourceBuffer>(new SourceBuffer(std::move(zipSourceHandle), std::move(data)));
}

std::unique_ptr<ZipArchive::SourceBuffer> ZipArchive::createEmptyZipArchiveSourceBuffer() {
	return createZipArchiveSourceBuffer(std::make_unique<ByteBuffer>());
}

std::unique_ptr<ZipArchive::SourceBuffer> ZipArchive::createZipArchiveSourceBuffer(std::unique_ptr<ByteBuffer> data) {
	if(data == nullptr) {
		fmt::print("Cannot create zip archive source buffer from null data!\n");
		return nullptr;
	}

	ZipUtilities::ZipErrorHandle zipError(ZipUtilities::createZipErrorHandle());

	ZipSourceHandle zipSourceHandle(createZipSourceHandle(zip_source_buffer_create(data->getRawData(), data->getSize(), 0, zipError.get())));

	if(zipSourceHandle == nullptr) {
		fmt::print("Failed to create zip archive source buffer. {}\n", zip_error_strerror(zipError.get()));
		return nullptr;
	}

	return std::unique_ptr<SourceBuffer>(new SourceBuffer(std::move(zipSourceHandle), std::move(data)));
}
