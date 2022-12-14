#include "RarArchive.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/TimeUtilities.h"

#include <spdlog/spdlog.h>

#include <filesystem>

RarArchive::RarArchive(ArchiveHandle archiveHandle, const std::string & filePath, std::unique_ptr<ByteBuffer> data)
	: Archive(Type::Rar)
	, m_archiveHandle(std::move(archiveHandle))
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
	for(std::vector<std::shared_ptr<RarArchive::Entry>>::iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
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

size_t RarArchive::numberOfEntries() const {
	return m_entries.size();
}

size_t RarArchive::numberOfFiles() const {
	return m_numberOfFiles;
}

size_t RarArchive::numberOfDirectories() const {
	return m_numberOfDirectories;
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

std::vector<std::shared_ptr<ArchiveEntry>> RarArchive::getEntries() const {
	std::vector<std::shared_ptr<ArchiveEntry>> entires(m_entries.size());
	std::copy(std::begin(m_entries), std::end(m_entries), std::begin(entires));

	return entires;
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
