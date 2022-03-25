#include "ZipArchive.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"
#include "ZipUtilities.h"

#include <fmt/core.h>
#include <magic_enum.hpp>

ZipArchive::Entry::Entry(const std::string & path, uint64_t index, std::unique_ptr<ByteBuffer> data, std::chrono::time_point<std::chrono::system_clock> date, CompressionMethod compressionMethod, EncryptionMethod encryptionMethod, uint64_t compressedSize, uint64_t inflatedSize, uint32_t crc32, ZipArchive * parentArchive)
	: m_parentArchive(parentArchive)
	, m_path(path)
	, m_index(index)
	, m_unsavedData(std::move(data))
	, m_date(date)
	, m_compressionMethod(compressionMethod)
	, m_encryptionMethod(encryptionMethod)
	, m_compressedSize(compressedSize)
	, m_inflatedSize(inflatedSize)
	, m_crc32(crc32) { }

ZipArchive::Entry::~Entry() = default;

bool ZipArchive::Entry::isFile() const {
	return isFile(m_path);
}

bool ZipArchive::Entry::isFile(std::string_view path) {
	if(path.empty()) {
		return false;
	}

	return path[path.length() - 1] != '/';
}

bool ZipArchive::Entry::isDirectory() const {
	return isDirectory(m_path);
}

bool ZipArchive::Entry::isDirectory(std::string_view path) {
	if(path.empty()) {
		return false;
	}

	return path[path.length() - 1] == '/';
}

bool ZipArchive::Entry::isInSubdirectory() const {
	return isInSubdirectory(m_path);
}

bool ZipArchive::Entry::isInSubdirectory(std::string_view path) {
	if(path.empty()) {
		return false;
	}

	return path.find_first_of("/") < path.length() - 1;
}

std::string_view ZipArchive::Entry::getName() const {
	if(isFile(m_path)) {
		return Utilities::getFileName(m_path);
	}

	return Utilities::getFileName(Utilities::trimTrailingPathSeparator(m_path));
}

