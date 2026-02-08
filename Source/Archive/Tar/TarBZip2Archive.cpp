#include "TarBZip2Archive.h"

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

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

bool TarBZip2Archive::isTarBZip2Archive(const std::string & filePath) {
	static constexpr size_t MAX_BZIP2_MAGIC_NUMBER_LENGTH = 3;

	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	std::ifstream fileStream(filePath, std::ios::binary);

	if(!fileStream.is_open()) {
		return false;
	}

	ByteBuffer buffer(MAX_BZIP2_MAGIC_NUMBER_LENGTH);
	buffer.resize(MAX_BZIP2_MAGIC_NUMBER_LENGTH);

	fileStream.read(reinterpret_cast<char *>(buffer.getData().data()), MAX_BZIP2_MAGIC_NUMBER_LENGTH);

	const bool endOfFile = fileStream.eof();

	fileStream.close();

	if(endOfFile) {
		return false;
	}

	return isTarBZip2Archive(buffer);
}

bool TarBZip2Archive::isTarBZip2Archive(const ByteBuffer & data) {
	static const std::array<std::array<uint8_t, 3>, 2> BZIP2_MAGIC_NUMBERS({
		{ 0x42, 0x5A, 0x68 },
		{ 0x42, 0x5A, 0x30 }
	});

	for(const std::array<uint8_t, 3> & magicNumber : BZIP2_MAGIC_NUMBERS) {
		if(data.getSize() < magicNumber.size()) {
			continue;
		}

		if(std::memcmp(data.getRawData(), magicNumber.data(), magicNumber.size()) == 0) {
			return true;
		}
	}

	return false;
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

	std::unique_ptr<TarArchive> tarArchive(TarArchive::createFrom(data->decompressed(ByteBuffer::CompressionMethod::BZip2)));

	if(tarArchive == nullptr) {
		return nullptr;
	}

	std::unique_ptr<TarBZip2Archive> tarBZip2Archive(std::unique_ptr<TarBZip2Archive>(new TarBZip2Archive(std::move(tarArchive))));
	tarBZip2Archive->m_compressedSize = data->getSize();

	return tarBZip2Archive;
}
