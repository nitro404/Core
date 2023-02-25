#include "HTTPRequestSettings.h"

using namespace std::chrono_literals;

const int64_t HTTPRequestSettings::DEFAULT_MAXIMUM_REDIRECTS = 10L;

HTTPRequestSettings::HTTPRequestSettings(std::chrono::seconds connectionTimeout, std::chrono::seconds networkTimeout, std::chrono::seconds transferTimeout, int64_t maximumRedirects, bool verboseLoggingEnabled)
	: m_connectionTimeout(connectionTimeout)
	, m_networkTimeout(networkTimeout)
	, m_transferTimeout(transferTimeout)
	, m_maximumRedirects(DEFAULT_MAXIMUM_REDIRECTS)
	, m_verboseLoggingEnabled(verboseLoggingEnabled) { }

HTTPRequestSettings::HTTPRequestSettings(HTTPRequestSettings && requestSettings) noexcept
	: m_connectionTimeout(requestSettings.m_connectionTimeout)
	, m_networkTimeout(requestSettings.m_networkTimeout)
	, m_transferTimeout(requestSettings.m_transferTimeout)
	, m_maximumRedirects(requestSettings.m_maximumRedirects)
	, m_verboseLoggingEnabled(requestSettings.m_verboseLoggingEnabled) { }

HTTPRequestSettings::HTTPRequestSettings(const HTTPRequestSettings & requestSettings)
	: m_connectionTimeout(requestSettings.m_connectionTimeout)
	, m_networkTimeout(requestSettings.m_networkTimeout)
	, m_transferTimeout(requestSettings.m_transferTimeout)
	, m_maximumRedirects(requestSettings.m_maximumRedirects)
	, m_verboseLoggingEnabled(requestSettings.m_verboseLoggingEnabled) { }

HTTPRequestSettings & HTTPRequestSettings::operator = (HTTPRequestSettings && requestSettings) noexcept {
	if(this != &requestSettings) {
		std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);
		std::lock_guard<std::recursive_mutex> otherLock(requestSettings.m_requestSettingsMutex);

		m_connectionTimeout = requestSettings.m_connectionTimeout;
		m_networkTimeout = requestSettings.m_networkTimeout;
		m_transferTimeout = requestSettings.m_transferTimeout;
		m_maximumRedirects = requestSettings.m_maximumRedirects;
		m_verboseLoggingEnabled = requestSettings.m_verboseLoggingEnabled;
	}

	return *this;
}

HTTPRequestSettings & HTTPRequestSettings::operator = (const HTTPRequestSettings & requestSettings) {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);
	std::lock_guard<std::recursive_mutex> otherLock(requestSettings.m_requestSettingsMutex);

	m_connectionTimeout = requestSettings.m_connectionTimeout;
	m_networkTimeout = requestSettings.m_networkTimeout;
	m_transferTimeout = requestSettings.m_transferTimeout;
	m_maximumRedirects = requestSettings.m_maximumRedirects;
	m_verboseLoggingEnabled = requestSettings.m_verboseLoggingEnabled;

	return *this;
}

HTTPRequestSettings::~HTTPRequestSettings() { }

bool HTTPRequestSettings::hasConnectionTimeout() const {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	return m_connectionTimeout != 0ms;
}

std::chrono::seconds HTTPRequestSettings::getConnectionTimeout() const {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	return m_connectionTimeout;
}

void HTTPRequestSettings::setConnectionTimeout(std::chrono::seconds timeout) {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	m_connectionTimeout = timeout;
}

bool HTTPRequestSettings::hasNetworkTimeout() const {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	return m_networkTimeout != 0ms;
}

std::chrono::seconds HTTPRequestSettings::getNetworkTimeout() const {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	return m_networkTimeout;
}

void HTTPRequestSettings::setNetworkTimeout(std::chrono::seconds timeout) {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	m_networkTimeout = timeout;
}

bool HTTPRequestSettings::hasTransferTimeout() const {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	return m_transferTimeout != 0ms;
}

std::chrono::seconds HTTPRequestSettings::getTransferTimeout() const {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	return m_transferTimeout;
}

void HTTPRequestSettings::setTransferTimeout(std::chrono::seconds timeout) {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	m_transferTimeout = timeout;
}

int64_t HTTPRequestSettings::getMaximumRedirects() const {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	return m_maximumRedirects;
}

void HTTPRequestSettings::setMaximumRedirects(int64_t maximumRedirects) {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	m_maximumRedirects = maximumRedirects >= 0L ? maximumRedirects : -1L;
}

bool HTTPRequestSettings::isVerboseLoggingEnabled() const {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	return m_verboseLoggingEnabled;
}

void HTTPRequestSettings::setVerboseLoggingEnabled(bool enabled) {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	m_verboseLoggingEnabled = enabled;
}

bool HTTPRequestSettings::operator == (const HTTPRequestSettings & requestSettings) const {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	return m_connectionTimeout == requestSettings.m_connectionTimeout &&
		   m_networkTimeout == requestSettings.m_networkTimeout &&
		   m_transferTimeout == requestSettings.m_transferTimeout &&
		   m_verboseLoggingEnabled == requestSettings.m_verboseLoggingEnabled;
}

bool HTTPRequestSettings::operator != (const HTTPRequestSettings & requestSettings) const {
	std::lock_guard<std::recursive_mutex> lock(m_requestSettingsMutex);

	return !operator == (requestSettings);
}
