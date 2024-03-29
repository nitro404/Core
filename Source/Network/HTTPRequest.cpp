#include "HTTPRequest.h"

#include "HTTPResponse.h"
#include "HTTPService.h"
#include "HTTPUtilities.h"
#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

#include <fmt/core.h>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <optional>
#include <sstream>

using namespace std::chrono_literals;

const HTTPRequest::EncodingTypes HTTPRequest::DEFAULT_ACCEPTED_ENCODING_TYPES = HTTPRequest::EncodingTypes::Disabled;

HTTPRequest::HTTPRequest(HTTPRequest::Method method, const std::string & url, HTTPService * service)
	: HTTPTransfer(service)
	, HTTPRequestSettings()
	, m_method(method)
	, m_url(Utilities::trimString(url))
	, m_acceptedEncodingTypes(DEFAULT_ACCEPTED_ENCODING_TYPES) { }

HTTPRequest::HTTPRequest(HTTPRequest && request) noexcept
	: HTTPTransfer(std::move(request))
	, HTTPRequestSettings(std::move(request))
	, m_method(request.m_method)
	, m_url(std::move(request.m_url))
	, m_acceptedEncodingTypes(request.m_acceptedEncodingTypes)
	, m_requestInitiatedSystemTimePoint(request.m_requestInitiatedSystemTimePoint)
	, m_requestInitiatedSteadyTimePoint(request.m_requestInitiatedSteadyTimePoint)
	, m_transferStartedSystemTimePoint(request.m_transferStartedSystemTimePoint)
	, m_transferStartedSteadyTimePoint(request.m_transferStartedSteadyTimePoint)
	, m_rawHTTPHeaderList(std::move(request.m_rawHTTPHeaderList))
	, m_response(request.m_response)
	, m_curlEasyHandle(std::move(request.m_curlEasyHandle)) { }

HTTPRequest::HTTPRequest(const HTTPRequest & request)
	: HTTPTransfer(request)
	, HTTPRequestSettings(request)
	, m_method(request.m_method)
	, m_url(request.m_url)
	, m_acceptedEncodingTypes(request.m_acceptedEncodingTypes)
	, m_requestInitiatedSystemTimePoint(request.m_requestInitiatedSystemTimePoint)
	, m_requestInitiatedSteadyTimePoint(request.m_requestInitiatedSteadyTimePoint)
	, m_transferStartedSystemTimePoint(request.m_transferStartedSystemTimePoint)
	, m_transferStartedSteadyTimePoint(request.m_transferStartedSteadyTimePoint)
	, m_rawHTTPHeaderList(HTTPUtilities::copyCURLStringList(request.m_rawHTTPHeaderList))
	, m_response(request.m_response) { }

HTTPRequest & HTTPRequest::operator = (HTTPRequest && request) noexcept {
	if(this != &request) {
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		std::lock_guard<std::recursive_mutex> otherLock(request.m_mutex);

		HTTPTransfer::operator = (std::move(request));
		HTTPRequestSettings::operator = (std::move(request));

		m_method = request.m_method;
		m_url = std::move(request.m_url);
		m_acceptedEncodingTypes = request.m_acceptedEncodingTypes;
		m_requestInitiatedSystemTimePoint = request.m_requestInitiatedSystemTimePoint;
		m_requestInitiatedSteadyTimePoint = request.m_requestInitiatedSteadyTimePoint;
		m_transferStartedSystemTimePoint = request.m_transferStartedSystemTimePoint;
		m_transferStartedSteadyTimePoint = request.m_transferStartedSteadyTimePoint;
		m_rawHTTPHeaderList = std::move(request.m_rawHTTPHeaderList);
		m_response = request.m_response;
		m_curlEasyHandle = std::move(request.m_curlEasyHandle);
	}

	return *this;
}

