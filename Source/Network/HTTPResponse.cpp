#include "HTTPResponse.h"

#include "HTTPRequest.h"
#include "HTTPStatusCode.h"
#include "Utilities/StringUtilities.h"

#include <fmt/core.h>
#include <magic_enum.hpp>

using namespace std::chrono_literals;

HTTPResponse::HTTPResponse(HTTPService * service, std::shared_ptr<HTTPRequest> request)
	: HTTPTransfer(service)
	, m_statusCode(static_cast<uint16_t>(magic_enum::enum_integer(HTTPStatusCode::None)))
	, m_state(State::None)
	, m_totalRawHeadersSize(0u)
	, m_request(request) { }

HTTPResponse::HTTPResponse(HTTPResponse && response) noexcept
	: HTTPTransfer(response)
	, m_statusCode(response.m_statusCode)
	, m_state(response.m_state)
	, m_connectionInitiatedTimePoint(response.m_connectionInitiatedTimePoint)
	, m_connectionEstablishedTimePoint(response.m_connectionEstablishedTimePoint)
	, m_transferCompletedTimePoint(response.m_transferCompletedTimePoint)
	, m_localIPAddress(std::move(response.m_localIPAddress))
	, m_primaryIPAddress(std::move(response.m_primaryIPAddress))
	, m_lastReceivedHeaderName(std::move(response.m_lastReceivedHeaderName))
	, m_totalRawHeadersSize(response.m_totalRawHeadersSize)
	, m_errorMessage(std::move(response.m_errorMessage))
	, m_request(response.m_request) { }

HTTPResponse::HTTPResponse(const HTTPResponse & response)
	: HTTPTransfer(response)
	, m_statusCode(response.m_statusCode)
	, m_state(response.m_state)
	, m_connectionInitiatedTimePoint(response.m_connectionInitiatedTimePoint)
	, m_connectionEstablishedTimePoint(response.m_connectionEstablishedTimePoint)
	, m_transferCompletedTimePoint(response.m_transferCompletedTimePoint)
	, m_localIPAddress(response.m_localIPAddress)
	, m_primaryIPAddress(response.m_primaryIPAddress)
	, m_lastReceivedHeaderName(response.m_lastReceivedHeaderName)
	, m_totalRawHeadersSize(response.m_totalRawHeadersSize)
	, m_errorMessage(response.m_errorMessage)
	, m_request(response.m_request)  { }

HTTPResponse & HTTPResponse::operator = (HTTPResponse && response) noexcept {
	if(this != &response) {
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		std::lock_guard<std::recursive_mutex> otherLock(response.m_mutex);

		HTTPTransfer::operator = (response);

		m_statusCode = response.m_statusCode;
		m_state = response.m_state;
		m_connectionInitiatedTimePoint = response.m_connectionInitiatedTimePoint;
		m_connectionEstablishedTimePoint = response.m_connectionEstablishedTimePoint;
		m_transferCompletedTimePoint = response.m_transferCompletedTimePoint;
		m_localIPAddress = std::move(response.m_localIPAddress);
		m_primaryIPAddress = std::move(response.m_primaryIPAddress);
		m_lastReceivedHeaderName = std::move(response.m_lastReceivedHeaderName);
		m_totalRawHeadersSize = response.m_totalRawHeadersSize;
		m_errorMessage = std::move(response.m_errorMessage);
		m_request = response.m_request;
	}

	return *this;
}

HTTPResponse & HTTPResponse::operator = (const HTTPResponse & response) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	std::lock_guard<std::recursive_mutex> otherLock(response.m_mutex);

	HTTPTransfer::operator = (response);

	m_statusCode = response.m_statusCode;
	m_state = response.m_state;
	m_connectionInitiatedTimePoint = response.m_connectionInitiatedTimePoint;
	m_connectionEstablishedTimePoint = response.m_connectionEstablishedTimePoint;
	m_transferCompletedTimePoint = response.m_transferCompletedTimePoint;
	m_localIPAddress = response.m_localIPAddress;
	m_primaryIPAddress = response.m_primaryIPAddress;
	m_lastReceivedHeaderName = response.m_lastReceivedHeaderName;
	m_totalRawHeadersSize = response.m_totalRawHeadersSize;
	m_errorMessage = response.m_errorMessage;
	m_request = response.m_request;

	return *this;
}

