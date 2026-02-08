#include "NullsoftScriptableInstallSystemArchive.h"

#include "ArchiveBufferInputStream.h"
#include "ArchiveOpenCallback.h"
#include "Platform/Windows/WindowsUtilities.h"
#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/TimeUtilities.h"

#include <SevenZip/CPP/7zip/IStream.h>
#include <SevenZip/CPP/7zip/Common/FileStreams.h>
#include <SevenZip/CPP/7zip/Common/ILibrary.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

const std::string NullsoftScriptableInstallSystemArchive::DEFAULT_FILE_EXTENSION("exe");

NullsoftScriptableInstallSystemArchive::NullsoftScriptableInstallSystemArchive(CMyComPtr<IInArchive> archiveHandle)
	: Archive(Type::NSIS)
	, m_numberOfFiles(0)
	, m_numberOfDirectories(0)
	, m_archiveHandle(std::move(archiveHandle)) {
	UInt32 entryCount = 0;
	m_archiveHandle->GetNumberOfItems(&entryCount);

	for(UInt32 i = 0; i < entryCount; i++) {
		m_entries.emplace_back(new Entry(i, this));

		if(m_entries[i]->isDirectory()) {
			m_numberOfDirectories++;
		}
		else {
			m_numberOfFiles++;
		}
	}
}

NullsoftScriptableInstallSystemArchive::NullsoftScriptableInstallSystemArchive(NullsoftScriptableInstallSystemArchive && archive) noexcept
	: Archive(std::move(archive))
	, m_filePath(std::move(archive.m_filePath))
	, m_entries(std::move(archive.m_entries))
	, m_numberOfFiles(archive.m_numberOfFiles)
	, m_numberOfDirectories(archive.m_numberOfDirectories)
	, m_archiveHandle(std::move(archive.m_archiveHandle)) {
	updateParentArchive();
}

const NullsoftScriptableInstallSystemArchive & NullsoftScriptableInstallSystemArchive::operator = (NullsoftScriptableInstallSystemArchive && archive) noexcept {
	if(this != &archive) {
		Archive::operator = (std::move(archive));

		m_filePath = std::move(archive.m_filePath);
		m_entries = std::move(archive.m_entries);
		m_numberOfFiles = archive.m_numberOfFiles;
		m_numberOfDirectories = archive.m_numberOfDirectories;
		m_archiveHandle = std::move(archive.m_archiveHandle);

		updateParentArchive();
	}

	return *this;
}

NullsoftScriptableInstallSystemArchive::~NullsoftScriptableInstallSystemArchive() {
	for(std::shared_ptr<Entry> & entry : m_entries) {
		entry->clearParentArchive();
	}
}

std::string NullsoftScriptableInstallSystemArchive::getDefaultFileExtension() const {
	return DEFAULT_FILE_EXTENSION;
}

std::string NullsoftScriptableInstallSystemArchive::getFilePath() const {
	return m_filePath;
}

void NullsoftScriptableInstallSystemArchive::setFilePath(const std::string & filePath) {
	m_filePath = filePath;
}

bool NullsoftScriptableInstallSystemArchive::hasComment() const {
	return false;
}

std::string NullsoftScriptableInstallSystemArchive::getComment() const {
	return {};
}

size_t NullsoftScriptableInstallSystemArchive::numberOfEntries() const {
	return m_entries.size();
}

size_t NullsoftScriptableInstallSystemArchive::numberOfFiles() const {
	return m_numberOfFiles;
}

size_t NullsoftScriptableInstallSystemArchive::numberOfDirectories() const {
	return m_numberOfDirectories;
}

std::vector<std::shared_ptr<ArchiveEntry>> NullsoftScriptableInstallSystemArchive::getEntries() const {
	std::vector<std::shared_ptr<ArchiveEntry>> entries(m_entries.size());
	std::copy(std::begin(m_entries), std::end(m_entries), std::begin(entries));

	return entries;
}

std::string NullsoftScriptableInstallSystemArchive::toDebugString(bool includeDate) const {
	std::stringstream stringStream;

	stringStream << fmt::format("File Path: '{}'\n", m_filePath);
	stringStream << fmt::format("Number of Entries: {} (Files: {}, Directories: {})\n", numberOfEntries(), m_numberOfFiles, m_numberOfDirectories);

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		if(i != m_entries.begin()) {
			stringStream << "\n";
		}

		stringStream << fmt::format("{}. '{}' Size: {}", (*i)->getIndex(), (*i)->getPath(), (*i)->getUncompressedSize());

		if(includeDate) {
			stringStream << fmt::format(" Date: {}", Utilities::timePointToString((*i)->getDate()));
		}
	}

	return stringStream.str();
}

