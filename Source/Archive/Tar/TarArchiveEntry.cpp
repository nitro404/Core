#include "TarArchive.h"

#include "TarUtilities.h"
#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

#include <spdlog/spdlog.h>

#include <filesystem>

static constexpr uint16_t TAR_BLOCK_SIZE = 512;

const uint16_t TarArchive::Entry::DEFAULT_USTAR_VERSION = 0;

const uint8_t TarArchive::Entry::NORMAL_FILE_FLAG = 0;
const uint8_t TarArchive::Entry::HARD_LINK_FLAG = 1;
const uint8_t TarArchive::Entry::SYMBOLIC_LINK_FLAG = 2;
const uint8_t TarArchive::Entry::CHARACTER_SPECIAL_FLAG = 3;
const uint8_t TarArchive::Entry::BLOCK_SPECIAL_FLAG = 4;
const uint8_t TarArchive::Entry::DIRECTORY_FLAG = 5;
const uint8_t TarArchive::Entry::FIFO_FLAG = 6;
const uint8_t TarArchive::Entry::CONTIGUOUS_FILE_FLAG = 7;
const uint8_t TarArchive::Entry::GLOBAL_EXTENDED_HEADER_WITH_METADATA_FLAG = 'g';
const uint8_t TarArchive::Entry::EXTENDED_HEADER_WITH_METADATA_FOR_NEXT_FILE_FLAG = 'x';

TarArchive::Entry::Entry() { }

TarArchive::Entry::Entry(TarArchive::Entry && e) noexcept
	: ArchiveEntry(e)
	, m_index(e.m_index)
	, m_entryPath(std::move(e.m_entryPath))
	, m_fileMode(e.m_fileMode)
	, m_userID(e.m_userID)
	, m_groupID(e.m_groupID)
	, m_fileSize(e.m_fileSize)
	, m_lastModifiedTimestamp(e.m_lastModifiedTimestamp)
	, m_checksum(e.m_checksum)
	, m_fileTypeFlag(e.m_fileTypeFlag)
	, m_linkedFileName(std::move(e.m_linkedFileName))
	, m_magic(std::move(e.m_magic))
	, m_version(std::move(e.m_version))
	, m_ownerUserName(std::move(e.m_ownerUserName))
	, m_ownerGroupName(std::move(e.m_ownerGroupName))
	, m_deviceMajorNumber(e.m_deviceMajorNumber)
	, m_deviceMinorNumber(e.m_deviceMinorNumber)
	, m_fileNamePrefix(std::move(e.m_fileNamePrefix))
	, m_padding(std::move(e.m_padding))
	, m_data(std::move(e.m_data))
	, m_parentArchive(nullptr) { }

TarArchive::Entry::Entry(const TarArchive::Entry & e)
	: ArchiveEntry(e)
	, m_index(e.m_index)
	, m_entryPath(e.m_entryPath)
	, m_fileMode(e.m_fileMode)
	, m_userID(e.m_userID)
	, m_groupID(e.m_groupID)
	, m_fileSize(e.m_fileSize)
	, m_lastModifiedTimestamp(e.m_lastModifiedTimestamp)
	, m_checksum(e.m_checksum)
	, m_fileTypeFlag(e.m_fileTypeFlag)
	, m_linkedFileName(e.m_linkedFileName)
	, m_magic(e.m_magic)
	, m_version(e.m_version)
	, m_ownerUserName(e.m_ownerUserName)
	, m_ownerGroupName(e.m_ownerGroupName)
	, m_deviceMajorNumber(e.m_deviceMajorNumber)
	, m_deviceMinorNumber(e.m_deviceMinorNumber)
	, m_fileNamePrefix(e.m_fileNamePrefix)
	, m_padding(e.m_padding)
	, m_data(e.m_data)
	, m_parentArchive(nullptr) { }

