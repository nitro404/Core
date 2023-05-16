#include "HTTPResponse.h"

#include "HTTPRequest.h"
#include "HTTPStatusCode.h"
#include "Utilities/StringUtilities.h"

#include <fmt/core.h>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

HTTPResponse::HTTPResponse(HTTPService * service, std::shared_ptr<HTTPRequest> request)
	: HTTPTransfer(service)
	, m_statusCode(static_cast<uint16_t>(magic_enum::enum_integer(HTTPStatusCode::None)))
	, m_state(State::None)
	, m_totalRawHeadersSize(0u)
	, m_expectedSize(0u)
	, m_request(request) { }

HTTPResponse::HTTPResponse(HTTPResponse && response) noexcept
	: HTTPTransfer(response)
	, m_statusCode(response.m_statusCode)
	, m_state(response.m_state)
	, m_connectionInitiatedSystemTimePoint(response.m_connectionInitiatedSystemTimePoint)
	, m_connectionInitiatedSteadyTimePoint(response.m_connectionInitiatedSteadyTimePoint)
	, m_connectionEstablishedSystemTimePoint(response.m_connectionEstablishedSystemTimePoint)
	, m_connectionEstablishedSteadyTimePoint(response.m_connectionEstablishedSteadyTimePoint)
	, m_transferCompletedSystemTimePoint(response.m_transferCompletedSystemTimePoint)
	, m_transferCompletedSteadyTimePoint(response.m_transferCompletedSteadyTimePoint)
	, m_localIPAddress(std::move(response.m_localIPAddress))
	, m_primaryIPAddress(std::move(response.m_primaryIPAddress))
	, m_lastReceivedHeaderName(std::move(response.m_lastReceivedHeaderName))
	, m_totalRawHeadersSize(response.m_totalRawHeadersSize)
	, m_expectedSize(response.m_expectedSize)
	, m_errorMessage(std::move(response.m_errorMessage))
	, m_request(response.m_request) { }

HTTPResponse::HTTPResponse(const HTTPResponse & response)
	: HTTPTransfer(response)
	, m_statusCode(response.m_statusCode)
	, m_state(response.m_state)
	, m_connectionInitiatedSystemTimePoint(response.m_connectionInitiatedSystemTimePoint)
	, m_connectionInitiatedSteadyTimePoint(response.m_connectionInitiatedSteadyTimePoint)
	, m_connectionEstablishedSystemTimePoint(response.m_connectionEstablishedSystemTimePoint)
	, m_connectionEstablishedSteadyTimePoint(response.m_connectionEstablishedSteadyTimePoint)
	, m_transferCompletedSystemTimePoint(response.m_transferCompletedSystemTimePoint)
	, m_transferCompletedSteadyTimePoint(response.m_transferCompletedSteadyTimePoint)
	, m_localIPAddress(response.m_localIPAddress)
	, m_primaryIPAddress(response.m_primaryIPAddress)
	, m_lastReceivedHeaderName(response.m_lastReceivedHeaderName)
	, m_totalRawHeadersSize(response.m_totalRawHeadersSize)
	, m_expectedSize(response.m_expectedSize)
	, m_errorMessage(response.m_errorMessage)
	, m_request(response.m_request)  { }