bool NullsoftScriptableInstallSystemArchive::isNSISArchive(const std::string & filePath) {
	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	std::ifstream fileStream(filePath, std::ios::binary | std::ios::ate);

	if(!fileStream.is_open()) {
		return false;
	}

	const size_t size = fileStream.tellg();

	ByteBuffer buffer(size, Endianness::LittleEndian);
	buffer.resize(size);

	fileStream.seekg(0, std::ios::beg);
	fileStream.read(reinterpret_cast<char *>(buffer.getData().data()), size);
	fileStream.close();

	return isNSISArchive(buffer);
}

bool NullsoftScriptableInstallSystemArchive::isNSISArchive(const ByteBuffer & data) {
	static const std::array<uint8_t, 2> PORTABLE_EXECUTABLE_MAGIC_NUMBER({ 0x4D, 0x5A }); // MZ
	static const std::array<uint8_t, 4> PORTABLE_EXECUTABLE_SIGNATURE({ 0x50, 0x45, 0x00, 0x00 }); // PE
	static const std::string NSIS_SIGNATURE("NullsoftInst");
	static constexpr size_t PORTABLE_EXECUTABLE_HEADER_SIZE = 24;
	static constexpr size_t DOS_HEADER_NEW_EXE_HEADER_OFFSET_VALUE_OFFSET = 0x3C; 
	static constexpr size_t NT_HEADER_NUMBER_OF_SECTIONS_OFFSET = 0x06;
	static constexpr size_t NT_HEADER_OPTIONAL_HEADER_SIZE_OFFSET = 0x14;
	static constexpr size_t SECTION_HEADER_SIZE = 40;
	static constexpr size_t SECTION_HEADER_RAW_DATA_SIZE_OFFSET = 0x10;
	static constexpr size_t SECTION_HEADER_RAW_DATA_POINTER_OFFSET = 0x14;

	if(data.getSize() < PORTABLE_EXECUTABLE_MAGIC_NUMBER.size()) {
		return false;
	}

	data.setEndianness(Endianness::LittleEndian);

	if(std::memcmp(data.getRawData(), PORTABLE_EXECUTABLE_MAGIC_NUMBER.data(), PORTABLE_EXECUTABLE_MAGIC_NUMBER.size()) != 0) {
		return false;
	}

	const std::optional<int32_t> optionalNewExeHeaderOffset(data.getInteger(DOS_HEADER_NEW_EXE_HEADER_OFFSET_VALUE_OFFSET));

	if(!optionalNewExeHeaderOffset.has_value()) {
		return false;
	}

	std::unique_ptr<std::array<uint8_t, 4>> optionalPortableExecutableSignature(data.getBytes<4>(optionalNewExeHeaderOffset.value()));

	if(optionalPortableExecutableSignature == nullptr || std::memcmp(optionalPortableExecutableSignature->data(), PORTABLE_EXECUTABLE_SIGNATURE.data(), PORTABLE_EXECUTABLE_SIGNATURE.size()) != 0) {
		return false;
	}

	//const std::optional<uint16_t> optionalSizeOfOptionalHeader(data.getUnsignedShort(static_cast<size_t>(optionalNewExeHeaderOffset.value() + PORTABLE_EXECUTABLE_SIGNATURE.size() + SIZE_OF_OPTIONAL_HEADER_OFFSET));
	const std::optional<uint16_t> optionalNumberOfSections(data.getUnsignedShort(static_cast<size_t>(optionalNewExeHeaderOffset.value() + NT_HEADER_NUMBER_OF_SECTIONS_OFFSET)));

	if(!optionalNumberOfSections.has_value()) {
		return false;
	}

	const std::optional<uint16_t> optionalHeaderSize(data.getUnsignedShort(static_cast<size_t>(optionalNewExeHeaderOffset.value() + NT_HEADER_OPTIONAL_HEADER_SIZE_OFFSET)));

	if(!optionalHeaderSize.has_value()) {
		return false;
	}

	const size_t sectionTableOffset = optionalNewExeHeaderOffset.value() + PORTABLE_EXECUTABLE_HEADER_SIZE + optionalHeaderSize.value();
	std::optional<size_t> portableExecutableEndOffset;

	for (uint16_t i = 0; i < optionalNumberOfSections.value(); ++i) {
		const size_t sectionHeaderOffset = sectionTableOffset + (i * SECTION_HEADER_SIZE);

		const std::optional<uint32_t> optionalRawSize(data.getInteger(sectionHeaderOffset + SECTION_HEADER_RAW_DATA_SIZE_OFFSET));

		if(!optionalRawSize.has_value()) {
			return false;
		}

		const std::optional<uint32_t> optionalPointerRaw(data.getInteger(sectionHeaderOffset + SECTION_HEADER_RAW_DATA_POINTER_OFFSET));

		if(!optionalPointerRaw.has_value()) {
			return false;
		}

		const size_t portableExecutableEndOffsetCandidate = static_cast<size_t>(optionalPointerRaw.value() + optionalRawSize.value());

		if(portableExecutableEndOffset.has_value()) {
			portableExecutableEndOffset = std::max(portableExecutableEndOffset.value(), portableExecutableEndOffsetCandidate);
		}
		else {
			portableExecutableEndOffset = portableExecutableEndOffsetCandidate;
		}
	}

	if(!portableExecutableEndOffset.has_value() || portableExecutableEndOffset.value() >= data.getSize()) {
		return false;
	}

	const size_t overlaySize = data.getSize() - portableExecutableEndOffset.value();
	const std::vector<uint8_t> & internalData = data.getData();

	return std::search(internalData.begin() + portableExecutableEndOffset.value(), internalData.end(), std::begin(NSIS_SIGNATURE), std::end(NSIS_SIGNATURE)) != internalData.end();
}