bool ZipArchive::Entry::setName(const std::string & name) {
	if(!isParentArchiveOpen()) {
		fmt::print("Zip archive must be open to set the entry name.\n");
		return false;
	}

	std::string_view formattedName(Utilities::getFileName(name));

	if(formattedName.empty()) {
		fmt::print("Failed to rename zip entry, entry name cannot be empty.\n");
		return false;
	}

	std::string currentName(getName());

	if(Utilities::areStringsEqualIgnoreCase(formattedName, currentName)) {
		return true;
	}

	std::string currentPath(m_path);
	std::string entryBasePath;
	size_t firstPathSeparatorIndex = currentPath.find_first_of("/");

	if(firstPathSeparatorIndex != std::string::npos && firstPathSeparatorIndex != currentPath.length() - 1) {
		entryBasePath = Utilities::getFilePath(Utilities::trimTrailingPathSeparator(currentPath));
	}

	std::string newEntryPath(Utilities::joinPaths(entryBasePath, formattedName));

	if(isDirectory()) {
		newEntryPath = Utilities::addTrailingPathSeparator(newEntryPath);
	}

	if(m_parentArchive->hasEntry(newEntryPath)) {
		fmt::print("Failed to rename zip entry, entry with requested name '{}' already exists!\n", formattedName);
		return false;
	}

	if(ZipUtilities::isSuccess(zip_file_rename(m_parentArchive->getRawArchiveHandle(), m_index, newEntryPath.c_str(), ZIP_FL_ENC_GUESS), fmt::format("Failed to rename zip entry from '{}' to '{}'.", currentName, formattedName))) {
#if _DEBUG
		fmt::print("Renamed zip entry from '{}' to '{}'.\n", currentName, formattedName);
#endif

		m_parentArchive->setModified();
		m_path = newEntryPath;
	}
	else {
		return false;
	}

	if(isFile()) {
		return true;
	}

	std::string_view currentEntryBasePath;
	std::vector<std::shared_ptr<Entry>> & entries = m_parentArchive->getEntries();

	for(std::vector<std::shared_ptr<Entry>>::iterator i = entries.begin(); i != entries.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		std::string curentEntryPath((*i)->getPath());
		size_t firstCurrentPathSeparatorIndex = curentEntryPath.find_first_of("/");

		if(firstCurrentPathSeparatorIndex == std::string::npos || firstCurrentPathSeparatorIndex == curentEntryPath.length() - 1) {
			continue;
		}

		currentEntryBasePath = Utilities::addTrailingPathSeparator(Utilities::getFilePath(Utilities::trimTrailingPathSeparator(curentEntryPath)));

		if(currentEntryBasePath.empty() || currentEntryBasePath.length() < currentPath.length()) {
			continue;
		}

		if(Utilities::areStringsEqualIgnoreCase(std::string_view(currentEntryBasePath.data(), currentPath.length()), currentPath)) {
			size_t lastCurrentPathSeparatorIndex = currentEntryBasePath.find_last_of("/", currentPath.length() - 2);

			std::string_view currentEntryPathLeadingData(currentEntryBasePath.data(), lastCurrentPathSeparatorIndex == std::string::npos ? 0 : lastCurrentPathSeparatorIndex);
			std::string_view currentEntryPathTrailingData(curentEntryPath.data() + currentPath.length(), curentEntryPath.length() - currentPath.length());
			std::string newCurrentEntryPath(Utilities::joinPaths(currentEntryPathLeadingData, formattedName, currentEntryPathTrailingData));

			if((*i)->isDirectory()) {
				newCurrentEntryPath = Utilities::addTrailingPathSeparator(newCurrentEntryPath);
			}

			if(ZipUtilities::isSuccess(zip_file_rename(m_parentArchive->getRawArchiveHandle(), (*i)->getIndex(), newCurrentEntryPath.c_str(), ZIP_FL_ENC_GUESS), fmt::format("Failed to update zip entry path from '{}' to '{}'.", curentEntryPath, newCurrentEntryPath))) {
				return false;
			}

#if _DEBUG
			fmt::print("Renamed zip entry directory child from '{}' to '{}'.\n", curentEntryPath, newCurrentEntryPath);
#endif

			(*i)->m_path = newCurrentEntryPath;
		}
	}

	return true;
}

const std::string & ZipArchive::Entry::getPath() const {
	return m_path;
}