HTTPResponse & HTTPResponse::operator = (HTTPResponse && response) noexcept {
	if(this != &response) {
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		std::lock_guard<std::recursive_mutex> otherLock(response.m_mutex);

		HTTPTransfer::operator = (response);

		m_statusCode = response.m_statusCode;
		m_state = response.m_state;
		m_connectionInitiatedSystemTimePoint = response.m_connectionInitiatedSystemTimePoint;
		m_connectionInitiatedSteadyTimePoint = response.m_connectionInitiatedSteadyTimePoint;
		m_connectionEstablishedSystemTimePoint = response.m_connectionEstablishedSystemTimePoint;
		m_connectionEstablishedSteadyTimePoint = response.m_connectionEstablishedSteadyTimePoint;
		m_transferCompletedSystemTimePoint = response.m_transferCompletedSystemTimePoint;
		m_transferCompletedSteadyTimePoint = response.m_transferCompletedSteadyTimePoint;
		m_localIPAddress = std::move(response.m_localIPAddress);
		m_primaryIPAddress = std::move(response.m_primaryIPAddress);
		m_lastReceivedHeaderName = std::move(response.m_lastReceivedHeaderName);
		m_totalRawHeadersSize = response.m_totalRawHeadersSize;
		m_expectedSize = response.m_expectedSize;
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
	m_connectionInitiatedSystemTimePoint = response.m_connectionInitiatedSystemTimePoint;
	m_connectionInitiatedSteadyTimePoint = response.m_connectionInitiatedSteadyTimePoint;
	m_connectionEstablishedSystemTimePoint = response.m_connectionEstablishedSystemTimePoint;
	m_connectionEstablishedSteadyTimePoint = response.m_connectionEstablishedSteadyTimePoint;
	m_transferCompletedSystemTimePoint = response.m_transferCompletedSystemTimePoint;
	m_transferCompletedSteadyTimePoint = response.m_transferCompletedSteadyTimePoint;
	m_localIPAddress = response.m_localIPAddress;
	m_primaryIPAddress = response.m_primaryIPAddress;
	m_lastReceivedHeaderName = response.m_lastReceivedHeaderName;
	m_totalRawHeadersSize = response.m_totalRawHeadersSize;
	m_expectedSize = response.m_expectedSize;
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

size_t HTTPResponse::getExpectedSize() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_expectedSize;
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

	return m_connectionInitiatedSystemTimePoint.has_value();
}

std::optional<std::chrono::time_point<std::chrono::system_clock>> HTTPResponse::getConnectionInitiatedSystemTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_connectionInitiatedSystemTimePoint;
}

std::optional<std::chrono::time_point<std::chrono::steady_clock>> HTTPResponse::getConnectionInitiatedSteadyTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_connectionInitiatedSteadyTimePoint;
}

bool HTTPResponse::isConnectionEstablished() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_connectionEstablishedSystemTimePoint.has_value();
}

std::optional<std::chrono::time_point<std::chrono::system_clock>> HTTPResponse::getConnectionEstablishedSystemTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_connectionEstablishedSystemTimePoint;
}

std::optional<std::chrono::time_point<std::chrono::steady_clock>> HTTPResponse::getConnectionEstablishedSteadyTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_connectionEstablishedSteadyTimePoint;
}

bool HTTPResponse::isTransferCompleted() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_transferCompletedSystemTimePoint.has_value();
}

std::optional<std::chrono::time_point<std::chrono::system_clock>> HTTPResponse::getTransferCompletedSystemTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_transferCompletedSystemTimePoint;
}

std::optional<std::chrono::time_point<std::chrono::steady_clock>> HTTPResponse::getTransferCompletedSteadyTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_transferCompletedSteadyTimePoint;
}

std::optional<std::chrono::milliseconds> HTTPResponse::getConnectionTimeElapsed() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::optional<std::chrono::time_point<std::chrono::steady_clock>> connectionInitiatedSteadyTimePoint(getConnectionInitiatedSteadyTimePoint());

	if(!connectionInitiatedSteadyTimePoint.has_value()) {
		return {};
	}

	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - connectionInitiatedSteadyTimePoint.value());
}

std::optional<std::chrono::milliseconds> HTTPResponse::getDataTransferTimeElapsed() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::optional<std::chrono::time_point<std::chrono::steady_clock>> connectionEstablishedSteadyTimePoint(getConnectionEstablishedSteadyTimePoint());

	if(!connectionEstablishedSteadyTimePoint.has_value()) {
		return {};
	}

	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - connectionEstablishedSteadyTimePoint.value());
}