TarArchive::Entry & TarArchive::Entry::operator = (TarArchive::Entry && e) noexcept {
	if(this != &e) {
		ArchiveEntry::operator = (e);

		m_index = e.m_index;
		m_entryPath = std::move(e.m_entryPath);
		m_fileMode = e.m_fileMode;
		m_userID = e.m_userID;
		m_groupID = e.m_groupID;
		m_fileSize = e.m_fileSize;
		m_lastModifiedTimestamp = e.m_lastModifiedTimestamp;
		m_checksum = e.m_checksum;
		m_fileTypeFlag = e.m_fileTypeFlag;
		m_linkedFileName = std::move(e.m_linkedFileName);
		m_magic = std::move(e.m_magic);
		m_version = std::move(e.m_version);
		m_ownerUserName = std::move(e.m_ownerUserName);
		m_ownerGroupName = std::move(e.m_ownerGroupName);
		m_deviceMajorNumber = e.m_deviceMajorNumber;
		m_deviceMinorNumber = e.m_deviceMinorNumber;
		m_fileNamePrefix = std::move(e.m_fileNamePrefix);
		m_padding = std::move(e.m_padding);
		m_data = std::move(e.m_data);
	}

	return *this;
}

TarArchive::Entry & TarArchive::Entry::operator = (const TarArchive::Entry & e) {
	ArchiveEntry::operator = (e);

	m_index = e.m_index;
	m_entryPath = e.m_entryPath;
	m_fileMode = e.m_fileMode;
	m_userID = e.m_userID;
	m_groupID = e.m_groupID;
	m_fileSize = e.m_fileSize;
	m_lastModifiedTimestamp = e.m_lastModifiedTimestamp;
	m_checksum = e.m_checksum;
	m_fileTypeFlag = e.m_fileTypeFlag;
	m_linkedFileName = e.m_linkedFileName;
	m_magic = e.m_magic;
	m_version = e.m_version;
	m_ownerUserName = e.m_ownerUserName;
	m_ownerGroupName = e.m_ownerGroupName;
	m_deviceMajorNumber = e.m_deviceMajorNumber;
	m_deviceMinorNumber = e.m_deviceMinorNumber;
	m_fileNamePrefix = e.m_fileNamePrefix;
	m_padding = e.m_padding;
	m_data = e.m_data;

	return *this;
}

TarArchive::Entry::~Entry() { }

std::string TarArchive::Entry::getPath() const {
	if(!isParentArchiveValid()) {
		return Utilities::emptyString;
	}

	return m_entryPath;
}

uint64_t TarArchive::Entry::getIndex() const {
	return m_index;
}

std::chrono::time_point<std::chrono::system_clock> TarArchive::Entry::getDate() const {
	return m_lastModifiedTimestamp;
}

bool TarArchive::Entry::hasComment() const {
	return false;
}

std::string TarArchive::Entry::getComment() const {
	return {};
}

uint64_t TarArchive::Entry::getCompressedSize() const {
	return 0;
};

uint64_t TarArchive::Entry::getUncompressedSize() const {
	return m_fileSize;
}

std::unique_ptr<ByteBuffer> TarArchive::Entry::getData() const {
	if(isDirectory()) {
		return nullptr;
	}

	return std::make_unique<ByteBuffer>(m_data);
}

uint32_t TarArchive::Entry::getCRC32() const {
	return m_checksum;
}

bool TarArchive::Entry::writeTo(const std::string & directoryPath, bool overwrite) const {
	if(!isParentArchiveValid() || isDirectory()) {
		return false;
	}

	std::string filePath(getPath());
	std::string destinationFilePath(Utilities::joinPaths(directoryPath, filePath));
	std::string formattedDestinationFilePath(Utilities::replaceAll(Utilities::replaceAll(destinationFilePath, "\\", "/"), "//", "/"));

	if(destinationFilePath != formattedDestinationFilePath) {
		spdlog::debug("Updating tar archive entry file extraction path from '{}' to '{}'.", destinationFilePath, formattedDestinationFilePath);
	}

	if(formattedDestinationFilePath.find_first_of("/") != std::string::npos) {
		std::string destinationFileBasePath(Utilities::getFilePath(formattedDestinationFilePath));

		if(!destinationFileBasePath.empty() && !std::filesystem::exists(std::filesystem::path(destinationFileBasePath))) {
			std::error_code errorCode;
			std::filesystem::create_directories(destinationFileBasePath, errorCode);

			if(errorCode) {
				spdlog::error("Failed to create tar archive file entry extraction destination directory structure for path '{}': {}", destinationFileBasePath, errorCode.message());
				return false;
			}
		}
	}

	std::unique_ptr<ByteBuffer> data(getData());

	if(data == nullptr) {
		spdlog::error("Failed to obtain tar archive entry file data when writing entry '{}' to directory: '{}'.", filePath, directoryPath);
		return false;
	}

	return data->writeTo(destinationFilePath, overwrite);
}