HTTPResponse::~HTTPResponse() {
	m_service = nullptr;
}

size_t HTTPResponse::getSize() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_totalRawHeadersSize + m_body->getSize();
}

bool HTTPResponse::isSuccessStatusCode() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_statusCode < 400;
}

bool HTTPResponse::isFailureStatusCode() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_statusCode >= 400;
}

uint16_t HTTPResponse::getStatusCode() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_statusCode;
}

void HTTPResponse::setStatusCode(uint16_t statusCode) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly) {
		return;
	}

	m_statusCode = statusCode;
}

HTTPResponse::State HTTPResponse::getState() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_state;
}

bool HTTPResponse::isConnecting() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return Any(m_state & State::Connecting);
}

bool HTTPResponse::isReceiving() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return Any(m_state & State::Receiving);
}

bool HTTPResponse::isCompleted() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_state == State::Completed;
}

bool HTTPResponse::isAborted() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return Any(m_state & State::Aborted);
}

bool HTTPResponse::canAbort() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_state != State::None &&
		   None(m_state & State::Done);
}

bool HTTPResponse::isTimedOut() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return Any(m_state & State::TimedOut);
}

bool HTTPResponse::isFailure() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return Any(m_state & State::Failed);
}

bool HTTPResponse::isDone() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return Any(m_state & State::Done);
}

bool HTTPResponse::isConnectionInitiated() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_connectionInitiatedTimePoint.has_value();
}

std::optional<std::chrono::time_point<std::chrono::system_clock>> HTTPResponse::getConnectionInitiatedTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_connectionInitiatedTimePoint;
}

bool HTTPResponse::isConnectionEstablished() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_connectionEstablishedTimePoint.has_value();
}

std::optional<std::chrono::time_point<std::chrono::system_clock>> HTTPResponse::getConnectionEstablishedTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_connectionEstablishedTimePoint;
}

bool HTTPResponse::isTransferCompleted() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_transferCompletedTimePoint.has_value();
}

std::optional<std::chrono::time_point<std::chrono::system_clock>> HTTPResponse::getTransferCompletedTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_transferCompletedTimePoint;
}

std::optional<std::chrono::milliseconds> HTTPResponse::getConnectionTimeElapsed() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::optional<std::chrono::time_point<std::chrono::system_clock>> connectionInitiatedTimePoint(getConnectionInitiatedTimePoint());

	if(!connectionInitiatedTimePoint.has_value()) {
		return {};
	}

	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - connectionInitiatedTimePoint.value());
}

std::optional<std::chrono::milliseconds> HTTPResponse::getDataTransferTimeElapsed() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::optional<std::chrono::time_point<std::chrono::system_clock>> connectionEstablishedTimePoint(getConnectionEstablishedTimePoint());

	if(!connectionEstablishedTimePoint.has_value()) {
		return {};
	}

	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - connectionEstablishedTimePoint.value());
}

std::optional<std::chrono::milliseconds> HTTPResponse::getRequestDuration() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::optional<std::chrono::time_point<std::chrono::system_clock>> requestInitiatedTimePoint(m_request->getRequestInitiatedTimePoint());

	if(!requestInitiatedTimePoint.has_value() || !m_transferCompletedTimePoint.has_value()) {
		return 0ms;
	}

	return std::chrono::duration_cast<std::chrono::milliseconds>(m_transferCompletedTimePoint.value() - requestInitiatedTimePoint.value());
}

bool HTTPResponse::hasErrorMessage() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return !m_errorMessage.empty();
}

const std::string & HTTPResponse::getErrorMessage() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_errorMessage;
}

bool HTTPResponse::hasLastModifiedDate() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return hasHeader(LAST_MODIFIED_HEADER_NAME);
}

bool HTTPResponse::hasDate() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return hasHeader(HTTPHeaders::DATE_HEADER_NAME);
}

std::string HTTPResponse::getDate() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return getHeaderValue(HTTPHeaders::DATE_HEADER_NAME);
}

bool HTTPResponse::hasAge() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return hasHeader(HTTPHeaders::AGE_HEADER_NAME);
}

std::string HTTPResponse::getAge() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return getHeaderValue(HTTPHeaders::AGE_HEADER_NAME);
}