HTTPRequest & HTTPRequest::operator = (const HTTPRequest & request) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	std::lock_guard<std::recursive_mutex> otherLock(request.m_mutex);

	HTTPTransfer::operator = (request);
	HTTPRequestSettings::operator = (request);

	m_method = request.m_method;
	m_url = request.m_url;
	m_acceptedEncodingTypes = request.m_acceptedEncodingTypes;
	m_requestInitiatedSystemTimePoint = request.m_requestInitiatedSystemTimePoint;
	m_requestInitiatedSteadyTimePoint = request.m_requestInitiatedSteadyTimePoint;
	m_transferStartedSystemTimePoint = request.m_transferStartedSystemTimePoint;
	m_transferStartedSteadyTimePoint = request.m_transferStartedSteadyTimePoint;
	m_rawHTTPHeaderList = HTTPUtilities::copyCURLStringList(request.m_rawHTTPHeaderList);
	m_response = request.m_response;

	return *this;
}

HTTPRequest::~HTTPRequest() { }

HTTPRequest::Method HTTPRequest::getMethod() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_method;
}

void HTTPRequest::setMethod(HTTPRequest::Method method) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly) {
		return;
	}

	m_method = method;
}

const std::string & HTTPRequest::getUrl() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_url;
}

void HTTPRequest::setUrl(const std::string & url) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly) {
		return;
	}

	m_url = Utilities::trimString(url);
}

bool HTTPRequest::hasAnyAcceptedEncodingTypes() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return Any(m_acceptedEncodingTypes);
}

HTTPRequest::EncodingTypes HTTPRequest::getAcceptedEncodingTypes() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_acceptedEncodingTypes;
}

std::string HTTPRequest::getAcceptedEncodingTypesAsString() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	static const std::array<EncodingTypes, 5> VALID_ENCODING_TYPES = {
		EncodingTypes::Identity,
		EncodingTypes::Deflate,
		EncodingTypes::GZip,
		EncodingTypes::Brotli,
		EncodingTypes::Zstandard
	};

	std::stringstream encodingTypesStringStream;

	for(EncodingTypes encodingType : VALID_ENCODING_TYPES) {
		if(Any(m_acceptedEncodingTypes & encodingType)) {
			if(encodingTypesStringStream.tellp() != 0) {
				encodingTypesStringStream << ", ";
			}

			encodingTypesStringStream << getEncodingTypeName(encodingType);
		}
	}

	return encodingTypesStringStream.str();
}

bool HTTPRequest::setAcceptedEncodingTypes(EncodingTypes encodingTypes) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly) {
		return false;
	}

	m_acceptedEncodingTypes = encodingTypes;

	return true;
}

bool HTTPRequest::clearAcceptedEncodingTypes() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return setAcceptedEncodingTypes(EncodingTypes::Disabled);
}

std::string HTTPRequest::getEncodingTypeName(EncodingTypes encodingType) {
	switch(encodingType) {
		case EncodingTypes::Identity:
			return "identity";

		case EncodingTypes::Deflate:
			return "deflate";

		case EncodingTypes::GZip:
			return "gzip";

		case EncodingTypes::Brotli:
			return "br";

		case EncodingTypes::Zstandard:
			return "zstd";

		default:
			break;
	}

	return {};
}

bool HTTPRequest::hasIfNoneMatchETag() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return hasHeader(HTTPHeaders::IF_NONE_MATCH_HEADER_NAME);
}

std::string HTTPRequest::getIfNoneMatchETag() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return HTTPHeaders::extractETagValue(getHeaderValue(HTTPHeaders::IF_NONE_MATCH_HEADER_NAME));
}

bool HTTPRequest::setIfNoneMatchETag(const std::string & eTag) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return setHeader(HTTPHeaders::IF_NONE_MATCH_HEADER_NAME, HTTPHeaders::formatETagValue(eTag));
}

bool HTTPRequest::clearIfNoneMatchETag() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return removeHeader(HTTPHeaders::IF_NONE_MATCH_HEADER_NAME);
}