std::optional<std::chrono::milliseconds> HTTPResponse::getRequestDuration() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::shared_ptr<HTTPRequest> request(m_request.lock());

	if(request == nullptr) {
		return 0ms;
	}

	std::optional<std::chrono::time_point<std::chrono::steady_clock>> requestInitiatedSteadyTimePoint(request->getRequestInitiatedSteadyTimePoint());

	if(!requestInitiatedSteadyTimePoint.has_value() || !m_transferCompletedSteadyTimePoint.has_value()) {
		return 0ms;
	}

	return std::chrono::duration_cast<std::chrono::milliseconds>(m_transferCompletedSteadyTimePoint.value() - requestInitiatedSteadyTimePoint.value());
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

	std::shared_ptr<HTTPRequest> request(m_request.lock());

	if(request == nullptr) {
		return false;
	}

	return request;
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

	if(!response->appendData(data, dataSize)) {
		return 0u;
	}

	response->m_lastDataReceivedSteadyTimePoint = std::chrono::steady_clock::now();

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

	response->m_lastDataReceivedSteadyTimePoint = std::chrono::steady_clock::now();

	response->setState(State::ReceivingHeaders);

	size_t dataSize = size * numberOfBytes;

	if(!response->appendHeader(data, dataSize)) {
		return 0u;
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
			m_connectionInitiatedSystemTimePoint = std::chrono::system_clock::now();
			m_connectionInitiatedSteadyTimePoint = std::chrono::steady_clock::now();

			return true;
		}

		case State::ReceivingHeaders: {
			if(m_state != State::Connecting) {
				return false;
			}

			m_state = State::ReceivingHeaders;
			m_connectionEstablishedSystemTimePoint = std::chrono::system_clock::now();
			m_connectionEstablishedSteadyTimePoint = std::chrono::steady_clock::now();

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
			m_transferCompletedSystemTimePoint = std::chrono::system_clock::now();
			m_transferCompletedSteadyTimePoint = std::chrono::steady_clock::now();
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

	std::shared_ptr<HTTPRequest> request(m_request.lock());

	if(request == nullptr) {
		return false;
	}

	if(m_state == State::Connecting) {
		if(request->getConnectionTimeout() != 0s) {
			std::optional<std::chrono::milliseconds> connectionTimeElapsed(getConnectionTimeElapsed());

			if(connectionTimeElapsed.has_value() && connectionTimeElapsed.value() > request->getConnectionTimeout()) {
				onConnectionTimedOut();
				return true;
			}
		}
	}
	else if(Any(m_state & State::Receiving)) {
		if(request->getNetworkTimeout() != 0s) {
			if(m_lastDataReceivedSteadyTimePoint.has_value() && std::chrono::steady_clock::now() - m_lastDataReceivedSteadyTimePoint.value() > request->getNetworkTimeout()) {
				onNetworkTimedOut();
				return true;
			}
		}

		if(request->getTransferTimeout() != 0s) {
			std::optional<std::chrono::milliseconds> dataTransferTimeElapsed(getDataTransferTimeElapsed());

			if(dataTransferTimeElapsed.has_value() && dataTransferTimeElapsed.value() > request->getTransferTimeout()) {
				onTransferTimedOut();
				return true;
			}
		}
	}

	return false;
}