bool ZipArchive::Entry::move(const std::string & newBasePath, bool overwrite) {
	if(!isParentArchiveOpen()) {
		fmt::print("Zip archive must be open to move an entry.\n");
		return false;
	}

	std::string formattedNewBasePath(ZipArchive::formatDirectoryPath(newBasePath));

	std::weak_ptr<ZipArchive::Entry> destinationDirectory;

	if(!formattedNewBasePath.empty()) {
		destinationDirectory = m_parentArchive->getEntry(formattedNewBasePath);

		if(destinationDirectory.expired()) {
			destinationDirectory = m_parentArchive->addDirectory(newBasePath);
		}

		if(destinationDirectory.expired()) {
			fmt::print("Failed to create zip entry destination directory: '{}'.\n", formattedNewBasePath);
			return false;
		}
	}

	std::string destinationEntryPath(Utilities::joinPaths(formattedNewBasePath, getName()));

	if(isDirectory()) {
		destinationEntryPath = Utilities::addTrailingPathSeparator(destinationEntryPath);
	}

	if(Utilities::areStringsEqualIgnoreCase(m_path, destinationEntryPath)) {
		return false;
	}

	std::weak_ptr<ZipArchive::Entry> existingDestinationEntry(m_parentArchive->getEntry(destinationEntryPath));

	if(!existingDestinationEntry.expired()) {
		if(isFile()) {
			if(!overwrite) {
				fmt::print("Destination zip entry already exists, must specify overwrite to replace it.\n");
				return false;
			}
		}

		if(!m_parentArchive->removeEntry(*existingDestinationEntry.lock(), false)) {
			fmt::print("Failed to remove existing destination zip entry.\n");
			return false;
		}
	}

	std::string previousPath = m_path;
	std::vector<std::weak_ptr<Entry>> children(getChildren(true, false));

	if(!ZipUtilities::isSuccess(zip_file_rename(m_parentArchive->getRawArchiveHandle(), m_index, destinationEntryPath.c_str(), ZIP_FL_ENC_GUESS), fmt::format("Failed to move zip entry from '{}' to '{}'.", m_path, destinationEntryPath))) {
		return false;
	}

#if _DEBUG
	fmt::print("Moved zip entry from '{}' to '{}'.\n", m_path, destinationEntryPath);
#endif

	m_parentArchive->setModified();
	m_path = destinationEntryPath;

	if(!children.empty()) {
		std::string newChildPath;
		std::vector<std::pair<std::shared_ptr<Entry>, std::string>> newChildPaths;

		for(std::vector<std::weak_ptr<Entry>>::const_iterator i = children.begin(); i != children.end(); ++i) {
			std::shared_ptr<Entry> child((*i).lock());

			if(child.get() == nullptr || child.get() == this) {
				continue;
			}

			const std::string & childPath = child->getPath();
			newChildPath = Utilities::joinPaths(destinationEntryPath, std::string_view(childPath.data() + previousPath.length(), childPath.length() - previousPath.length()));

			if(child->isDirectory()) {
				newChildPath = Utilities::addTrailingPathSeparator(newChildPath);
			}

			if(!overwrite && m_parentArchive->hasEntry(newChildPath)) {
				fmt::print("Destination child zip entry already exists, must specify overwrite to replace it: '{}'.\n", newChildPath);
				return false;
			}

			newChildPaths.emplace_back(child, newChildPath);
		}

		children.clear();

		for(std::vector<std::pair<std::shared_ptr<Entry>, std::string>>::iterator i = newChildPaths.begin(); i != newChildPaths.end(); ++i) {
			if(!ZipUtilities::isSuccess(zip_file_rename(m_parentArchive->getRawArchiveHandle(), i->first->getIndex(), i->second.c_str(), ZIP_FL_ENC_GUESS), fmt::format("Failed to update child zip entry path from '{}' to '{}'.", i->first->getPath(), i->second))) {
				return false;
			}

#if _DEBUG
			fmt::print("Renamed zip entry directory child from '{}' to '{}'.\n", i->first->getPath(), i->second);
#endif

			i->first->m_path = i->second;
		}
	}

	return true;
}

std::vector<std::weak_ptr<ZipArchive::Entry>> ZipArchive::Entry::getChildren(bool includeSubdirectories, bool caseSensitive) const {
	if(m_parentArchive == nullptr || !isDirectory()) {
		return {};
	}

	std::vector<std::weak_ptr<Entry>> children;
	const std::vector<std::shared_ptr<Entry>> & entries = m_parentArchive->getEntries();

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = entries.begin(); i != entries.end(); ++i) {
		if(*i == nullptr || (*i).get() == this) {
			continue;
		}

		const std::string & currentPath = (*i)->getPath();
		size_t firstPathSeparatorIndex = currentPath.find_first_of("/");

		std::string entryBasePath;

		if(firstPathSeparatorIndex != std::string::npos && firstPathSeparatorIndex != currentPath.length() - 1) {
			entryBasePath = Utilities::addTrailingPathSeparator(Utilities::getFilePath(Utilities::trimTrailingPathSeparator(currentPath)));
		}

		if(entryBasePath.empty()) {
			continue;
		}

		if(includeSubdirectories) {
			if(entryBasePath.length() < m_path.length()) {
				continue;
			}

			if(Utilities::areStringsEqual(std::string_view(entryBasePath.data(), m_path.length()), m_path, caseSensitive)) {
				children.push_back(*i);
			}
		}
		else {
			if(Utilities::areStringsEqual(entryBasePath, m_path, caseSensitive)) {
				children.push_back(*i);
			}
		}
	}

	return children;
}

