#include "NullsoftScriptableInstallSystemArchive.h"

#include "ArchiveExtractFileCallback.h"
#include "ArchiveFileBufferOutputCallback.h"
#include "ArchiveFileBufferOutputStream.h"
#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

#include <SevenZip/CPP/Windows/PropVariant.h>

#include <spdlog/spdlog.h>

NullsoftScriptableInstallSystemArchive::Entry::~Entry() = default;

NullsoftScriptableInstallSystemArchive::Entry::Entry(uint64_t index, NullsoftScriptableInstallSystemArchive * parentArchive)
	: m_index(index)
	, m_parentArchive(parentArchive) { }

bool NullsoftScriptableInstallSystemArchive::Entry::isFile() const {
	if(!isParentArchiveValid()) {
		return false;
	}

	return !isDirectory();
}

bool NullsoftScriptableInstallSystemArchive::Entry::isDirectory() const {
	if(!isParentArchiveValid()) {
		return false;
	}

	NWindows::NCOM::CPropVariant property;
	getParentArchiveHandle()->GetProperty(m_index, kpidIsDir, &property);

	if(property.vt != VT_BOOL) {
		return false;
	}

	return static_cast<bool>(property.boolVal);
}

std::string NullsoftScriptableInstallSystemArchive::Entry::getPath() const {
	if(!isParentArchiveValid()) {
		return {};
	}

	NWindows::NCOM::CPropVariant property;
	getParentArchiveHandle()->GetProperty(m_index, kpidPath, &property);

	if(property.vt != VT_BSTR) {
		return {};
	}

	return Utilities::wideStringToString(property.bstrVal);
}

uint64_t NullsoftScriptableInstallSystemArchive::Entry::getIndex() const {
	return m_index;
}

bool NullsoftScriptableInstallSystemArchive::Entry::hasComment() const {
	return false;
}

std::string NullsoftScriptableInstallSystemArchive::Entry::getComment() const {
	return {};
}

uint64_t NullsoftScriptableInstallSystemArchive::Entry::getCompressedSize() const {
	return 0;
}

uint64_t NullsoftScriptableInstallSystemArchive::Entry::getUncompressedSize() const {
	if(!isParentArchiveValid()) {
		return {};
	}

	NWindows::NCOM::CPropVariant property;
	getParentArchiveHandle()->GetProperty(m_index, kpidSize, &property);

	switch(property.vt) {
		case VT_I1:
			return static_cast<int8_t>(property.bVal);
		case VT_UI1:
			return static_cast<uint8_t>(property.bVal);
		case VT_I2:
			return static_cast<int16_t>(property.iVal);
		case VT_UI2:
			return static_cast<uint16_t>(property.uiVal);
		case VT_INT:
		case VT_I4:
			return static_cast<int32_t>(property.lVal);
		case VT_UINT:
		case VT_UI4:
			return static_cast<uint32_t>(property.ulVal);
		case VT_I8:
			return static_cast<int64_t>(property.hVal.QuadPart);
		case VT_UI8:
			return static_cast<uint64_t>(property.uhVal.QuadPart);
		case VT_EMPTY:
			break;
		default:
			spdlog::error("Invalid NSIS uncompressed file size property type.");
			break;
	}

	return 0;
}

std::chrono::time_point<std::chrono::system_clock> NullsoftScriptableInstallSystemArchive::Entry::getDate() const {
	if(!isParentArchiveValid()) {
		return {};
	}

	NWindows::NCOM::CPropVariant property;
	getParentArchiveHandle()->GetProperty(m_index, kpidMTime, &property);

	if(property.vt == VT_EMPTY) {
		return {};
	}

	if(property.vt != VT_FILETIME) {
		spdlog::error("Invalid NSIS file modification timestamp property type.");
		return {};
	}

	return std::chrono::system_clock::from_time_t(time_t{0}) + std::chrono::milliseconds(((property.filetime.dwLowDateTime | (static_cast<UInt64>(property.filetime.dwHighDateTime) << 32)) / 10000ULL) - 11644473600000ULL);;
}

std::unique_ptr<ByteBuffer> NullsoftScriptableInstallSystemArchive::Entry::getData() const {
	if(!isParentArchiveValid() || !isFile()) {
		return nullptr;
	}

	std::string path(getPath());

	const CMyComPtr<IInArchive> & archiveHandle = getParentArchiveHandle();

	UInt32 fileIndicies[1] = {
		static_cast<UInt32>(m_index)
	};

	ArchiveFileBufferOutputCallback * bufferOutputCallbackSpec = new ArchiveFileBufferOutputCallback(*this);
	CMyComPtr<IArchiveExtractCallback> archiveExtractCallback(bufferOutputCallbackSpec);

	HRESULT result = archiveHandle->Extract(fileIndicies, 1, false, archiveExtractCallback);

	if(result != S_OK) {
		spdlog::error("Failed to get NSIS installer file '{}' data.", Utilities::getFileName(path));
		return nullptr;
	}

	return bufferOutputCallbackSpec->transferData();
}

uint32_t NullsoftScriptableInstallSystemArchive::Entry::getCRC32() const {
	return 0;
}

bool NullsoftScriptableInstallSystemArchive::Entry::writeToFile(const std::string & filePath, bool overwrite) {
	if(!isParentArchiveValid()) {
		return false;
	}

	const CMyComPtr<IInArchive> & archiveHandle = getParentArchiveHandle();

	UInt32 fileIndicies[1] = {
		static_cast<UInt32>(m_index)
	};

	HRESULT result = archiveHandle->Extract(fileIndicies, 1, false, CMyComPtr<IArchiveExtractCallback>(new ArchiveExtractFileCallback(*this, filePath, overwrite)));

	if(result != S_OK) {
		spdlog::error("Failed to extract NSIS installer file to: '{}'.", filePath);
		return false;
	}

	return true;
}

Archive * NullsoftScriptableInstallSystemArchive::Entry::getParentArchive() const {
	return m_parentArchive;
}

bool NullsoftScriptableInstallSystemArchive::Entry::setParentArchive(Archive * archive) {
	if(archive == nullptr) {
		m_parentArchive = nullptr;
		return true;
	}

	NullsoftScriptableInstallSystemArchive * nsisArchive = dynamic_cast<NullsoftScriptableInstallSystemArchive *>(archive);

	if(nsisArchive == nullptr) {
		return false;
	}

	m_parentArchive = nsisArchive;

	return true;
}

CMyComPtr<IInArchive> & NullsoftScriptableInstallSystemArchive::Entry::getParentArchiveHandle() {
	static CMyComPtr<IInArchive> emptyArchiveHandle;

	if(!isParentArchiveValid()) {
		return emptyArchiveHandle;
	}

	return m_parentArchive->getArchiveHandle();
}

const CMyComPtr<IInArchive> & NullsoftScriptableInstallSystemArchive::Entry::getParentArchiveHandle() const {
	static const CMyComPtr<IInArchive> EMPTY_ARCHIVE_HANDLE;

	if(!isParentArchiveValid()) {
		return EMPTY_ARCHIVE_HANDLE;
	}

	return m_parentArchive->getArchiveHandle();
}
