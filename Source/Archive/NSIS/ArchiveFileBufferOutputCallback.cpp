#include "ArchiveFileBufferOutputCallback.h"

#include "ArchiveFileBufferOutputStream.h"

#include <spdlog/spdlog.h>

ArchiveFileBufferOutputCallback::ArchiveFileBufferOutputCallback(const NullsoftScriptableInstallSystemArchive::Entry & entry)
	: m_entry(entry)
	, m_bufferOutputStreamSpec(nullptr)
	, m_extractMode(true)
	, m_data(std::make_unique<ByteBuffer>()) { }

ArchiveFileBufferOutputCallback::~ArchiveFileBufferOutputCallback() { }

void ArchiveFileBufferOutputCallback::setPassword(const std::string & password) {
	m_password = password.c_str();
}

void ArchiveFileBufferOutputCallback::clearPassword() {
	m_password.reset();
}

ByteBuffer * ArchiveFileBufferOutputCallback::getData() {
	return m_data.get();
}

const ByteBuffer * ArchiveFileBufferOutputCallback::getData() const {
	return m_data.get();
}

std::unique_ptr<ByteBuffer> ArchiveFileBufferOutputCallback::transferData() {
	return std::move(m_data);
}

STDMETHODIMP ArchiveFileBufferOutputCallback::SetTotal(UInt64 size) {
	return S_OK;
}

STDMETHODIMP ArchiveFileBufferOutputCallback::SetCompleted(const UInt64 * completeValue) {
	return S_OK;
}

STDMETHODIMP ArchiveFileBufferOutputCallback::GetStream(UInt32 index, ISequentialOutStream ** outStream, Int32 askExtractMode) {
	*outStream = 0;
	m_bufferOutputStream.Release();

	if(askExtractMode != NArchive::NExtract::NAskMode::kExtract) {
		return S_OK;
	}

	m_data->resize(m_entry.getUncompressedSize());
	m_bufferOutputStreamSpec = new ArchiveFileBufferOutputStream(m_data.get());
	m_bufferOutputStream = m_bufferOutputStreamSpec;

	*outStream = m_bufferOutputStream.Detach();

	return S_OK;
}

STDMETHODIMP ArchiveFileBufferOutputCallback::PrepareOperation(Int32 askExtractMode) {
	m_extractMode = askExtractMode == NArchive::NExtract::NAskMode::kExtract;

	return S_OK;
}

STDMETHODIMP ArchiveFileBufferOutputCallback::SetOperationResult(Int32 operationResult) {
	if(operationResult != NArchive::NExtract::NOperationResult::kOK) {
		spdlog::error(NullsoftScriptableInstallSystemArchive::getExtractionOperationErrorMessage(operationResult));
	}

	m_bufferOutputStream.Release();

	return S_OK;
}

STDMETHODIMP ArchiveFileBufferOutputCallback::CryptoGetTextPassword(BSTR * password) {
	if(!m_password.has_value()) {
		spdlog::error("Password is not set.");
		return E_ABORT;
	}

	return StringToBstr(m_password.value(), password);
}
