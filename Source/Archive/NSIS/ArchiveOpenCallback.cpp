#include "ArchiveOpenCallback.h"

#include <spdlog/spdlog.h>

ArchiveOpenCallback::ArchiveOpenCallback() { }

ArchiveOpenCallback::~ArchiveOpenCallback() { }

void ArchiveOpenCallback::setPassword(const std::string & password) {
	m_password = password.c_str();
}

void ArchiveOpenCallback::clearPassword() {
	m_password.reset();
}

STDMETHODIMP ArchiveOpenCallback::SetTotal(const UInt64 * files, const UInt64 * bytes) {
	return S_OK;
}

STDMETHODIMP ArchiveOpenCallback::SetCompleted(const UInt64 * files, const UInt64 * bytes) {
	return S_OK;
}

STDMETHODIMP ArchiveOpenCallback::CryptoGetTextPassword(BSTR * password) {
	if(!m_password.has_value()) {
		spdlog::error("Password is not set.");
		return E_ABORT;
	}

	return StringToBstr(m_password.value(), password);
}
