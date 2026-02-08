#include "TarGZipArchive.h"

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

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

bool TarGZipArchive::isTarGZipArchive(const std::string & filePath) {
	static const size_t MAX_GZIP_MAGIC_NUMBER_LENGTH = 2;

	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	std::ifstream fileStream(filePath, std::ios::binary);

	if(!fileStream.is_open()) {
		return false;
	}

	ByteBuffer buffer(MAX_GZIP_MAGIC_NUMBER_LENGTH, Endianness::LittleEndian);
	buffer.resize(MAX_GZIP_MAGIC_NUMBER_LENGTH);

	fileStream.read(reinterpret_cast<char *>(buffer.getData().data()), MAX_GZIP_MAGIC_NUMBER_LENGTH);

	const bool endOfFile = fileStream.eof();

	fileStream.close();

	if(endOfFile) {
		return false;
	}

	return isTarGZipArchive(buffer);
}

bool TarGZipArchive::isTarGZipArchive(const ByteBuffer & data) {
	static const std::array<std::array<uint8_t, 2>, 2> GZIP_MAGIC_NUMBERS({
		{ 0x1F, 0x8B },
		{ 0x1F, 0x9E }
	});

	for(const std::array<uint8_t, 2> & magicNumber : GZIP_MAGIC_NUMBERS) {
		if(data.getSize() < magicNumber.size()) {
			continue;
		}

		if(std::memcmp(data.getRawData(), magicNumber.data(), magicNumber.size()) == 0) {
			return true;
		}
	}

	return false;
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
