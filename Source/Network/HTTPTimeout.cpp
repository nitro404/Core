#include "HTTPTimeout.h"

using namespace std::chrono_literals;

HTTPTimeout::HTTPTimeout(std::chrono::seconds connectionTimeout, std::chrono::seconds networkTimeout)
	: m_connectionTimeout(connectionTimeout)
	, m_networkTimeout(networkTimeout) { }

HTTPTimeout::HTTPTimeout(HTTPTimeout && timeout) noexcept
	: m_connectionTimeout(timeout.m_connectionTimeout)
	, m_networkTimeout(timeout.m_networkTimeout) { }

HTTPTimeout::HTTPTimeout(const HTTPTimeout & timeout)
	: m_connectionTimeout(timeout.m_connectionTimeout)
	, m_networkTimeout(timeout.m_networkTimeout) { }

HTTPTimeout & HTTPTimeout::operator = (HTTPTimeout && timeout) noexcept {
	if(this != &timeout) {
		std::lock_guard<std::recursive_mutex> lock(m_timeoutMutex);
		std::lock_guard<std::recursive_mutex> otherLock(timeout.m_timeoutMutex);

		m_connectionTimeout = timeout.m_connectionTimeout;
		m_networkTimeout = timeout.m_networkTimeout;
	}

	return *this;
}

HTTPTimeout & HTTPTimeout::operator = (const HTTPTimeout & timeout) {
	std::lock_guard<std::recursive_mutex> lock(m_timeoutMutex);
	std::lock_guard<std::recursive_mutex> otherLock(timeout.m_timeoutMutex);

	m_connectionTimeout = timeout.m_connectionTimeout;
	m_networkTimeout = timeout.m_networkTimeout;

	return *this;
}

HTTPTimeout::~HTTPTimeout() { }

bool HTTPTimeout::hasConnectionTimeout() const {
	std::lock_guard<std::recursive_mutex> lock(m_timeoutMutex);

	return m_connectionTimeout != 0ms;
}

std::chrono::seconds HTTPTimeout::getConnectionTimeout() const {
	std::lock_guard<std::recursive_mutex> lock(m_timeoutMutex);

	return m_connectionTimeout;
}

void HTTPTimeout::setConnectionTimeout(std::chrono::seconds timeout) {
	std::lock_guard<std::recursive_mutex> lock(m_timeoutMutex);

	m_connectionTimeout = timeout;
}

bool HTTPTimeout::hasNetworkTimeout() const {
	std::lock_guard<std::recursive_mutex> lock(m_timeoutMutex);

	return m_networkTimeout != 0ms;
}

std::chrono::seconds HTTPTimeout::getNetworkTimeout() const {
	std::lock_guard<std::recursive_mutex> lock(m_timeoutMutex);

	return m_networkTimeout;
}

void HTTPTimeout::setNetworkTimeout(std::chrono::seconds timeout) {
	std::lock_guard<std::recursive_mutex> lock(m_timeoutMutex);

	m_networkTimeout = timeout;
}

bool HTTPTimeout::operator == (const HTTPTimeout & timeout) const {
	std::lock_guard<std::recursive_mutex> lock(m_timeoutMutex);

	return m_connectionTimeout == timeout.m_connectionTimeout &&
		   m_networkTimeout == timeout.m_networkTimeout;
}

bool HTTPTimeout::operator != (const HTTPTimeout & timeout) const {
	std::lock_guard<std::recursive_mutex> lock(m_timeoutMutex);

	return !operator == (timeout);
}