uint32_t TarArchive::Entry::getFileMode() const {
	return m_fileMode;
}

uint32_t TarArchive::Entry::getUserID() const {
	return m_userID;
}

uint32_t TarArchive::Entry::getGroupID() const {
	return m_groupID;
}

uint8_t TarArchive::Entry::getFileTypeFlag() const {
	return m_fileTypeFlag;
}

const std::string & TarArchive::Entry::getLinkedFileName() const {
	return m_linkedFileName;
}

const std::string & TarArchive::Entry::getMagic() const {
	return m_magic;
}

const std::string & TarArchive::Entry::getVersion() const {
	return m_version;
}

const std::string & TarArchive::Entry::getOwnerUserName() const {
	return m_ownerUserName;
}

const std::string & TarArchive::Entry::getOwnerGroupName() const {
	return m_ownerGroupName;
}

uint32_t TarArchive::Entry::getDeviceMajorNumber() const {
	return m_deviceMajorNumber;
}

uint32_t TarArchive::Entry::getDeviceMinorNumber() const {
	return m_deviceMinorNumber;
}

const std::string & TarArchive::Entry::getFileNamePrefix() const {
	return m_fileNamePrefix;
}

const std::array<uint8_t, 12> TarArchive::Entry::getPadding() const {
	return m_padding;
}

bool TarArchive::Entry::isUStar() const {
	static const std::string USTAR_IDENTIFIER("ustar");

	return Utilities::areStringsEqual(m_magic, USTAR_IDENTIFIER);
}

std::string TarArchive::Entry::fileTypeToString() const {
	return fileTypeToString(m_fileTypeFlag);
}

std::string TarArchive::Entry::fileTypeToString(uint8_t fileTypeFlag) {
	switch(fileTypeFlag) {
		case NORMAL_FILE_FLAG:
			return "Normal File";

		case HARD_LINK_FLAG:
			return "Hard Link";

		case SYMBOLIC_LINK_FLAG:
			return "Symbolic Link";

		case CHARACTER_SPECIAL_FLAG:
			return "Character Special";

		case BLOCK_SPECIAL_FLAG:
			return "Block Special";

		case DIRECTORY_FLAG:
			return "Directory";

		case FIFO_FLAG:
			return "Fifo";

		case CONTIGUOUS_FILE_FLAG:
			return "Contiguous File";

		case GLOBAL_EXTENDED_HEADER_WITH_METADATA_FLAG:
			return "Global Extended Header with Metadata";

		case EXTENDED_HEADER_WITH_METADATA_FOR_NEXT_FILE_FLAG:
			return "Extended Header with Metadata for Next File";
	}

	return {};
}

