#include "TarArchive.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/TimeUtilities.h"

#include <spdlog/spdlog.h>

#include <filesystem>

const std::string TarArchive::DEFAULT_FILE_EXTENSION("tar");

TarArchive::TarArchive(const std::string & filePath)
	: Archive(Type::Tar)
	, m_filePath(filePath)
	, m_numberOfFiles(0)
	, m_numberOfDirectories(0) { }

TarArchive::TarArchive(TarArchive && t) noexcept
	: Archive(std::move(t))
	, m_filePath(std::move(t.m_filePath))
	, m_numberOfFiles(t.m_numberOfFiles)
	, m_numberOfDirectories(t.m_numberOfDirectories)
	, m_entries(std::move(t.m_entries)) {
	updateParentArchive();
}

TarArchive::TarArchive(const TarArchive & t)
	: Archive(t)
	, m_filePath(t.m_filePath)
	, m_numberOfFiles(t.m_numberOfFiles)
	, m_numberOfDirectories(t.m_numberOfDirectories) {
	m_entries.clear();

	for(std::shared_ptr<TarArchive::Entry> entry : t.m_entries) {
		m_entries.push_back(entry);
	}

	updateParentArchive();
}

TarArchive & TarArchive::operator = (TarArchive && t) noexcept {
	if(this != &t) {
		Archive::operator = (std::move(t));

		m_filePath = std::move(t.m_filePath);
		m_numberOfFiles = t.m_numberOfFiles;
		m_numberOfDirectories = t.m_numberOfDirectories;
		m_entries = std::move(t.m_entries);

		updateParentArchive();
	}

	return *this;
}

TarArchive & TarArchive::operator = (const TarArchive & t) {
	Archive::operator = (t);

	m_entries.clear();

	m_filePath = t.m_filePath;
	m_numberOfFiles = t.m_numberOfFiles;
	m_numberOfDirectories = t.m_numberOfDirectories;

	updateParentArchive();

	return *this;
}

TarArchive::~TarArchive() {
	for(std::vector<std::shared_ptr<TarArchive::Entry>>::iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		(*i)->clearParentArchive();
	}
}

std::string TarArchive::getDefaultFileExtension() const {
	return DEFAULT_FILE_EXTENSION;
}

std::string TarArchive::getFilePath() const {
	return m_filePath;
}

bool TarArchive::hasComment() const {
	return false;
}

std::string TarArchive::getComment() const {
	return {};
}

size_t TarArchive::numberOfEntries() const {
	return m_entries.size();
}

size_t TarArchive::numberOfFiles() const {
	return m_numberOfFiles;
}

size_t TarArchive::numberOfDirectories() const {
	return m_numberOfDirectories;
}

std::string TarArchive::toDebugString(bool includeDate) const {
	std::stringstream stringStream;

	stringStream << fmt::format("File Path: '{}'\n", m_filePath);
	stringStream << fmt::format("Number of Entries: {} (Files: {}, Directories: {})\n", numberOfEntries(), m_numberOfFiles, m_numberOfDirectories);

	uint64_t compressedSize = getCompressedSize();

	if(compressedSize != 0) {
		stringStream << fmt::format("Compressed Size: {}\n", compressedSize);
	}

	stringStream << fmt::format("Uncompressed Size: {}\n", getUncompressedSize());

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		if(i != m_entries.begin()) {
			stringStream << "\n";
		}

		stringStream << fmt::format("{}. '{}' Size: {}", (*i)->getIndex(), (*i)->getPath(), (*i)->getUncompressedSize());

		if(includeDate) {
			stringStream << fmt::format(" Date: {}", Utilities::timePointToString((*i)->getDate()));
		}
	}

	return stringStream.str();
}

std::unique_ptr<TarArchive> TarArchive::readFrom(const std::string & filePath) {
	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::error("Failed to read tar archive from non-existent file: '{}'!", filePath);
		return nullptr;
	}

	std::unique_ptr<TarArchive> tarArchive(createFrom(ByteBuffer::readFrom(filePath)));

	if(tarArchive == nullptr) {
		return nullptr;
	}

	tarArchive->m_filePath = filePath;

	return tarArchive;
}

std::unique_ptr<TarArchive> TarArchive::createFrom(std::unique_ptr<ByteBuffer> data) {
	if(data == nullptr || data->isEmpty()) {
		return nullptr;
	}

	uint64_t tarEntryIndex = 0;
	std::unique_ptr<TarArchive> tarArchive(new TarArchive());

	do {
		std::unique_ptr<Entry> tarEntry(Entry::parseFrom(*data));

		if(tarEntry == nullptr) {
			return nullptr;
		}

		if(!tarEntry->m_entryPath.empty()) {
			tarEntry->m_index = tarEntryIndex++;
			tarEntry->m_parentArchive = tarArchive.get();

			if(tarEntry->isDirectory()) {
				tarArchive->m_numberOfDirectories++;
			}
			else {
				tarArchive->m_numberOfFiles++;
			}

			tarArchive->m_entries.emplace_back(std::move(tarEntry));
		}
	} while(data->canReadBytes(1));

	return tarArchive;
}

std::vector<std::shared_ptr<ArchiveEntry>> TarArchive::getEntries() const {
	std::vector<std::shared_ptr<ArchiveEntry>> entries(m_entries.size());
	std::copy(std::begin(m_entries), std::end(m_entries), std::begin(entries));

	return entries;
}
