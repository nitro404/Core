#include "ZipArchive.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"
#include "ZipUtilities.h"

#include <fmt/core.h>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>

ZipArchive::Entry::Entry(const std::string & path, uint64_t index, std::unique_ptr<ByteBuffer> data, std::chrono::time_point<std::chrono::system_clock> date, CompressionMethod compressionMethod, EncryptionMethod encryptionMethod, uint64_t compressedSize, uint64_t uncompressedSize, uint32_t crc32, ZipArchive * parentArchive)
	: m_parentArchive(parentArchive)
	, m_path(path)
	, m_index(index)
	, m_unsavedData(std::move(data))
	, m_date(date)
	, m_compressionMethod(compressionMethod)
	, m_encryptionMethod(encryptionMethod)
	, m_compressedSize(compressedSize)
	, m_uncompressedSize(uncompressedSize)
	, m_crc32(crc32) { }

ZipArchive::Entry::Entry(Entry && entry) noexcept
	: ArchiveEntry(std::move(entry))
	, m_parentArchive(entry.m_parentArchive)
	, m_path(std::move(entry.m_path))
	, m_index(entry.m_index)
	, m_unsavedData(std::move(entry.m_unsavedData))
	, m_date(entry.m_date)
	, m_compressionMethod(entry.m_compressionMethod)
	, m_encryptionMethod(entry.m_encryptionMethod)
	, m_compressedSize(entry.m_compressedSize)
	, m_uncompressedSize(entry.m_uncompressedSize)
	, m_crc32(entry.m_crc32) { }

const ZipArchive::Entry & ZipArchive::Entry::operator = (Entry && entry) noexcept {
	if(this != &entry) {
		ArchiveEntry::operator = (std::move(entry));

		m_parentArchive = entry.m_parentArchive;
		m_path = std::move(entry.m_path);
		m_index = entry.m_index;
		m_unsavedData = std::move(entry.m_unsavedData);
		m_date = entry.m_date;
		m_compressionMethod = entry.m_compressionMethod;
		m_encryptionMethod = entry.m_encryptionMethod;
		m_compressedSize = entry.m_compressedSize;
		m_uncompressedSize = entry.m_uncompressedSize;
		m_crc32 = entry.m_crc32;
	}

	return *this;
}

ZipArchive::Entry::~Entry() { }

bool ZipArchive::Entry::setName(const std::string & name) {
	if(!isParentArchiveValid()) {
		spdlog::error("Zip archive must be open to set the entry name.");
		return false;
	}

	std::string_view formattedName(Utilities::getFileName(name));

	if(formattedName.empty()) {
		spdlog::error("Failed to rename zip entry, entry name cannot be empty.");
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
		spdlog::error("Failed to rename zip entry, entry with requested name '{}' already exists!", formattedName);
		return false;
	}

	if(ZipUtilities::isSuccess(zip_file_rename(m_parentArchive->getRawArchiveHandle(), m_index, newEntryPath.c_str(), ZIP_FL_ENC_GUESS), fmt::format("Failed to rename zip entry from '{}' to '{}'.", currentName, formattedName))) {
		spdlog::debug("Renamed zip entry from '{}' to '{}'.", currentName, formattedName);

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
	std::vector<std::shared_ptr<ArchiveEntry>> entries = m_parentArchive->getEntries();

	for(std::vector<std::shared_ptr<ArchiveEntry>>::iterator i = entries.begin(); i != entries.end(); ++i) {
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

			spdlog::debug("Renamed zip entry directory child from '{}' to '{}'.", curentEntryPath, newCurrentEntryPath);

			std::dynamic_pointer_cast<ZipArchive::Entry>(*i)->m_path = newCurrentEntryPath;
		}
	}

	return true;
}

std::string ZipArchive::Entry::getPath() const {
	return m_path;
}