bool HTTPResponse::hasExpiresDate() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return hasHeader(HTTPHeaders::EXPIRES_HEADER_NAME);
}

std::string HTTPResponse::getExpiresDate() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return getHeaderValue(HTTPHeaders::EXPIRES_HEADER_NAME);
}

std::string HTTPResponse::getLastModifiedDate() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return getHeaderValue(LAST_MODIFIED_HEADER_NAME);
}

std::shared_ptr<HTTPRequest> HTTPResponse::getRequest() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_request;
}

bool HTTPResponse::hasETag() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return hasHeader(HTTPHeaders::ETAG_HEADER_NAME);
}

std::string HTTPResponse::getETag() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return HTTPHeaders::extractETagValue(getHeaderValue(HTTPHeaders::ETAG_HEADER_NAME));
}

std::future<std::shared_ptr<HTTPResponse>> HTTPResponse::getFuture() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_promise.get_future();
}

size_t HTTPResponse::receiveData(const char * data, size_t size, size_t numberOfBytes, void * context) {
	if(context == nullptr) {
		return 0u;
	}

	HTTPResponse * response = reinterpret_cast<HTTPResponse *>(context);

	if(response->isReadOnly()) {
		return 0u;
	}

	response->setState(State::ReceivingData);

	size_t dataSize = size * numberOfBytes;
	response->appendData(data, dataSize);

	return dataSize;
}

size_t HTTPResponse::receiveHeader(const char * data, size_t size, size_t numberOfBytes, void * context) {
	if(context == nullptr) {
		return 0u;
	}

	HTTPResponse * response = reinterpret_cast<HTTPResponse *>(context);

	if(response->isReadOnly()) {
		return 0u;
	}

	response->setState(State::ReceivingHeaders);

	size_t dataSize = size * numberOfBytes;

	response->incrementTotalRawHeaderSizeBy(dataSize);

	std::string_view header(data, dataSize);

	if(header.find("HTTP") == 0) {
		response->clearHeaders();
		response->setTotalRawHeaderSize(dataSize);

		size_t statusCodeSeparatorIndex = header.find(" ");

		if(statusCodeSeparatorIndex == std::string::npos) {
			response->onTransferError("Malformed HTTP response header received, missing status code!");
			return 0u;
		}

		size_t statusNameSeparatorIndex = header.find(" ", statusCodeSeparatorIndex + 1);

		if(statusNameSeparatorIndex == std::string::npos) {
			response->onTransferError("Malformed HTTP response header received, missing status name!");
			return 0u;
		}

		std::string statusCodeData(std::string(header.data() + statusCodeSeparatorIndex + 1, statusNameSeparatorIndex - statusCodeSeparatorIndex - 1));
		std::optional<uint16_t> optionalStatusCode(Utilities::parseUnsignedShort(statusCodeData));

		if(!optionalStatusCode.has_value()) {
			response->onTransferError(fmt::format("Malformed HTTP response header received, invalid status code: '{}'!", statusCodeData));
			return 0u;
		}

		response->setStatusCode(optionalStatusCode.value());
	}
	else if(header.find_first_of(" \t") == 0) {
		size_t headerValueStartIndex = header.find_first_not_of(" \t");
		size_t headerValueEndIndex = header.find_last_not_of("\r\n");

		if(headerValueStartIndex == std::string::npos || headerValueEndIndex == std::string::npos) {
			response->onTransferError("Received invalid folded header.");
			return 0u;
		}

		std::string foldedHeaderValue(header.data() + headerValueStartIndex, headerValueEndIndex - headerValueStartIndex + 1);

		if(response->m_lastReceivedHeaderName.empty()) {
			response->onTransferError("Received unexpected folded header.");
			return 0u;
		}

		response->setHeader(response->m_lastReceivedHeaderName, response->getHeaderValue(response->m_lastReceivedHeaderName) + foldedHeaderValue);
	}
	else {
		size_t headerNameEndIndex = header.find_first_of(":");
		size_t headerValueEndIndex = header.find_last_not_of("\r\n");

		// ignore empty headers
		if(headerValueEndIndex == std::string::npos) {
			return dataSize;
		}

		if(headerNameEndIndex == std::string::npos) {
			response->onTransferError(fmt::format("Malformed response header, missing separator: '{}'!", header));
			return 0u;
		}

		size_t headerValueStartIndex = header.find_first_not_of(" \t", headerNameEndIndex + 1);

		std::string headerName(header.data(), headerNameEndIndex);
		std::string headerValue;

		if(headerValueStartIndex != std::string::npos) {
			headerValue = std::string(header.data() + headerValueStartIndex, headerValueEndIndex - headerValueStartIndex + 1);
		}

		if(!response->setHeader(headerName, headerValue)) {
			response->onTransferError(fmt::format("Failed to set response header: '{}: {}'.", headerName, headerValue));
			return 0u;
		}

		response->m_lastReceivedHeaderName = headerName;

		// update body capacity based on content-length header when received
		if(Utilities::areStringsEqualIgnoreCase(headerName, HTTPHeaders::CONTENT_LENGTH_HEADER_NAME)) {
			std::optional<uint64_t> optionalContentLength(Utilities::parseUnsignedLong(headerValue));

			if(!optionalContentLength.has_value()) {
				response->onTransferError(fmt::format("Invalid '{}' header value: '{}'.", HTTPHeaders::CONTENT_LENGTH_HEADER_NAME, headerValue));
				return 0u;
			}

			if(response->getRequest()->getMethod() != HTTPRequest::Method::Head &&
			   optionalContentLength.value() > response->getBody()->getCapacity()) {
				response->getBody()->reserve(optionalContentLength.value());
			}
		}
	}

	return dataSize;
}