bool HTTPResponse::appendHeader(const char * data, size_t size) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	incrementTotalRawHeaderSizeBy(size);

	std::string_view header(data, size);

	if(header.find("HTTP") == 0) {
		clearHeaders();
		setTotalRawHeaderSize(size);

		size_t statusCodeSeparatorIndex = header.find(" ");

		if(statusCodeSeparatorIndex == std::string::npos) {
			onTransferError("Malformed HTTP response header received, missing status code!");
			return false;
		}

		size_t statusNameSeparatorIndex = header.find(" ", statusCodeSeparatorIndex + 1);

		if(statusNameSeparatorIndex == std::string::npos) {
			onTransferError("Malformed HTTP response header received, missing status name!");
			return false;
		}

		std::string statusCodeData(std::string(header.data() + statusCodeSeparatorIndex + 1, statusNameSeparatorIndex - statusCodeSeparatorIndex - 1));
		std::optional<uint16_t> optionalStatusCode(Utilities::parseUnsignedShort(statusCodeData));

		if(!optionalStatusCode.has_value()) {
			onTransferError(fmt::format("Malformed HTTP response header received, invalid status code: '{}'!", statusCodeData));
			return false;
		}

		setStatusCode(optionalStatusCode.value());
	}
	else if(header.find_first_of(" \t") == 0) {
		size_t headerValueStartIndex = header.find_first_not_of(" \t");
		size_t headerValueEndIndex = header.find_last_not_of("\r\n");

		if(headerValueStartIndex == std::string::npos || headerValueEndIndex == std::string::npos) {
			onTransferError("Received invalid folded header.");
			return false;
		}

		std::string foldedHeaderValue(header.data() + headerValueStartIndex, headerValueEndIndex - headerValueStartIndex + 1);

		if(m_lastReceivedHeaderName.empty()) {
			onTransferError("Received unexpected folded header.");
			return false;
		}

		setHeader(m_lastReceivedHeaderName, getHeaderValue(m_lastReceivedHeaderName) + foldedHeaderValue);
	}
	else {
		size_t headerNameEndIndex = header.find_first_of(":");
		size_t headerValueEndIndex = header.find_last_not_of("\r\n");

		// ignore empty headers
		if(headerValueEndIndex == std::string::npos) {
			return size;
		}

		if(headerNameEndIndex == std::string::npos) {
			onTransferError(fmt::format("Malformed response header, missing separator: '{}'!", header));
			return false;
		}

		size_t headerValueStartIndex = header.find_first_not_of(" \t", headerNameEndIndex + 1);

		std::string headerName(header.data(), headerNameEndIndex);
		std::string headerValue;

		if(headerValueStartIndex != std::string::npos) {
			headerValue = std::string(header.data() + headerValueStartIndex, headerValueEndIndex - headerValueStartIndex + 1);
		}

		if(!setHeader(headerName, headerValue)) {
			onTransferError(fmt::format("Failed to set response header: '{}: {}'.", headerName, headerValue));
			return false;
		}

		m_lastReceivedHeaderName = headerName;

		// update body capacity based on content-length header when received
		if(Utilities::areStringsEqualIgnoreCase(headerName, HTTPHeaders::CONTENT_LENGTH_HEADER_NAME)) {
			std::optional<uint64_t> optionalContentLength(Utilities::parseUnsignedLong(headerValue));

			if(!optionalContentLength.has_value()) {
				onTransferError(fmt::format("Invalid '{}' header value: '{}'.", HTTPHeaders::CONTENT_LENGTH_HEADER_NAME, headerValue));
				return false;
			}

			if(getRequest()->getMethod() != HTTPRequest::Method::Head) {
				m_expectedSize = optionalContentLength.value();

				if(m_expectedSize > getBody()->getCapacity()) {
					getBody()->reserve(m_expectedSize);

					notifyProgress();
				}
			}
		}
	}

	return true;
}

bool HTTPResponse::appendData(const char * data, size_t size) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(data == nullptr || size == 0u || m_readOnly || m_state != State::ReceivingData) {
		return false;
	}

	m_body->writeBytes(reinterpret_cast<const uint8_t *>(data), size);

	notifyProgress();

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

void HTTPResponse::notifyProgress() {
	std::shared_ptr<HTTPRequest> request(m_request.lock());

	if(request == nullptr) {
		return;
	}
	
	request->progress(*request, m_body->getSize(), m_expectedSize);
}

void HTTPResponse::notifyCompleted() {
	std::shared_ptr<HTTPRequest> request(m_request.lock());

	if(request == nullptr) {
		return;
	}

	request->completed(*request);
}

