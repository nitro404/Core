#include "TarBZip2Archive.h"

#include <spdlog/spdlog.h>

#include <filesystem>

const std::string TarBZip2Archive::DEFAULT_FILE_EXTENSION("tar.bz2");
const std::string TarBZip2Archive::ALTERNATE_FILE_EXTENSION("tbz2");

const std::vector<std::string> TarBZip2Archive::FILE_EXTENSIONS = {
	DEFAULT_FILE_EXTENSION,
	ALTERNATE_FILE_EXTENSION
};

TarBZip2Archive::TarBZip2Archive(const std::string & filePath)
	: CompressedTarArchive(filePath, ByteBuffer::CompressionMethod::BZip2) { }

TarBZip2Archive::TarBZip2Archive(std::unique_ptr<TarArchive> tarArchive)
	: CompressedTarArchive(tarArchive->getFilePath(), ByteBuffer::CompressionMethod::BZip2) {
	m_numberOfFiles = tarArchive->numberOfFiles();
	m_numberOfDirectories = tarArchive->numberOfDirectories();

	std::vector<std::shared_ptr<ArchiveEntry>> entries(tarArchive->getEntries());

	for(std::shared_ptr<ArchiveEntry> entry : entries) {
		m_entries.push_back(std::dynamic_pointer_cast<TarArchive::Entry>(entry));
	}

	tarArchive.reset();

	updateParentArchive();
}

TarBZip2Archive::TarBZip2Archive(TarBZip2Archive && t) noexcept
	: CompressedTarArchive(std::move(t)) { }

TarBZip2Archive::TarBZip2Archive(const TarBZip2Archive & t)
	: CompressedTarArchive(t) { }

TarBZip2Archive & TarBZip2Archive::operator = (TarBZip2Archive && t) noexcept {
	if(this != &t) {
		CompressedTarArchive::operator = (std::move(t));
	}

	return *this;
}

TarBZip2Archive & TarBZip2Archive::operator = (const TarBZip2Archive & t) {
	CompressedTarArchive::operator = (t);

	return *this;
}

TarBZip2Archive::~TarBZip2Archive() { }

std::string TarBZip2Archive::getDefaultFileExtension() const {
	return DEFAULT_FILE_EXTENSION;
}

std::unique_ptr<TarBZip2Archive> TarBZip2Archive::readFrom(const std::string & filePath) {
	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::error("Failed to read BZip2 tar archive from non-existent file: '{}'!", filePath);
		return nullptr;
	}

	std::unique_ptr<TarBZip2Archive> tarBZip2Archive(createFrom(ByteBuffer::readFrom(filePath)));

	if(tarBZip2Archive == nullptr) {
		return nullptr;
	}

	tarBZip2Archive->m_filePath = filePath;

	return tarBZip2Archive;
}

std::unique_ptr<TarBZip2Archive> TarBZip2Archive::createFrom(std::unique_ptr<ByteBuffer> data) {
	if(data == nullptr) {
		return nullptr;
	}

	std::unique_ptr<TarArchive> tarArchive(TarArchive::createFrom(data->decompressed(ByteBuffer::CompressionMethod::ZLib)));

	if(tarArchive == nullptr) {
		return nullptr;
	}

	std::unique_ptr<TarBZip2Archive> tarBZip2Archive(std::unique_ptr<TarBZip2Archive>(new TarBZip2Archive(std::move(tarArchive))));
	tarBZip2Archive->m_compressedSize = data->getSize();

	return tarBZip2Archive;
}
