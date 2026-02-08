#include "TarZStandardArchive.h"

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

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

bool TarZStandardArchive::isTarZStandardArchive(const std::string & filePath) {
	static constexpr size_t MAX_ZSTANDARD_MAGIC_NUMBER_LENGTH = 4;

	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	std::ifstream fileStream(filePath, std::ios::binary);

	if(!fileStream.is_open()) {
		return false;
	}

	ByteBuffer buffer(MAX_ZSTANDARD_MAGIC_NUMBER_LENGTH, Endianness::BigEndian);
	buffer.resize(MAX_ZSTANDARD_MAGIC_NUMBER_LENGTH);

	fileStream.read(reinterpret_cast<char *>(buffer.getData().data()), MAX_ZSTANDARD_MAGIC_NUMBER_LENGTH);

	const bool endOfFile = fileStream.eof();

	fileStream.close();

	if(endOfFile) {
		return false;
	}

	return isTarZStandardArchive(buffer);
}

bool TarZStandardArchive::isTarZStandardArchive(const ByteBuffer & data) {
	static const std::array<std::array<uint8_t, 4>, 8> ZSTANDARD_MAGIC_NUMBERS({
		{ 0x28, 0xB5, 0x2F, 0xFD }, // v0.8+
		{ 0x27, 0xB5, 0x2F, 0xFD }, // v0.7+
		{ 0x26, 0xB5, 0x2F, 0xFD }, // v0.6+
		{ 0x25, 0xB5, 0x2F, 0xFD }, // v0.5+
		{ 0x24, 0xB5, 0x2F, 0xFD }, // v0.4+
		{ 0x23, 0xB5, 0x2F, 0xFD }, // v0.3+
		{ 0x22, 0xB5, 0x2F, 0xFD }, // v0.2+
		{ 0x1E, 0xB5, 0x2F, 0xFD }  // v0.1
	});

	for(const std::array<uint8_t, 4> & magicNumber : ZSTANDARD_MAGIC_NUMBERS) {
		if(data.getSize() < magicNumber.size()) {
			continue;
		}

		if(std::memcmp(data.getRawData(), magicNumber.data(), magicNumber.size()) == 0) {
			return true;
		}
	}

	return false;
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