bool HTTPRequest::hasIfMatchETag() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return hasHeader(HTTPHeaders::IF_MATCH_HEADER_NAME);
}

std::string HTTPRequest::getIfMatchETag() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return HTTPHeaders::extractETagValue(getHeaderValue(HTTPHeaders::IF_MATCH_HEADER_NAME));
}

bool HTTPRequest::setIfMatchETag(const std::string & eTag) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return setHeader(HTTPHeaders::IF_MATCH_HEADER_NAME, HTTPHeaders::formatETagValue(eTag));
}

bool HTTPRequest::clearIfMatchETag() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return removeHeader(HTTPHeaders::IF_MATCH_HEADER_NAME);
}

bool HTTPRequest::isRequestInitiated() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_requestInitiatedSystemTimePoint.has_value();
}

std::optional<std::chrono::time_point<std::chrono::system_clock>> HTTPRequest::getRequestInitiatedSystemTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_requestInitiatedSystemTimePoint;
}

std::optional<std::chrono::time_point<std::chrono::steady_clock>> HTTPRequest::getRequestInitiatedSteadyTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_requestInitiatedSteadyTimePoint;
}

bool HTTPRequest::isTransferStarted() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_transferStartedSystemTimePoint.has_value();
}

std::optional<std::chrono::time_point<std::chrono::system_clock>> HTTPRequest::getTransferStartedSystemTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_transferStartedSystemTimePoint;
}

std::optional<std::chrono::time_point<std::chrono::steady_clock>> HTTPRequest::getTransferStartedSteadyTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_transferStartedSteadyTimePoint;
}

std::shared_ptr<HTTPResponse> HTTPRequest::getResponse() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_response;
}

bool HTTPRequest::setResponse(std::shared_ptr<HTTPResponse> response) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(isRequestInitiated() ||
	   response == nullptr ||
	   response->getRequest().get() != this) {
		return false;
	}

	m_response = response;
	m_requestInitiatedSystemTimePoint = std::chrono::system_clock::now();
	m_requestInitiatedSteadyTimePoint = std::chrono::steady_clock::now();
	m_readOnly = true;

	return true;
}

HTTPResponse::State HTTPRequest::getState() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return HTTPResponse::State::None;
	}

	return m_response->getState();
}

bool HTTPRequest::isConnecting() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return false;
	}

	return m_response->isConnecting();
}

bool HTTPRequest::isReceiving() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return false;
	}

	return m_response->isReceiving();
}

bool HTTPRequest::isCompleted() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return false;
	}

	return m_response->isCompleted();
}

bool HTTPRequest::isAborted() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return false;
	}

	return m_response->isAborted();
}

bool HTTPRequest::canAbort() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return false;
	}

	return m_response->canAbort();
}

bool HTTPRequest::isTimedOut() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return false;
	}

	return m_response->isTimedOut();
}

bool HTTPRequest::isFailure() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return false;
	}

	return m_response->isFailure();
}

bool HTTPRequest::isDone() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return false;
	}

	return m_response->isDone();
}

bool HTTPRequest::isConnectionInitiated() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return false;
	}

	return m_response->isConnectionInitiated();
}

std::optional<std::chrono::time_point<std::chrono::system_clock>> HTTPRequest::getConnectionInitiatedSystemTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return {};
	}

	return m_response->getConnectionInitiatedSystemTimePoint();
}

std::optional<std::chrono::time_point<std::chrono::steady_clock>> HTTPRequest::getConnectionInitiatedSteadyTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return {};
	}

	return m_response->getConnectionInitiatedSteadyTimePoint();
}

bool HTTPRequest::isConnectionEstablished() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return false;
	}

	return m_response->isConnectionEstablished();
}

std::optional<std::chrono::time_point<std::chrono::system_clock>> HTTPRequest::getConnectionEstablishedSystemTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return {};
	}

	return m_response->getConnectionEstablishedSystemTimePoint();
}