std::unique_ptr<NullsoftScriptableInstallSystemArchive> NullsoftScriptableInstallSystemArchive::readFrom(const std::string & filePath) {
	FString archiveFilePath(filePath.c_str());
	CInFileStream * archiveFileSpec = new CInFileStream();
	CMyComPtr<IInStream> archiveFile = archiveFileSpec;

	if(!archiveFileSpec->Open(archiveFilePath)) {
		spdlog::error("Failed to open NSIS executable file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<NullsoftScriptableInstallSystemArchive> nsisArchive(createFrom(archiveFile));

	if(nsisArchive == nullptr) {
		spdlog::error("Failed to read NSIS archive from '{}' executable data.", Utilities::getFileName(filePath));
		return nullptr;
	}

	nsisArchive->setFilePath(filePath);

	return nsisArchive;
}

std::unique_ptr<NullsoftScriptableInstallSystemArchive> NullsoftScriptableInstallSystemArchive::createFrom(std::unique_ptr<ByteBuffer> data) {
	if(data == nullptr) {
		return nullptr;
	}

	return createFrom(new ArchiveBufferInputStream(std::move(data)));
}

std::unique_ptr<NullsoftScriptableInstallSystemArchive> NullsoftScriptableInstallSystemArchive::createFrom(CMyComPtr<IInStream> inputStream) {
	if(inputStream == nullptr) {
		return nullptr;
	}

	CMyComPtr<IInArchive> archiveHandle;
	HRESULT result = SevenZip::CreateObject(&CLSID_CFormatNsis, &IID_IInArchive, reinterpret_cast<void **>(&archiveHandle));

	if(result != S_OK) {
		spdlog::error("Failed to create NullSoft Scriptable Install System archive: {}", WindowsUtilities::getErrorMessage(result));
		return nullptr;
	}

	ArchiveOpenCallback * openArchiveCallbackSpec = new ArchiveOpenCallback();
	CMyComPtr<IArchiveOpenCallback> openArchiveCallback(openArchiveCallbackSpec);

	const UInt64 scanSize = 1 << 23;
	result = archiveHandle->Open(inputStream, &scanSize, openArchiveCallback);
	if(result != S_OK) {
		spdlog::error("Failed to read NSIS archive from data with error: {}", WindowsUtilities::getErrorMessage(result));
		return nullptr;
	}

	return std::unique_ptr<NullsoftScriptableInstallSystemArchive>(new NullsoftScriptableInstallSystemArchive(archiveHandle));
}

CMyComPtr<IInArchive> & NullsoftScriptableInstallSystemArchive::getArchiveHandle() {
	return m_archiveHandle;
}

const CMyComPtr<IInArchive> & NullsoftScriptableInstallSystemArchive::getArchiveHandle() const {
	return m_archiveHandle;
}

std::string NullsoftScriptableInstallSystemArchive::getExtractionOperationErrorMessage(Int32 result) {
	switch(result) {
		case NArchive::NExtract::NOperationResult::kOK:
			return "Ok";
		case NArchive::NExtract::NOperationResult::kUnsupportedMethod:
			return "Unsupported Method";
		case NArchive::NExtract::NOperationResult::kCRCError:
			return "CRC Error";
		case NArchive::NExtract::NOperationResult::kDataError:
			return "Data Error";
		case NArchive::NExtract::NOperationResult::kUnavailable:
			return "Data Unavailable";
		case NArchive::NExtract::NOperationResult::kUnexpectedEnd:
			return "Unexpected End of Data";
		case NArchive::NExtract::NOperationResult::kDataAfterEnd:
			return "Unexpected Trailing Data";
		case NArchive::NExtract::NOperationResult::kIsNotArc:
			return "Invalid Archive";
		case NArchive::NExtract::NOperationResult::kHeadersError:
			return "Headers Error";
	}

	return "Unknown Error";
}

void NullsoftScriptableInstallSystemArchive::updateParentArchive() {
	for(std::shared_ptr<Entry> & entry : m_entries) {
		entry->setParentArchive(this);
	}
}