std::unique_ptr<TarArchive::Entry> TarArchive::Entry::parseFrom(const ByteBuffer & data) {
	static constexpr uint16_t CHECKSUM_OFFSET = 148;
	static constexpr uint16_t CHECKSUM_SIZE = 8;
	static constexpr uint8_t EMPTY_CHECKSUM_BYTE = ' ';

	if(!data.canReadBytes(TAR_BLOCK_SIZE)) {
		spdlog::error("Tar data is truncated, missing entry header data. Expected at least {} bytes, but found only {} bytes.", TAR_BLOCK_SIZE, data.getSize() - data.getReadOffset());
		return {};
	}

	if(data.getReadOffset() % TAR_BLOCK_SIZE != 0) {
		spdlog::warn("Tar data is corrupted, data read offset of {} is not aligned to {} byte block.", data.getReadOffset(), TAR_BLOCK_SIZE);
	}

	size_t entryOffset = data.getReadOffset();

	std::unique_ptr<Entry> tarEntry(new Entry());

	bool error = false;

	tarEntry->m_entryPath = data.readString(100, &error);
	tarEntry->m_fileMode = static_cast<uint32_t>(TarUtilities::parseOctalNumber(data.readString(8, &error)));
	tarEntry->m_userID = static_cast<uint32_t>(TarUtilities::parseOctalNumber(data.readString(8, &error)));
	tarEntry->m_groupID = static_cast<uint32_t>(TarUtilities::parseOctalNumber(data.readString(8, &error)));
	tarEntry->m_fileSize = TarUtilities::parseOctalNumber(data.readString(12, &error));
	tarEntry->m_lastModifiedTimestamp = std::chrono::system_clock::from_time_t(time_t{0}) + std::chrono::seconds(TarUtilities::parseOctalNumber(data.readString(12, &error)));
	tarEntry->m_checksum = static_cast<uint32_t>(TarUtilities::parseOctalNumber(data.readString(8, &error)));
	tarEntry->m_fileTypeFlag = static_cast<bool>(data.readUnsignedByte(&error));
	tarEntry->m_linkedFileName = data.readString(100, &error);
	tarEntry->m_magic = data.readString(6, &error);
	tarEntry->m_version = data.readString(2, &error);
	tarEntry->m_ownerUserName = data.readString(32, &error);
	tarEntry->m_ownerGroupName = data.readString(32, &error);
	tarEntry->m_deviceMajorNumber = static_cast<uint32_t>(TarUtilities::parseOctalNumber(data.readString(8, &error)));
	tarEntry->m_deviceMinorNumber = static_cast<uint32_t>(TarUtilities::parseOctalNumber(data.readString(8, &error)));
	tarEntry->m_fileNamePrefix = data.readString(155, &error);
	tarEntry->m_padding = data.readBytes<12>(&error);

	if(error) {
		return nullptr;
	}

	if(tarEntry->isFile()) {
		size_t dataPadding = (TAR_BLOCK_SIZE - (tarEntry->m_fileSize % TAR_BLOCK_SIZE));

		tarEntry->m_data = data.readBytes(tarEntry->m_fileSize, &error);

		if(error) {
			spdlog::error("Failed to read '{}' bytes for tar {} entry: '{}'.", tarEntry->m_fileSize, tarEntry->isDirectory() ? "directory" : "file", tarEntry->m_entryPath);
			return nullptr;
		}

		if(dataPadding != 512) {
			if(!data.skipReadBytes(dataPadding)) {
				return nullptr;
			}
		}
	}

	if(!tarEntry->m_entryPath.empty()) {
		uint8_t currentByte = 0;
		int64_t unsignedSum = 0;
		int64_t signedSum = 0;

		for(size_t i = 0; i < TAR_BLOCK_SIZE; i++) {
			if(i >= CHECKSUM_OFFSET && i < CHECKSUM_OFFSET + CHECKSUM_SIZE) {
				currentByte = EMPTY_CHECKSUM_BYTE;
			}
			else {
				currentByte = data[entryOffset + i];
			}

			unsignedSum += currentByte;
			signedSum += static_cast<int8_t>(currentByte);
		}

		if(tarEntry->m_checksum != unsignedSum && tarEntry->m_checksum != signedSum) {
			spdlog::error("Tar entry '{}' is corrupted, checksum verification failed.", tarEntry->m_entryPath);
			return nullptr;
		}
	}

	return tarEntry;
}

Archive * TarArchive::Entry::getParentArchive() const {
	return m_parentArchive;
}

bool TarArchive::Entry::setParentArchive(Archive * archive) {
	if(archive == nullptr) {
		m_parentArchive = nullptr;
		return true;
	}

	TarArchive * tarArchive = dynamic_cast<TarArchive *>(archive);

	if(tarArchive == nullptr) {
		return false;
	}

	m_parentArchive = tarArchive;

	return true;
}
