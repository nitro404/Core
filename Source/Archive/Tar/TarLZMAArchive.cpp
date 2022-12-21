#include "TarLZMAArchive.h"

#include <spdlog/spdlog.h>

#include <filesystem>

const std::string TarLZMAArchive::DEFAULT_FILE_EXTENSION("tar.lzma");
const std::string TarLZMAArchive::ALTERNATE_FILE_EXTENSION("tlz");

const std::vector<std::string> TarLZMAArchive::FILE_EXTENSIONS = {
	DEFAULT_FILE_EXTENSION,
	ALTERNATE_FILE_EXTENSION
};

TarLZMAArchive::TarLZMAArchive(const std::string & filePath)
	: CompressedTarArchive(filePath, ByteBuffer::CompressionMethod::LZMA) { }

TarLZMAArchive::TarLZMAArchive(std::unique_ptr<TarArchive> tarArchive)
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

TarLZMAArchive::TarLZMAArchive(TarLZMAArchive && t) noexcept
	: CompressedTarArchive(t) { }

TarLZMAArchive::TarLZMAArchive(const TarLZMAArchive & t)
	: CompressedTarArchive(t) { }

TarLZMAArchive & TarLZMAArchive::operator = (TarLZMAArchive && t) noexcept {
	if(this != &t) {
		CompressedTarArchive::operator = (t);
	}

	return *this;
}

TarLZMAArchive & TarLZMAArchive::operator = (const TarLZMAArchive & t) {
	CompressedTarArchive::operator = (t);

	return *this;
}

TarLZMAArchive::~TarLZMAArchive() { }

std::string TarLZMAArchive::getDefaultFileExtension() const {
	return DEFAULT_FILE_EXTENSION;
}

std::unique_ptr<TarLZMAArchive> TarLZMAArchive::readFrom(const std::string & filePath) {
	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::error("Failed to read LZMA tar archive from non-existent file: '{}'!", filePath);
		return nullptr;
	}

	std::unique_ptr<TarLZMAArchive> tarLZMAArchive(createFrom(ByteBuffer::readFrom(filePath)));

	if(tarLZMAArchive == nullptr) {
		return nullptr;
	}

	tarLZMAArchive->m_filePath = filePath;

	return tarLZMAArchive;
}

std::unique_ptr<TarLZMAArchive> TarLZMAArchive::createFrom(std::unique_ptr<ByteBuffer> data) {
	if(data == nullptr) {
		return nullptr;
	}

	std::unique_ptr<TarArchive> tarArchive(TarArchive::createFrom(data->decompressed(ByteBuffer::CompressionMethod::ZLib)));

	if(tarArchive == nullptr) {
		return nullptr;
	}

	std::unique_ptr<TarLZMAArchive> tarLZMAArchive(std::unique_ptr<TarLZMAArchive>(new TarLZMAArchive(std::move(tarArchive))));
	tarLZMAArchive->m_compressedSize = data->getSize();

	return tarLZMAArchive;
}
