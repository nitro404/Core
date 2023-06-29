#include "TarGZipArchive.h"

#include <spdlog/spdlog.h>

#include <filesystem>

const std::string TarGZipArchive::DEFAULT_FILE_EXTENSION("tar.gz");
const std::string TarGZipArchive::ALTERNATE_FILE_EXTENSION("tgz");

const std::vector<std::string> TarGZipArchive::FILE_EXTENSIONS = {
	DEFAULT_FILE_EXTENSION,
	ALTERNATE_FILE_EXTENSION
};

TarGZipArchive::TarGZipArchive(const std::string & filePath)
	: CompressedTarArchive(filePath, ByteBuffer::CompressionMethod::ZLib) { }

TarGZipArchive::TarGZipArchive(std::unique_ptr<TarArchive> tarArchive)
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

TarGZipArchive::TarGZipArchive(TarGZipArchive && t) noexcept
	: CompressedTarArchive(std::move(t)) { }

TarGZipArchive::TarGZipArchive(const TarGZipArchive & t)
	: CompressedTarArchive(t) { }

TarGZipArchive & TarGZipArchive::operator = (TarGZipArchive && t) noexcept {
	if(this != &t) {
		CompressedTarArchive::operator = (std::move(t));
	}

	return *this;
}

TarGZipArchive & TarGZipArchive::operator = (const TarGZipArchive & t) {
	CompressedTarArchive::operator = (t);

	return *this;
}

TarGZipArchive::~TarGZipArchive() { }

std::string TarGZipArchive::getDefaultFileExtension() const {
	return DEFAULT_FILE_EXTENSION;
}

std::unique_ptr<TarGZipArchive> TarGZipArchive::readFrom(const std::string & filePath) {
	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::error("Failed to read GZip tar archive from non-existent file: '{}'!", filePath);
		return nullptr;
	}

	std::unique_ptr<TarGZipArchive> tarGZipArchive(createFrom(ByteBuffer::readFrom(filePath)));

	if(tarGZipArchive == nullptr) {
		return nullptr;
	}

	tarGZipArchive->m_filePath = filePath;

	return tarGZipArchive;
}

std::unique_ptr<TarGZipArchive> TarGZipArchive::createFrom(std::unique_ptr<ByteBuffer> data) {
	if(data == nullptr) {
		return nullptr;
	}

	std::unique_ptr<TarArchive> tarArchive(TarArchive::createFrom(data->decompressed(ByteBuffer::CompressionMethod::ZLib)));

	if(tarArchive == nullptr) {
		return nullptr;
	}

	std::unique_ptr<TarGZipArchive> tarGZipArchive(std::unique_ptr<TarGZipArchive>(new TarGZipArchive(std::move(tarArchive))));
	tarGZipArchive->m_compressedSize = data->getSize();

	return tarGZipArchive;
}