std::optional<std::chrono::time_point<std::chrono::steady_clock>> HTTPRequest::getConnectionEstablishedSteadyTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return {};
	}

	return m_response->getConnectionEstablishedSteadyTimePoint();
}

bool HTTPRequest::isTransferCompleted() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return false;
	}

	return m_response->isTransferCompleted();
}

std::optional<std::chrono::time_point<std::chrono::system_clock>> HTTPRequest::getTransferCompletedSystemTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return {};
	}

	return m_response->getTransferCompletedSystemTimePoint();
}

std::optional<std::chrono::time_point<std::chrono::steady_clock>> HTTPRequest::getTransferCompletedSteadyTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return {};
	}

	return m_response->getTransferCompletedSteadyTimePoint();
}

std::optional<std::chrono::milliseconds> HTTPRequest::getConnectionTimeElapsed() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return {};
	}

	return m_response->getConnectionTimeElapsed();
}

std::optional<std::chrono::milliseconds> HTTPRequest::getDataTransferTimeElapsed() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return {};
	}

	return m_response->getDataTransferTimeElapsed();
}

std::optional<std::chrono::milliseconds> HTTPRequest::getRequestDuration() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return {};
	}

	return m_response->getRequestDuration();
}

bool HTTPRequest::hasErrorMessage() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return false;
	}

	return m_response->hasErrorMessage();
}

const std::string & HTTPRequest::getErrorMessage() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_response == nullptr) {
		return Utilities::emptyString;
	}

	return m_response->getErrorMessage();
}