uint64_t ZipArchive::Entry::getIndex() const {
	return m_index;
}

bool ZipArchive::Entry::setIndex(uint64_t index) {
	if(!isParentArchiveOpen()) {
		fmt::print("Zip archive must be open to set the entry index.\n");
		return false;
	}

	m_parentArchive->setModified();
	m_index = index;

	return true;
}

std::chrono::time_point<std::chrono::system_clock> ZipArchive::Entry::getDate() const {
	return m_date;
}

bool ZipArchive::Entry::hasComment() const {
	if(!isParentArchiveOpen()) {
		return false;
	}

	uint32_t commentLength = 0;
	return zip_file_get_comment(m_parentArchive->getRawArchiveHandle(), m_index, &commentLength, ZIP_FL_ENC_GUESS) != nullptr && commentLength != 0;
}

std::string ZipArchive::Entry::getComment() const {
	if(!isParentArchiveOpen()) {
		return Utilities::emptyString;
	}

	uint32_t commentLength = 0;
	const char * comment = zip_file_get_comment(m_parentArchive->getRawArchiveHandle(), m_index, &commentLength, ZIP_FL_ENC_GUESS);

	if(comment == nullptr || commentLength == 0) {
		return Utilities::emptyString;
	}

	return std::string(comment, commentLength);
}

bool ZipArchive::Entry::setComment(const std::string & comment) {
	if(!isParentArchiveOpen()) {
		fmt::print("Zip archive must be open to set the entry comment.\n");
		return false;
	}

	if(ZipUtilities::isSuccess(zip_file_set_comment(m_parentArchive->getRawArchiveHandle(), m_index, comment.data(), comment.length(), ZIP_FL_ENC_GUESS), "Failed to set zip entry comment.")) {
		m_parentArchive->setModified();

		return true;
	}

	return false;
}

bool ZipArchive::Entry::clearComment() {
	return setComment(Utilities::emptyString);
}

uint64_t ZipArchive::Entry::getCompressedSize() const {
	return m_compressedSize;
}

uint64_t ZipArchive::Entry::getInflatedSize() const {
	return m_inflatedSize;
}

bool ZipArchive::Entry::hasUnsavedData() const {
	return m_unsavedData != nullptr;
}

std::unique_ptr<ByteBuffer> ZipArchive::Entry::getData() const {
	if(!isParentArchiveOpen()) {
		fmt::print("Zip archive must be open to get the entry data.\n");
		return nullptr;
	}

	if(m_unsavedData != nullptr) {
		return std::make_unique<ByteBuffer>(*m_unsavedData);
	}

	ZipFileHandle zipFileHandle(ZipArchive::Entry::createZipFileHandle(zip_fopen_index(m_parentArchive->getRawArchiveHandle(), m_index, ZIP_FL_ENC_GUESS)));

	if(zipFileHandle == nullptr) {
		return nullptr;
	}

	std::unique_ptr<ByteBuffer> zipEntryData(std::make_unique<ByteBuffer>(m_inflatedSize));

	int64_t numberOfBytesRead = zip_fread(zipFileHandle.get(), zipEntryData->getRawData(), m_inflatedSize);

	if(numberOfBytesRead != m_inflatedSize) {
		fmt::print("Failed to retrieve zip entry file data, number of bytes read ({}) did not match expected inflated size ({}) for entry: '{}'.\n", numberOfBytesRead, m_inflatedSize, m_path);
		return nullptr;
	}

	return zipEntryData;
}

bool ZipArchive::Entry::isCompressed() const {
	return m_compressionMethod != CompressionMethod::Store;
}

ZipArchive::CompressionMethod ZipArchive::Entry::getCompressionMethod() const {
	return m_compressionMethod;
}

