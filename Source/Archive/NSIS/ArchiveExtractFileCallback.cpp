#include "ArchiveExtractFileCallback.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

#include <SevenZip/CPP/Windows/FileDir.h>
#include <SevenZip/CPP/Windows/FileFind.h>
#include <SevenZip/CPP/Windows/FileName.h>
#include <SevenZip/CPP/Windows/PropVariant.h>

#include <spdlog/spdlog.h>

ArchiveExtractFileCallback::ArchiveExtractFileCallback(NullsoftScriptableInstallSystemArchive::Entry & entry, const std::string & outputFilePath, bool overwrite)
	: m_entry(entry)
	, m_outFileStreamSpec(nullptr)
	, m_outputFilePath(FString(outputFilePath.c_str()))
	, m_overwrite(overwrite)
	, m_extractMode(true) {
	FString outputFileBasePath(std::string(Utilities::getBasePath(Utilities::wideStringToString(m_outputFilePath.GetBuf()))).c_str());
	std::string outputFileName(Utilities::getFileName(Utilities::wideStringToString(m_outputFilePath.GetBuf())));
	NWindows::NFile::NName::NormalizeDirPathPrefix(outputFileBasePath);
	m_outputFilePath = Utilities::joinPaths(Utilities::wideStringToString(outputFileBasePath.GetBuf()), outputFileName).c_str();
}

ArchiveExtractFileCallback::~ArchiveExtractFileCallback() { }

void ArchiveExtractFileCallback::setPassword(const std::string & password) {
	m_password = password.c_str();
}

void ArchiveExtractFileCallback::clearPassword() {
	m_password.reset();
}

STDMETHODIMP ArchiveExtractFileCallback::SetTotal(UInt64 size) {
	return S_OK;
}

STDMETHODIMP ArchiveExtractFileCallback::SetCompleted(const UInt64 * completeValue) {
	return S_OK;
}

STDMETHODIMP ArchiveExtractFileCallback::GetStream(UInt32 index, ISequentialOutStream ** outStream, Int32 askExtractMode) {
	*outStream = 0;
	m_outFileStream.Release();

	if(askExtractMode != NArchive::NExtract::NAskMode::kExtract) {
		return S_OK;
	}

	CMyComPtr<IInArchive> & archiveHandle = m_entry.getParentArchiveHandle();
	NWindows::NCOM::CPropVariant property;

	// get file attributes
	RINOK(archiveHandle->GetProperty(index, kpidAttrib, &property));

	if(property.vt == VT_UI4) {
		m_fileAttributes = property.ulVal;
	}
	else if(property.vt != VT_EMPTY) {
		return E_FAIL;
	}

	// get file modified timestamp
	RINOK(archiveHandle->GetProperty(index, kpidMTime, &property));

	m_fileModifiedTimestamp.reset();

	switch(property.vt) {
		case VT_EMPTY: {
			break;
		}
		case VT_FILETIME: {
			m_fileModifiedTimestamp = property.filetime;
			break;
		}
		default: {
			return E_FAIL;
		}
	}

	int pathSeparatorIndex = m_outputFilePath.ReverseFind_PathSepar();

	if(pathSeparatorIndex >= 0) {
		NWindows::NFile::NDir::CreateComplexDir(FString(std::string(Utilities::getBasePath(Utilities::wideStringToString(m_outputFilePath.GetBuf()))).c_str()));
	}

	FString fullProcessedPath = m_outputFilePath;
	m_diskFilePath = fullProcessedPath;

	if(m_entry.isDirectory()) {
		NWindows::NFile::NDir::CreateComplexDir(fullProcessedPath);
	}
	else {
		NWindows::NFile::NFind::CFileInfo fileInfo;

		if(fileInfo.Find(fullProcessedPath)) {
			if(!m_overwrite) {
				spdlog::error("Output file '{}' already exists, specify overwrite to replace.", Utilities::wideStringToString(fullProcessedPath.GetBuf()));
				return E_ABORT;
			}

			if(!NWindows::NFile::NDir::DeleteFileAlways(fullProcessedPath)) {
				spdlog::error("Failed to delete output file: {}", Utilities::wideStringToString(fullProcessedPath.GetBuf()));
				return E_ABORT;
			}
		}

		m_outFileStreamSpec = new COutFileStream();
		CMyComPtr<ISequentialOutStream> outStreamLoc(m_outFileStreamSpec);

		if(!m_outFileStreamSpec->Open(fullProcessedPath, CREATE_ALWAYS)) {
			spdlog::error("Failed to open output file: {}", Utilities::wideStringToString(fullProcessedPath.GetBuf()));
			return E_ABORT;
		}

		m_outFileStream = outStreamLoc;
		*outStream = outStreamLoc.Detach();
	}

	return S_OK;
}

STDMETHODIMP ArchiveExtractFileCallback::PrepareOperation(Int32 askExtractMode) {
	m_extractMode = askExtractMode == NArchive::NExtract::NAskMode::kExtract;

	return S_OK;
}

STDMETHODIMP ArchiveExtractFileCallback::SetOperationResult(Int32 operationResult) {
	if(operationResult != NArchive::NExtract::NOperationResult::kOK) {
		spdlog::error(NullsoftScriptableInstallSystemArchive::getExtractionOperationErrorMessage(operationResult));
	}

	if(m_outFileStream) {
		if(m_fileModifiedTimestamp.has_value()) {
			m_outFileStreamSpec->SetMTime(&m_fileModifiedTimestamp.value());
		}

		RINOK(m_outFileStreamSpec->Close());
	}

	m_outFileStream.Release();

	if(m_extractMode && m_fileAttributes.has_value()) {
		NWindows::NFile::NDir::SetFileAttrib_PosixHighDetect(m_diskFilePath, m_fileAttributes.value());
	}

	return S_OK;
}

STDMETHODIMP ArchiveExtractFileCallback::CryptoGetTextPassword(BSTR * password) {
	if(!m_password.has_value()) {
		spdlog::error("Password is not set.");
		return E_ABORT;
	}

	return StringToBstr(m_password.value(), password);
}