void HTTPResponse::notifyFailed() {
	std::shared_ptr<HTTPRequest> request(m_request.lock());

	if(request == nullptr) {
		return;
	}

	request->failed(*request);
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

	std::shared_ptr<HTTPRequest> request(m_request.lock());

	if(request == nullptr) {
		return false;
	}

	int64_t responseCode = 0L;

	if(HTTPUtilities::isSuccess(curl_easy_getinfo(request->getCURLEasyHandle().get(), CURLINFO_RESPONSE_CODE, &responseCode))) {
		m_statusCode = static_cast<uint16_t>(responseCode);
	}
	else {
		spdlog::error("Failed to get HTTP status code.");
	}

	char * localIPAddress = nullptr;

	if(HTTPUtilities::isSuccess(curl_easy_getinfo(request->getCURLEasyHandle().get(), CURLINFO_LOCAL_IP, &localIPAddress))) {
		m_localIPAddress = localIPAddress;
	}

	char * primaryIPAddress = nullptr;

	if(HTTPUtilities::isSuccess(curl_easy_getinfo(request->getCURLEasyHandle().get(), CURLINFO_PRIMARY_IP, &primaryIPAddress))) {
		m_primaryIPAddress = primaryIPAddress;
	}

	std::optional<uint64_t> contentLength(getContentLength());

	if(request->getMethod() != HTTPRequest::Method::Head &&
	   contentLength.has_value() &&
	   m_body->getSize() != contentLength.value()) {
		onTransferError(fmt::format("Response size {} does not match '{}' header value of: {}.", m_body->getSize(), HTTPHeaders::CONTENT_LENGTH_HEADER_NAME, contentLength.value()));
	}
	else {
		setState(State::Completed);

		m_promise.set_value(request->getResponse());

		notifyCompleted();
	}

	return true;
}

bool HTTPResponse::onConnectionTimedOut() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(isDone()) {
		return false;
	}

	std::shared_ptr<HTTPRequest> request(m_request.lock());

	if(request == nullptr) {
		return false;
	}

	if(getConnectionTimeElapsed().has_value()) {
		m_errorMessage = fmt::format("Request connection timed out after {} milliseconds.", getConnectionTimeElapsed().value().count());
	}
	else {
		m_errorMessage = fmt::format("Request connection timed out.");
	}

	setState(State::ConnectionTimedOut);

	m_promise.set_value(request->getResponse());

	notifyFailed();

	return true;
}

bool HTTPResponse::onNetworkTimedOut() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(isDone()) {
		return false;
	}

	std::shared_ptr<HTTPRequest> request(m_request.lock());

	if(request == nullptr) {
		return false;
	}

	m_errorMessage = fmt::format("Request timed out with no data received for {} seconds.", request->getNetworkTimeout().count());

	setState(State::NetworkTimedOut);

	m_promise.set_value(request->getResponse());

	notifyFailed();

	return true;
}

bool HTTPResponse::onTransferTimedOut() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(isDone()) {
		return false;
	}

	std::shared_ptr<HTTPRequest> request(m_request.lock());

	if(request == nullptr) {
		return false;
	}

	if(getDataTransferTimeElapsed().has_value()) {
		m_errorMessage = fmt::format("Request data transfer timed out after {} milliseconds.", getDataTransferTimeElapsed().value().count());
	}
	else {
		m_errorMessage = fmt::format("Request data transfer timed out.");
	}

	setState(State::TransferTimedOut);

	m_promise.set_value(request->getResponse());

	notifyFailed();

	return true;
}

bool HTTPResponse::onTransferAborted() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_state != State::Aborted) {
		return false;
	}

	std::shared_ptr<HTTPRequest> request(m_request.lock());

	if(request == nullptr) {
		return false;
	}

	m_promise.set_value(request->getResponse());

	notifyFailed();

	return true;
}

bool HTTPResponse::onTransferError(const std::string & errorMessage) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_state == State::Error) {
		return false;
	}

	std::shared_ptr<HTTPRequest> request(m_request.lock());

	if(request == nullptr) {
		return false;
	}

	m_errorMessage = Utilities::trimString(errorMessage);

	setState(State::Error);

	m_promise.set_value(request->getResponse());

	notifyFailed();

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