bool ZipArchive::Entry::move(const std::string & newBasePath, bool overwrite) {
	if(!isParentArchiveValid()) {
		spdlog::error("Zip archive must be open to move an entry.");
		return false;
	}

	std::string formattedNewBasePath(ZipArchive::formatDirectoryPath(newBasePath));

	std::shared_ptr<ArchiveEntry> destinationDirectory;

	if(!formattedNewBasePath.empty()) {
		destinationDirectory = m_parentArchive->getEntry(formattedNewBasePath);

		if(destinationDirectory == nullptr) {
			destinationDirectory = m_parentArchive->addDirectory(newBasePath);
		}

		if(destinationDirectory == nullptr) {
			spdlog::error("Failed to create zip entry destination directory: '{}'.", formattedNewBasePath);
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

	std::shared_ptr<ArchiveEntry> existingDestinationEntry(m_parentArchive->getEntry(destinationEntryPath));

	if(existingDestinationEntry != nullptr) {
		if(isFile()) {
			if(!overwrite) {
				spdlog::error("Destination zip entry already exists, must specify overwrite to replace it.");
				return false;
			}
		}

		if(!m_parentArchive->removeEntry(*std::dynamic_pointer_cast<ZipArchive::Entry>(existingDestinationEntry), false)) {
			spdlog::error("Failed to remove existing destination zip entry.");
			return false;
		}
	}

	std::string previousPath = m_path;
	std::vector<std::shared_ptr<ArchiveEntry>> children(getChildren(true, false));

	if(!ZipUtilities::isSuccess(zip_file_rename(m_parentArchive->getRawArchiveHandle(), m_index, destinationEntryPath.c_str(), ZIP_FL_ENC_GUESS), fmt::format("Failed to move zip entry from '{}' to '{}'.", m_path, destinationEntryPath))) {
		return false;
	}

	spdlog::debug("Moved zip entry from '{}' to '{}'.", m_path, destinationEntryPath);

	m_parentArchive->setModified();
	m_path = destinationEntryPath;

	if(!children.empty()) {
		std::string newChildPath;
		std::shared_ptr<ArchiveEntry> child;
		std::vector<std::pair<std::shared_ptr<ArchiveEntry>, std::string>> newChildPaths;

		for(std::vector<std::shared_ptr<ArchiveEntry>>::const_iterator i = children.begin(); i != children.end(); ++i) {
			child = *i;

			if(child.get() == nullptr || child.get() == this) {
				continue;
			}

			const std::string & childPath = child->getPath();
			newChildPath = Utilities::joinPaths(destinationEntryPath, std::string_view(childPath.data() + previousPath.length(), childPath.length() - previousPath.length()));

			if(child->isDirectory()) {
				newChildPath = Utilities::addTrailingPathSeparator(newChildPath);
			}

			if(!overwrite && m_parentArchive->hasEntry(newChildPath)) {
				spdlog::warn("Destination child zip entry already exists, must specify overwrite to replace it: '{}'.", newChildPath);
				return false;
			}

			newChildPaths.emplace_back(child, newChildPath);
		}

		children.clear();

		for(std::vector<std::pair<std::shared_ptr<ArchiveEntry>, std::string>>::iterator i = newChildPaths.begin(); i != newChildPaths.end(); ++i) {
			if(!ZipUtilities::isSuccess(zip_file_rename(m_parentArchive->getRawArchiveHandle(), i->first->getIndex(), i->second.c_str(), ZIP_FL_ENC_GUESS), fmt::format("Failed to update child zip entry path from '{}' to '{}'.", i->first->getPath(), i->second))) {
				return false;
			}

			spdlog::debug("Renamed zip entry directory child from '{}' to '{}'.", i->first->getPath(), i->second);

			std::dynamic_pointer_cast<ZipArchive::Entry>(i->first)->m_path = i->second;
		}
	}

	return true;
}

uint64_t ZipArchive::Entry::getIndex() const {
	return m_index;
}

bool ZipArchive::Entry::setIndex(uint64_t index) {
	if(!isParentArchiveValid()) {
		spdlog::error("Zip archive must be open to set the entry index.");
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
	if(!isParentArchiveValid()) {
		return false;
	}

	uint32_t commentLength = 0;
	return zip_file_get_comment(m_parentArchive->getRawArchiveHandle(), m_index, &commentLength, ZIP_FL_ENC_GUESS) != nullptr && commentLength != 0;
}

std::string ZipArchive::Entry::getComment() const {
	if(!isParentArchiveValid()) {
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
	if(!isParentArchiveValid()) {
		spdlog::error("Zip archive must be open to set the entry comment.");
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

uint64_t ZipArchive::Entry::getUncompressedSize() const {
	return m_uncompressedSize;
}

bool ZipArchive::Entry::hasUnsavedData() const {
	return m_unsavedData != nullptr;
}

std::unique_ptr<ByteBuffer> ZipArchive::Entry::getData() const {
	if(!isParentArchiveValid()) {
		spdlog::error("Zip archive must be open to get the entry data.");
		return nullptr;
	}

	if(m_unsavedData != nullptr) {
		return std::make_unique<ByteBuffer>(*m_unsavedData);
	}

	ZipFileHandle zipFileHandle(ZipArchive::Entry::createZipFileHandle(zip_fopen_index(m_parentArchive->getRawArchiveHandle(), m_index, ZIP_FL_ENC_GUESS)));

	if(zipFileHandle == nullptr) {
		return nullptr;
	}

	std::unique_ptr<ByteBuffer> zipEntryData(std::make_unique<ByteBuffer>(m_uncompressedSize));

	int64_t numberOfBytesRead = zip_fread(zipFileHandle.get(), zipEntryData->getRawData(), m_uncompressedSize);

	if(numberOfBytesRead != m_uncompressedSize) {
		spdlog::error("Failed to retrieve zip entry file data, number of bytes read ({}) did not match expected uncompressed size ({}) for entry: '{}'.", numberOfBytesRead, m_uncompressedSize, m_path);
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
	if(!isParentArchiveValid()) {
		spdlog::error("Zip archive must be open to set the entry compression method.");
		return false;
	}

	if(isDirectory()) {
		spdlog::error("Cannot set compression method on directory entries.");
		return false;
	}

	if(!ZipArchive::isCompressionMethodSupported(compressionMethod)) {
		spdlog::error("Failed to change zip entry compression method to unsupported method: '{}'. ", magic_enum::enum_name(compressionMethod));
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
	if(!isParentArchiveValid()) {
		spdlog::error("Zip archive must be open to set the entry encryption method.");
		return false;
	}

	if(isDirectory()) {
		spdlog::error("Cannot set encryption method on directory entries.");
		return false;
	}

	if(!ZipArchive::isEncryptionMethodSupported(encryptionMethod)) {
		spdlog::error("Failed to change zip entry encryption method to unsupported method: '{}'. ", magic_enum::enum_name(encryptionMethod));
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

bool ZipArchive::Entry::writeToFile(const std::string & filePath, bool overwrite) {
	std::unique_ptr<ByteBuffer> data(getData());

	if(data == nullptr) {
		spdlog::error("Failed to obtain zip entry file data when writing entry to file: '{}'.", filePath);
		return false;
	}

	std::string formattedDestinationFilePath(Utilities::replaceAll(Utilities::replaceAll(filePath, "\\", "/"), "//", "/"));

	if(filePath != formattedDestinationFilePath) {
		spdlog::debug("Updating zip entry file extraction path from '{}' to '{}'.", filePath, formattedDestinationFilePath);
	}

	return data->writeTo(formattedDestinationFilePath, overwrite);
}

bool ZipArchive::Entry::isParentArchiveValid() const {
	return m_parentArchive != nullptr && m_parentArchive->isOpen();
}

Archive * ZipArchive::Entry::getParentArchive() const {
	return m_parentArchive;
}

bool ZipArchive::Entry::setParentArchive(Archive * archive) {
	if(archive == nullptr) {
		m_parentArchive = nullptr;
		return true;
	}

	ZipArchive * zipArchive = dynamic_cast<ZipArchive *>(archive);

	if(zipArchive == nullptr) {
		return false;
	}

	m_parentArchive = zipArchive;

	return true;
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
