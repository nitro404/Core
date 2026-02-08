#include "TarLZMAArchive.h"

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

const std::string TarLZMAArchive::DEFAULT_FILE_EXTENSION("tar.lzma");
const std::string TarLZMAArchive::ALTERNATE_FILE_EXTENSION("tlz");

const std::vector<std::string> TarLZMAArchive::FILE_EXTENSIONS = {
	DEFAULT_FILE_EXTENSION,
	ALTERNATE_FILE_EXTENSION
};

TarLZMAArchive::TarLZMAArchive(const std::string & filePath)
	: CompressedTarArchive(filePath, ByteBuffer::CompressionMethod::LZMA) { }

TarLZMAArchive::TarLZMAArchive(std::unique_ptr<TarArchive> tarArchive)
	: CompressedTarArchive(tarArchive->getFilePath(), ByteBuffer::CompressionMethod::LZMA) {
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
	: CompressedTarArchive(std::move(t)) { }

TarLZMAArchive::TarLZMAArchive(const TarLZMAArchive & t)
	: CompressedTarArchive(t) { }

TarLZMAArchive & TarLZMAArchive::operator = (TarLZMAArchive && t) noexcept {
	if(this != &t) {
		CompressedTarArchive::operator = (std::move(t));
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

bool TarLZMAArchive::isTarLZMAArchive(const std::string & filePath) {
	static constexpr size_t MAX_LZMA_MAGIC_NUMBER_LENGTH = 3;

	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	std::ifstream fileStream(filePath, std::ios::binary);

	if(!fileStream.is_open()) {
		return false;
	}

	ByteBuffer buffer(MAX_LZMA_MAGIC_NUMBER_LENGTH, Endianness::BigEndian);
	buffer.resize(MAX_LZMA_MAGIC_NUMBER_LENGTH);

	fileStream.read(reinterpret_cast<char *>(buffer.getData().data()), MAX_LZMA_MAGIC_NUMBER_LENGTH);

	const bool endOfFile = fileStream.eof();

	fileStream.close();

	if(endOfFile) {
		return false;
	}

	return isTarLZMAArchive(buffer);
}

bool TarLZMAArchive::isTarLZMAArchive(const ByteBuffer & data) {
	static const std::array<uint8_t, 3> LZMA_MAGIC_NUMBER({ 0x5D, 0x00, 0x00 });

	if(data.getSize() < LZMA_MAGIC_NUMBER.size()) {
		return false;
	}

	return std::memcmp(data.getRawData(), LZMA_MAGIC_NUMBER.data(), LZMA_MAGIC_NUMBER.size()) == 0;
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

	std::unique_ptr<TarArchive> tarArchive(TarArchive::createFrom(data->decompressed(ByteBuffer::CompressionMethod::LZMA)));

	if(tarArchive == nullptr) {
		return nullptr;
	}

	std::unique_ptr<TarLZMAArchive> tarLZMAArchive(std::unique_ptr<TarLZMAArchive>(new TarLZMAArchive(std::move(tarArchive))));
	tarLZMAArchive->m_compressedSize = data->getSize();

	return tarLZMAArchive;
}