bool HTTPResponse::setState(State state) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_state == state || m_readOnly) {
		return false;
	}

	switch(state) {
		case State::None: {
			break;
		}

		case State::Connecting: {
			if(m_state != State::None) {
				return false;
			}

			m_state = State::Connecting;
			m_connectionInitiatedTimePoint = std::chrono::system_clock::now();

			return true;
		}

		case State::ReceivingHeaders: {
			if(m_state != State::Connecting) {
				return false;
			}

			m_state = State::ReceivingHeaders;
			m_connectionEstablishedTimePoint = std::chrono::system_clock::now();

			return true;
		}

		case State::ReceivingData: {
			if(m_state != State::ReceivingHeaders) {
				return false;
			}

			m_state = State::ReceivingData;

			return true;
		}

		case State::Completed: {
			if(None(m_state & State::Receiving) ||
			   Any(m_state & State::Done)) {
				return false;
			}

			m_state = State::Completed;
			m_transferCompletedTimePoint = std::chrono::system_clock::now();
			m_readOnly = true;

			return true;
		}

		case State::Aborted: {
			if(m_state == State::None ||
			   Any(m_state & State::Done)) {
				return false;
			}

			m_state = State::Aborted;
			m_readOnly = true;

			return true;
		}

		case State::NetworkTimedOut: {
			if(m_state == State::None ||
			   Any(m_state & State::Done)) {
				return false;
			}

			m_state = State::NetworkTimedOut;
			m_readOnly = true;

			return true;
		}

		case State::ConnectionTimedOut: {
			if(m_state == State::None || 
			   Any(m_state & State::Done)) {
				return false;
			}

			m_state = State::ConnectionTimedOut;
			m_readOnly = true;

			return true;
		}

		case State::Error: {
			if(Any(m_state & (State::Completed | State::Failed))) {
				return false;
			}

			m_state = State::Error;
			m_readOnly = true;

			return true;
		}
	}

	return false;
}

bool HTTPResponse::checkTimeouts() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_state == State::Connecting) {
		if(m_request->getConnectionTimeout() == 0s) {
			return false;
		}

		std::optional<std::chrono::milliseconds> connectionTimeElapsed(getConnectionTimeElapsed());

		if(!connectionTimeElapsed.has_value()) {
			return false;
		}

		if(connectionTimeElapsed.value() > m_request->getConnectionTimeout()) {
			onConnectionTimedOut();
			return true;
		}

		return false;
	}
	else if(Any(m_state & State::Receiving)) {
		if(m_request->getNetworkTimeout() == 0s) {
			return false;
		}

		std::optional<std::chrono::milliseconds> dataTransferTimeElapsed(getDataTransferTimeElapsed());

		if(!dataTransferTimeElapsed.has_value()) {
			return false;
		}

		if(dataTransferTimeElapsed.value() > m_request->getNetworkTimeout()) {
			onNetworkTimedOut();
			return true;
		}

		return false;
	}

	return false;
}

