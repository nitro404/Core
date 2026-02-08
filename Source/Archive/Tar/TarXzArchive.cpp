#include "TarXzArchive.h"

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

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

bool TarXZArchive::isTarXZArchive(const std::string & filePath) {
	static constexpr size_t MAX_XZ_MAGIC_NUMBER_LENGTH = 6;

	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	std::ifstream fileStream(filePath, std::ios::binary);

	if(!fileStream.is_open()) {
		return false;
	}

	ByteBuffer buffer(MAX_XZ_MAGIC_NUMBER_LENGTH, Endianness::BigEndian);
	buffer.resize(MAX_XZ_MAGIC_NUMBER_LENGTH);

	fileStream.read(reinterpret_cast<char *>(buffer.getData().data()), MAX_XZ_MAGIC_NUMBER_LENGTH);

	const bool endOfFile = fileStream.eof();

	fileStream.close();

	if(endOfFile) {
		return false;
	}

	return isTarXZArchive(buffer);
}

bool TarXZArchive::isTarXZArchive(const ByteBuffer & data) {
	static const std::array<uint8_t, 6> XZ_MAGIC_NUMBER({ 0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00 });

	if(data.getSize() < XZ_MAGIC_NUMBER.size()) {
		return false;
	}

	return std::memcmp(data.getRawData(), XZ_MAGIC_NUMBER.data(), XZ_MAGIC_NUMBER.size()) == 0;
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
