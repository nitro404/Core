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

	nsisArchive->m_filePath = filePath;

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
