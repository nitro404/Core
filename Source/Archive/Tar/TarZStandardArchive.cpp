#include "TarZStandardArchive.h"

#include <spdlog/spdlog.h>

#include <filesystem>

const std::string TarZStandardArchive::DEFAULT_FILE_EXTENSION("tar.zst");
const std::string TarZStandardArchive::ALTERNATE_FILE_EXTENSION("tzst");

const std::vector<std::string> TarZStandardArchive::FILE_EXTENSIONS = {
	DEFAULT_FILE_EXTENSION,
	ALTERNATE_FILE_EXTENSION
};

TarZStandardArchive::TarZStandardArchive(const std::string & filePath)
	: CompressedTarArchive(filePath, ByteBuffer::CompressionMethod::ZStandard) { }

TarZStandardArchive::TarZStandardArchive(std::unique_ptr<TarArchive> tarArchive)
	: CompressedTarArchive(tarArchive->getFilePath(), ByteBuffer::CompressionMethod::ZStandard) {
	m_numberOfFiles = tarArchive->numberOfFiles();
	m_numberOfDirectories = tarArchive->numberOfDirectories();

	std::vector<std::shared_ptr<ArchiveEntry>> entries(tarArchive->getEntries());

	for(std::shared_ptr<ArchiveEntry> entry : entries) {
		m_entries.push_back(std::dynamic_pointer_cast<TarArchive::Entry>(entry));
	}

	tarArchive.reset();

	updateParentArchive();
}

TarZStandardArchive::TarZStandardArchive(TarZStandardArchive && t) noexcept
	: CompressedTarArchive(std::move(t)) { }

TarZStandardArchive::TarZStandardArchive(const TarZStandardArchive & t)
	: CompressedTarArchive(t) { }

TarZStandardArchive & TarZStandardArchive::operator = (TarZStandardArchive && t) noexcept {
	if(this != &t) {
		CompressedTarArchive::operator = (std::move(t));
	}

	return *this;
}

TarZStandardArchive & TarZStandardArchive::operator = (const TarZStandardArchive & t) {
	CompressedTarArchive::operator = (t);

	return *this;
}

TarZStandardArchive::~TarZStandardArchive() { }

std::string TarZStandardArchive::getDefaultFileExtension() const {
	return DEFAULT_FILE_EXTENSION;
}

std::unique_ptr<TarZStandardArchive> TarZStandardArchive::readFrom(const std::string & filePath) {
	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::error("Failed to read ZStandard tar archive from non-existent file: '{}'!", filePath);
		return nullptr;
	}

	std::unique_ptr<TarZStandardArchive> tarZStandardArchive(createFrom(ByteBuffer::readFrom(filePath)));

	if(tarZStandardArchive == nullptr) {
		return nullptr;
	}

	tarZStandardArchive->m_filePath = filePath;

	return tarZStandardArchive;
}

std::unique_ptr<TarZStandardArchive> TarZStandardArchive::createFrom(std::unique_ptr<ByteBuffer> data) {
	if(data == nullptr) {
		return nullptr;
	}

	std::unique_ptr<TarArchive> tarArchive(TarArchive::createFrom(data->decompressed(ByteBuffer::CompressionMethod::ZStandard)));

	if(tarArchive == nullptr) {
		return nullptr;
	}

	std::unique_ptr<TarZStandardArchive> tarZStandardArchive(std::unique_ptr<TarZStandardArchive>(new TarZStandardArchive(std::move(tarArchive))));
	tarZStandardArchive->m_compressedSize = data->getSize();

	return tarZStandardArchive;
}