bool HTTPResponse::appendData(const char * data, size_t size) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(data == nullptr || size == 0u || m_readOnly || m_state != State::ReceivingData) {
		return false;
	}

	m_body->writeBytes(reinterpret_cast<const uint8_t *>(data), size);

	return true;
}

void HTTPResponse::incrementTotalRawHeaderSizeBy(size_t size) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	setTotalRawHeaderSize(m_totalRawHeadersSize + size);
}

void HTTPResponse::setTotalRawHeaderSize(size_t size) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly || m_state != State::ReceivingHeaders) {
		return;
	}

	m_totalRawHeadersSize = size;
}

void HTTPResponse::resetTotalRawHeaderSize() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	setTotalRawHeaderSize(0);
}

bool HTTPResponse::onTransferCompleted(bool success) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!success) {
		onTransferError("Request failed.");
		return false;
	}

	if(None(m_state & State::Receiving)) {
		return false;
	}

	int64_t responseCode = 0L;

	if(HTTPUtilities::isSuccess(curl_easy_getinfo(m_request->getCURLEasyHandle().get(), CURLINFO_RESPONSE_CODE, &responseCode))) {
		m_statusCode = static_cast<uint16_t>(responseCode);
	}
	else {
		fmt::print("Failed to get HTTP status code.\n");
	}

	char * localIPAddress = nullptr;

	if(HTTPUtilities::isSuccess(curl_easy_getinfo(m_request->getCURLEasyHandle().get(), CURLINFO_LOCAL_IP, &localIPAddress))) {
		m_localIPAddress = localIPAddress;
	}

	char * primaryIPAddress = nullptr;

	if(HTTPUtilities::isSuccess(curl_easy_getinfo(m_request->getCURLEasyHandle().get(), CURLINFO_PRIMARY_IP, &primaryIPAddress))) {
		m_primaryIPAddress = primaryIPAddress;
	}

	std::optional<uint64_t> contentLength(getContentLength());

	if(m_request->getMethod() != HTTPRequest::Method::Head &&
	   contentLength.has_value() &&
	   m_body->getSize() != contentLength.value()) {
		onTransferError(fmt::format("Response size {} does not match '{}' header value of: {}.", m_body->getSize(), HTTPHeaders::CONTENT_LENGTH_HEADER_NAME, contentLength.value()));
	}
	else {
		setState(State::Completed);

		m_promise.set_value(m_request->getResponse());
	}

	return true;
}

bool HTTPResponse::onConnectionTimedOut() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(isDone()) {
		return false;
	}

	if(getConnectionTimeElapsed().has_value()) {
		m_errorMessage = fmt::format("Request connection timed out after {} milliseconds.", getConnectionTimeElapsed().value().count());
	}
	else {
		m_errorMessage = fmt::format("Request connection timed out.");
	}

	setState(State::ConnectionTimedOut);

	m_promise.set_value(m_request->getResponse());

	return true;
}

bool HTTPResponse::onNetworkTimedOut() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(isDone()) {
		return false;
	}

	if(getDataTransferTimeElapsed().has_value()) {
		m_errorMessage = fmt::format("Request data transfer timed out after {} milliseconds.", getDataTransferTimeElapsed().value().count());
	}
	else {
		m_errorMessage = fmt::format("Request data transfer timed out.");
	}

	setState(State::NetworkTimedOut);

	m_promise.set_value(m_request->getResponse());

	return true;
}

bool HTTPResponse::onTransferAborted() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_state != State::Aborted) {
		return false;
	}

	m_promise.set_value(m_request->getResponse());

	return true;
}

bool HTTPResponse::onTransferError(const std::string & errorMessage) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_state == State::Error) {
		return false;
	}

	m_errorMessage = Utilities::trimString(errorMessage);

	setState(State::Error);

	m_promise.set_value(m_request->getResponse());

	return true;
}

bool HTTPResponse::operator == (const HTTPResponse & response) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return HTTPTransfer::operator == (response);
}

bool HTTPResponse::operator != (const HTTPResponse & response) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return !operator == (response);
}
