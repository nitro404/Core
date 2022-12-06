#include "RarArchive.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

#include <spdlog/spdlog.h>

#include <filesystem>

RarArchive::Entry::Entry(uint64_t index, RarArchive * parentArchive)
	: m_index(index)
	, m_parentArchive(parentArchive) { }

RarArchive::Entry::~Entry() = default;

bool RarArchive::Entry::isFile() const {
	if(!isParentArchiveValid()) {
		return false;
	}

	return !isDirectory();
}

bool RarArchive::Entry::isInSubdirectory() const {
	return isInSubdirectory(getPath());
}

bool RarArchive::Entry::isInSubdirectory(std::string_view path) {
	if(path.empty()) {
		return false;
	}

	return path.find_first_of("/") < path.length() - 1;
}

bool RarArchive::Entry::isDirectory() const {
	if(!isParentArchiveValid()) {
		return false;
	}

	return dmc_unrar_file_is_directory(getRawParentArchiveHandle(), m_index);
}

std::string RarArchive::Entry::getName() const {
	std::string filePath(getPath());

	if(isFile()) {
		return std::string(Utilities::getFileName(filePath));
	}

	return std::string(Utilities::getFileName(Utilities::trimTrailingPathSeparator(filePath)));
}

std::string RarArchive::Entry::getPath() const {
	if(!isParentArchiveValid()) {
		return Utilities::emptyString;
	}

	dmc_unrar_archive * parentArchiveHandle = getRawParentArchiveHandle();

	bool isDir = dmc_unrar_file_is_directory(parentArchiveHandle, m_index);

	std::string path(dmc_unrar_get_filename(parentArchiveHandle, m_index, nullptr, 0) - 1 + (isDir ? 1 : 0), '\0');
	dmc_unrar_get_filename(parentArchiveHandle, m_index, const_cast<char *>(path.c_str()), path.length() + 1);
	dmc_unrar_unicode_make_valid_utf8(const_cast<char *>(path.c_str()));

	if(isDir) {
		path[path.length() - 1] = '/';
	}

	return path;
}

std::vector<std::weak_ptr<RarArchive::Entry>> RarArchive::Entry::getChildren(bool includeSubdirectories, bool caseSensitive) const {
	if(!isParentArchiveValid() || !isDirectory()) {
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

		std::string path(getPath());

		if(includeSubdirectories) {
			if(entryBasePath.length() < path.length()) {
				continue;
			}

			if(Utilities::areStringsEqual(std::string_view(entryBasePath.data(), path.length()), path, caseSensitive)) {
				children.push_back(*i);
			}
		}
		else {
			if(Utilities::areStringsEqual(entryBasePath, path, caseSensitive)) {
				children.push_back(*i);
			}
		}
	}

	return children;
}

uint64_t RarArchive::Entry::getIndex() const {
	return m_index;
}

std::chrono::time_point<std::chrono::system_clock> RarArchive::Entry::getDate() const {
	const dmc_unrar_file * statistics = getStatistics();

	if(statistics == nullptr) {
		return {};
	}

	// Note: Dr. McCoy's Un-Rar library doesn't appear to populate the unix epoch timestamp value

	return std::chrono::system_clock::from_time_t(time_t{0}) + std::chrono::milliseconds(statistics->unix_time);
}

bool RarArchive::Entry::hasComment() const {
	if(!isParentArchiveValid()) {
		return false;
	}

	return dmc_unrar_get_file_comment(getRawParentArchiveHandle(), m_index, nullptr, 0) != 0;
}

std::string RarArchive::Entry::getComment() const {
	if(!isParentArchiveValid()) {
		return {};
	}

	dmc_unrar_archive * parentArchiveHandle = getRawParentArchiveHandle();

	size_t commentLength = dmc_unrar_get_file_comment(parentArchiveHandle, m_index, nullptr, 0);

	if(commentLength == 0) {
		return {};
	}

	std::string comment(commentLength - 1, '\0');
	dmc_unrar_get_file_comment(parentArchiveHandle, m_index, const_cast<char *>(comment.c_str()), comment.length() + 1);

	return convertComment(comment);
}

uint64_t RarArchive::Entry::getCompressedSize() const {
	const dmc_unrar_file * statistics = getStatistics();

	return statistics == nullptr ? 0 : statistics->compressed_size;
};

uint64_t RarArchive::Entry::getUncompressedSize() const {
	const dmc_unrar_file * statistics = getStatistics();

	return statistics == nullptr ? 0 : statistics->uncompressed_size;
}

std::unique_ptr<ByteBuffer> RarArchive::Entry::getData() const {
	if(isDirectory()) {
		return nullptr;
	}

	std::unique_ptr<ByteBuffer> data(std::make_unique<ByteBuffer>(getUncompressedSize()));

	if(data->isEmpty()) {
		return data;
	}

	if(!isSuccess(dmc_unrar_extract_file_to_mem(getRawParentArchiveHandle(), m_index, data->getRawData(), data->getSize(), nullptr, true), fmt::format("Failed to obtain decompressed data for file '{}'.", getPath()))) {
		return nullptr;
	}

	return data;
}

uint32_t RarArchive::Entry::getCRC32() const {
	const dmc_unrar_file * statistics = getStatistics();

	return statistics == nullptr || !statistics->has_crc ? 0 : statistics->crc;
}

bool RarArchive::Entry::writeTo(const std::string & directoryPath, bool overwrite) const {
	if(!isParentArchiveValid() || isDirectory()) {
		return false;
	}

	std::string filePath(getPath());
	std::string destinationFilePath(Utilities::joinPaths(directoryPath, filePath));
	std::string formattedDestinationFilePath(Utilities::replaceAll(Utilities::replaceAll(destinationFilePath, "\\", "/"), "//", "/"));

	if(destinationFilePath != formattedDestinationFilePath) {
		spdlog::debug("Updating Rar entry file extraction path from '{}' to '{}'.", destinationFilePath, formattedDestinationFilePath);
	}

	if(formattedDestinationFilePath.find_first_of("/") != std::string::npos) {
		std::string destinationFileBasePath(Utilities::getFilePath(formattedDestinationFilePath));

		if(!destinationFileBasePath.empty() && !std::filesystem::exists(std::filesystem::path(destinationFileBasePath))) {
			std::error_code errorCode;
			std::filesystem::create_directories(destinationFileBasePath, errorCode);

			if(errorCode) {
				spdlog::error("Failed to create Rar archive file entry extraction destination directory structure for path '{}': {}", destinationFileBasePath, errorCode.message());
				return false;
			}
		}
	}

	return isSuccess(dmc_unrar_extract_file_to_path(getRawParentArchiveHandle(), m_index, formattedDestinationFilePath.c_str(), nullptr, true), fmt::format("Failed to extract file '{}' to '{}'!", filePath, directoryPath));
}

RarArchive * RarArchive::Entry::getParentArchive() const {
	return m_parentArchive;
}

void RarArchive::Entry::clearParentArchive() {
	m_parentArchive = nullptr;
}

bool RarArchive::Entry::isParentArchiveValid() const {
	return m_parentArchive != nullptr;
}

dmc_unrar_archive * RarArchive::Entry::getRawParentArchiveHandle() const {
	return m_parentArchive == nullptr ? nullptr : m_parentArchive->getRawArchiveHandle();
}

const dmc_unrar_file * RarArchive::Entry::getStatistics() const {
	if(!isParentArchiveValid()) {
		return nullptr;
	}

	return dmc_unrar_get_file_stat(getRawParentArchiveHandle(), m_index);
}