bool HTTPRequest::startTransfer(const HTTPConfiguration & configuration, HTTPUtilities::CURLMultiHandle & curlMultiHandle) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	static const std::array<std::string, 1> IGNORED_HEADERS = {
		HTTPHeaders::USER_AGENT_HEADER_NAME
	};

	if(!m_requestInitiatedSystemTimePoint.has_value() || m_transferStartedSystemTimePoint.has_value() || m_response == nullptr || curlMultiHandle == nullptr) {
		return false;
	}

	m_curlEasyHandle = HTTPUtilities::createCURLEasyHandle();

	HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_DEBUGDATA, this), fmt::format("Failed to set cURL debug data on request #{}.", m_id));

	HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_DEBUGFUNCTION, &HTTPRequest::debugCallback), fmt::format("Failed to set cURL debug function on request #{}.", m_id));

	// enable verbose output for debug configurations
	if(m_verboseLoggingEnabled) {
		HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_VERBOSE, 1L), fmt::format("Failed to enable cURL verbose output mode on request #{}.", m_id));
	}

	// disable signals
	if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_NOSIGNAL, 1L), fmt::format("Failed to disable cURL from installing signal handlers for request #{}.", m_id))) {
		return false;
	}

	// configure method details
	std::optional<CURLoption> requestType;
	bool hasBody = false;

	switch(m_method) {
		case Method::Head: {
			requestType = CURLOPT_NOBODY;
			break;
		}

		case Method::Get: {
			requestType = CURLOPT_HTTPGET;
			break;
		}

		case Method::Patch: {
			requestType = CURLOPT_CUSTOMREQUEST;
			hasBody = true;
			break;
		}

		case Method::Put: {
			requestType = CURLOPT_CUSTOMREQUEST;
			hasBody = true;
			break;
		}

		case Method::Post: {
			requestType = CURLOPT_POST;
			hasBody = true;
			break;
		}

		case Method::Delete: {
			requestType = CURLOPT_CUSTOMREQUEST;
			hasBody = true;
			break;
		}
	}

	if(!requestType.has_value()) {
		return false;
	}

	// set request method type
	if(requestType.value() == CURLOPT_CUSTOMREQUEST) {
		if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_CUSTOMREQUEST, Utilities::toUpperCase(magic_enum::enum_name(m_method))), fmt::format("Failed to set cURL request #{} to custom type '{}'.", m_id, magic_enum::enum_name(m_method)))) {
			return false;
		}
	}
	else {
		if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), requestType.value(), 1L), fmt::format("Failed to set cURL request #{} type to '{}'.", m_id, magic_enum::enum_name(m_method)))) {
			return false;
		}
	}

	// set request URL
	std::string formattedURL;

	if(!configuration.baseURL.empty() &&
	   !(m_url.find("http://") == 0 || m_url.find("https://") == 0)) {
		formattedURL = Utilities::joinPaths(configuration.baseURL, m_url);
	}
	else {
		formattedURL = m_url;
	}

	formattedURL = Utilities::replaceAll(formattedURL, " ", "%20");

	if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_URL, formattedURL.c_str()), fmt::format("Failed to set cURL request #{} URL to '{}'.", m_id, m_url))) {
		return false;
	}

	// set request user agent
	if(hasUserAgent()) {
		if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_USERAGENT, getUserAgent().c_str()), fmt::format("Failed to set cURL request #{} user agent to '{}'.", m_id, getUserAgent()))) {
			return false;
		}
	}

	// set request body
	if(hasBody) {
		if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_POSTFIELDSIZE, m_body->getSize()), fmt::format("Failed to set cURL request #{} body size to {}.", m_id, m_body->getSize()))) {
			return false;
		}

		if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_POSTFIELDS, m_body->isEmpty() ? nullptr : m_body->getRawData()), fmt::format("Failed to set cURL request #{} body data.", m_id))) {
			return false;
		}
	}

	// set request headers
	if(hasHeaders()) {
		m_rawHTTPHeaderList = HTTPUtilities::createCURLStringList();

		for(HTTPHeaders::HeaderMap::const_iterator i = m_headers.begin(); i != m_headers.end(); ++i) {
			bool ignoreHeader = false;

			for(const std::string & ignoredHeader : IGNORED_HEADERS) {
				if(Utilities::areStringsEqualIgnoreCase(i->first, ignoredHeader)) {
					ignoreHeader = true;
					break;
				}
			}

			if(ignoreHeader) {
				continue;
			}

			std::string header(fmt::format("{}: {}", i->first, i->second));

			if(!HTTPUtilities::appendToCURLStringList(m_rawHTTPHeaderList, header)) {
				spdlog::error("Failed to append header '{}' to cURL string list for cURL request #{}.", header, m_id);
				return false;
			}
		}

		if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_HTTPHEADER, m_rawHTTPHeaderList.get()), fmt::format("Failed to set cURL request #{} headers.", m_id))) {
			return false;
		}
	}

	// configure accepted encoding types
	if(hasAnyAcceptedEncodingTypes()) {
		std::string acceptedEncodingTypes(getAcceptedEncodingTypesAsString());

		if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_ACCEPT_ENCODING, acceptedEncodingTypes.c_str()), fmt::format("Failed to set cURL request #{} accepted encoding types to: '{}'.", m_id, acceptedEncodingTypes))) {
			return false;
		}
	}

	// configure re-direct following
	if(m_maximumRedirects != 0) {
		if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_FOLLOWLOCATION, 1L), fmt::format("Failed to set cURL follow re-directs flag."))) {
			return false;
		}

		if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_MAXREDIRS, m_maximumRedirects), fmt::format("Failed to set cURL maximum re-directs property to {}.", m_maximumRedirects))) {
			return false;
		}
	}

	// configure SSL certificate authority bundle type
	if(!configuration.certificateAuthorityCertificateStoreDirectoryPath.empty()) {
		if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_SSLCERTTYPE, "PEM"), fmt::format("Failed to set cURL request #{} SSL certificate authority store file type.", m_id))) {
			return false;
		}

		// set SSL certificate authority bundle file path
		// updated cacert.pem files can be obtained from: https://curl.se/docs/caextract.html
		std::string certificateAuthorityCertificateStoreFilePath(Utilities::joinPaths(configuration.certificateAuthorityCertificateStoreDirectoryPath, HTTPService::CERTIFICATE_AUTHORITY_CERTIFICATE_STORE_FILE_NAME));

		if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_CAINFO, certificateAuthorityCertificateStoreFilePath.c_str()), fmt::format("Failed to set cURL request #{} SSL certificate authority store file path to: '{}'.", m_id, certificateAuthorityCertificateStoreFilePath))) {
			return false;
		}
	}

	// enable SSL host verification
	static constexpr const int64_t DISCONNECT_ON_HOST_VERIFICATION_FAILURE = 2L;

	if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_SSL_VERIFYHOST, DISCONNECT_ON_HOST_VERIFICATION_FAILURE), fmt::format("Failed to set disconnect on SSL host verification failure flag for cURL request #{}.", m_id))) {
		return false;
	}

	// enable SSL peer verification
	if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_SSL_VERIFYPEER, 1L), fmt::format("Failed to enable SSL peer verification flag for cURL request #{}.", m_id))) {
		return false;
	}

	// configure receive headers callback
	if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_HEADERFUNCTION, HTTPResponse::receiveHeader), fmt::format("Failed to set receive header function for cURL request #{}.", m_id))) {
		return false;
	}

	if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_HEADERDATA, static_cast<void *>(m_response.get())), fmt::format("Failed set receive header function context for cURL request #{}.", m_id))) {
		return false;
	}

	// configure receive data callback
	if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_WRITEFUNCTION, HTTPResponse::receiveData), fmt::format("Failed to set write function for cURL request #{}.", m_id))) {
		return false;
	}

	if(!HTTPUtilities::isSuccess(curl_easy_setopt(m_curlEasyHandle.get(), CURLOPT_WRITEDATA, static_cast<void *>(m_response.get())), fmt::format("Failed to set write function context for cURL request #{}.", m_id))) {
		return false;
	}

	// add cURL easy handle to multi handle
	if(!HTTPUtilities::isSuccess(curl_multi_add_handle(curlMultiHandle.get(), m_curlEasyHandle.get()), fmt::format("Failed to start cURL request #{} transfer.", m_id))) {
		return false;
	}

	m_transferStartedSystemTimePoint = std::chrono::system_clock::now();
	m_transferStartedSteadyTimePoint = std::chrono::steady_clock::now();

	return true;
}

