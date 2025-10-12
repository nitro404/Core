#include "TarXzArchive.h"

#include <spdlog/spdlog.h>

#include <filesystem>

const std::string TarXZArchive::DEFAULT_FILE_EXTENSION("tar.xz");
const std::string TarXZArchive::ALTERNATE_FILE_EXTENSION("txz");

const std::vector<std::string> TarXZArchive::FILE_EXTENSIONS = {
	DEFAULT_FILE_EXTENSION,
	ALTERNATE_FILE_EXTENSION
};

TarXZArchive::TarXZArchive(const std::string & filePath)
	: CompressedTarArchive(filePath, ByteBuffer::CompressionMethod::XZ) { }

TarXZArchive::TarXZArchive(std::unique_ptr<TarArchive> tarArchive)
	: CompressedTarArchive(tarArchive->getFilePath(), ByteBuffer::CompressionMethod::ZLib) {
	m_numberOfFiles = tarArchive->numberOfFiles();
	m_numberOfDirectories = tarArchive->numberOfDirectories();

	std::vector<std::shared_ptr<ArchiveEntry>> entries(tarArchive->getEntries());

	for(std::shared_ptr<ArchiveEntry> entry : entries) {
		m_entries.push_back(std::dynamic_pointer_cast<TarArchive::Entry>(entry));
	}

	tarArchive.reset();

	updateParentArchive();
}

TarXZArchive::TarXZArchive(TarXZArchive && t) noexcept
	: CompressedTarArchive(std::move(t)) { }

TarXZArchive::TarXZArchive(const TarXZArchive & t)
	: CompressedTarArchive(t) { }

TarXZArchive & TarXZArchive::operator = (TarXZArchive && t) noexcept {
	if(this != &t) {
		CompressedTarArchive::operator = (std::move(t));
	}

	return *this;
}

TarXZArchive & TarXZArchive::operator = (const TarXZArchive & t) {
	CompressedTarArchive::operator = (t);

	return *this;
}

TarXZArchive::~TarXZArchive() { }

std::string TarXZArchive::getDefaultFileExtension() const {
	return DEFAULT_FILE_EXTENSION;
}

std::unique_ptr<TarXZArchive> TarXZArchive::readFrom(const std::string & filePath) {
	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::error("Failed to read XZ tar archive from non-existent file: '{}'!", filePath);
		return nullptr;
	}

	std::unique_ptr<TarXZArchive> tarXZArchive(createFrom(ByteBuffer::readFrom(filePath)));

	if(tarXZArchive == nullptr) {
		return nullptr;
	}

	tarXZArchive->m_filePath = filePath;

	return tarXZArchive;
}

std::unique_ptr<TarXZArchive> TarXZArchive::createFrom(std::unique_ptr<ByteBuffer> data) {
	if(data == nullptr) {
		return nullptr;
	}

	std::unique_ptr<TarArchive> tarArchive(TarArchive::createFrom(data->decompressed(ByteBuffer::CompressionMethod::ZLib)));

	if(tarArchive == nullptr) {
		return nullptr;
	}

	std::unique_ptr<TarXZArchive> tarXZArchive(std::unique_ptr<TarXZArchive>(new TarXZArchive(std::move(tarArchive))));
	tarXZArchive->m_compressedSize = data->getSize();

	return tarXZArchive;
}