bool ZipArchive::Entry::setCompressionMethod(CompressionMethod compressionMethod) {
	if(!isParentArchiveOpen()) {
		fmt::print("Zip archive must be open to set the entry compression method.\n");
		return false;
	}

	if(isDirectory()) {
		fmt::print("Cannot set compression method on directory entries.\n");
		return false;
	}

	if(!ZipArchive::isCompressionMethodSupported(compressionMethod)) {
		fmt::print("Failed to change zip entry compression method to unsupported method: '{}'. \n", magic_enum::enum_name(compressionMethod));
		return false;
	}

	if(m_compressionMethod == compressionMethod) {
		return true;
	}

	if(!ZipUtilities::isSuccess(zip_set_file_compression(m_parentArchive->getRawArchiveHandle(), m_index, magic_enum::enum_integer(compressionMethod), 0), fmt::format("Failed to change zip entry compression method from '{}' to '{}'.", magic_enum::enum_name(m_compressionMethod), magic_enum::enum_name(compressionMethod)))) {
		return false;
	}

	m_parentArchive->setModified();
	m_compressionMethod = compressionMethod;

	return true;
}

bool ZipArchive::Entry::isEncrypted() const {
	return m_encryptionMethod != EncryptionMethod::None;
}

ZipArchive::EncryptionMethod ZipArchive::Entry::getEncryptionMethod() const {
	return m_encryptionMethod;
}

bool ZipArchive::Entry::setEncryptionMethod(EncryptionMethod encryptionMethod) {
	if(!isParentArchiveOpen()) {
		fmt::print("Zip archive must be open to set the entry encryption method.\n");
		return false;
	}

	if(isDirectory()) {
		fmt::print("Cannot set encryption method on directory entries.\n");
		return false;
	}

	if(!ZipArchive::isEncryptionMethodSupported(encryptionMethod)) {
		fmt::print("Failed to change zip entry encryption method to unsupported method: '{}'. \n", magic_enum::enum_name(encryptionMethod));
		return false;
	}

	if(m_encryptionMethod == encryptionMethod) {
		return true;
	}

	if(!ZipUtilities::isSuccess(zip_file_set_encryption(m_parentArchive->getRawArchiveHandle(), m_index, magic_enum::enum_integer(encryptionMethod), nullptr), fmt::format("Failed to change zip entry encryption method from '{}' to '{}'.", magic_enum::enum_name(m_encryptionMethod), magic_enum::enum_name(encryptionMethod)))) {
		return false;
	}

	m_parentArchive->setModified();
	m_encryptionMethod = encryptionMethod;

	return true;
}

uint32_t ZipArchive::Entry::getCRC32() const {
	return m_crc32;
}

bool ZipArchive::Entry::writeTo(const std::string & directoryPath, bool overwrite) const {
	std::unique_ptr<ByteBuffer> data(getData());

	if(data == nullptr) {
		fmt::print("Failed to obtain zip entry file data when writing entry '{}' to directory: '{}'.\n", m_path, directoryPath);
		return false;
	}

	return data->writeTo(Utilities::joinPaths(directoryPath, m_path), overwrite);
}

bool ZipArchive::Entry::isParentArchiveOpen() const {
	return m_parentArchive != nullptr && m_parentArchive->isOpen();
}

ZipArchive * ZipArchive::Entry::getParentArchive() const {
	return m_parentArchive;
}

void ZipArchive::Entry::clearParentArchive() {
	m_parentArchive = nullptr;
}

void ZipArchive::Entry::clearUnsavedData() {
	m_unsavedData.reset();
}

ZipArchive::Entry::ZipFileHandle ZipArchive::Entry::createZipFileHandle(zip_file * zipFileHandle) {
	return ZipFileHandle(zipFileHandle, [](zip_file * zipFileHandle) {
		if(zipFileHandle != nullptr) {
			ZipUtilities::isSuccess(zip_fclose(zipFileHandle), "Failed to close zip file handle.");
		}
	});
}