int HTTPRequest::debugCallback(CURL * handle, curl_infotype type, char * data, size_t size, void * userData) {
	if(userData == nullptr) {
		return 0;
	}

	return reinterpret_cast<HTTPRequest *>(userData)->debugCallbackHelper(handle, type, data, size);
}

int HTTPRequest::debugCallbackHelper(CURL * handle, curl_infotype type, char * data, size_t size) {
	if(handle == nullptr) {
		return 0;
	}

	switch(type) {
		case CURLINFO_TEXT: {
			if(data != nullptr) {
				std::string_view text(data, size);
				size_t textEndIndex = text.find_last_not_of("\r\n");
				spdlog::debug("cURL: {}", text.substr(0, textEndIndex != std::string::npos ? textEndIndex + 1 : text.length()));
			}

			break;
		}

		case CURLINFO_HEADER_IN:
		case CURLINFO_HEADER_OUT:
		case CURLINFO_DATA_IN:
		case CURLINFO_DATA_OUT:
		case CURLINFO_SSL_DATA_IN:
		case CURLINFO_SSL_DATA_OUT:
		case CURLINFO_END: {
			break;
		}
	}

	return 0;
}

HTTPUtilities::CURLEasyHandle & HTTPRequest::getCURLEasyHandle() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_curlEasyHandle;
}

bool HTTPRequest::operator == (const HTTPRequest & request) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return HTTPTransfer::operator == (request);
}

bool HTTPRequest::operator != (const HTTPRequest & request) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return !operator == (request);
}
