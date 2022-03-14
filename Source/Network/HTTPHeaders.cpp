#include "HTTPHeaders.h"

#include "Utilities/StringUtilities.h"

#include <fmt/core.h>

const HTTPHeaders::HeaderCase HTTPHeaders::DEFAULT_HEADER_CASE = HeaderCase::OriginalCase;
const std::string HTTPHeaders::CONTENT_TYPE_HEADER_NAME("Content-Type");
const std::string HTTPHeaders::CONTENT_LENGTH_HEADER_NAME("Content-Length");
const std::string HTTPHeaders::CONTENT_ENCODING_HEADER_NAME("Content-Encoding");
const std::string HTTPHeaders::DATE_HEADER_NAME("Date");
const std::string HTTPHeaders::LAST_MODIFIED_HEADER_NAME("Last-Modified");
const std::string HTTPHeaders::AGE_HEADER_NAME("Age");
const std::string HTTPHeaders::EXPIRES_HEADER_NAME("Expires");
const std::string HTTPHeaders::USER_AGENT_HEADER_NAME("User-Agent");
const std::string HTTPHeaders::AUTHORIZATION_HEADER_NAME("Authorization");
const std::string HTTPHeaders::ETAG_HEADER_NAME("ETag");
const std::string HTTPHeaders::IF_NONE_MATCH_HEADER_NAME("If-None-Match");
const std::string HTTPHeaders::IF_MATCH_HEADER_NAME("If-Match");
const std::string HTTPHeaders::APPLICATION_JSON_CONTENT_TYPE("application/json");
const std::string HTTPHeaders::APPLICATION_XML_CONTENT_TYPE("application/xml");
const std::string HTTPHeaders::TEXT_XML_CONTENT_TYPE("application/xml");

HTTPHeaders::HTTPHeaders(HeaderCase headerCase)
	: m_headerCase(headerCase)
	, m_readOnly(false) { }

HTTPHeaders::HTTPHeaders(HTTPHeaders && headers) noexcept
	: m_headers(std::move(headers.m_headers))
	, m_headerCase(headers.m_headerCase)
	, m_readOnly(headers.m_readOnly) { }

HTTPHeaders::HTTPHeaders(const HTTPHeaders & headers)
	: m_headers(headers.m_headers)
	, m_headerCase(headers.m_headerCase)
	, m_readOnly(headers.m_readOnly) { }

HTTPHeaders & HTTPHeaders::operator = (HTTPHeaders && headers) noexcept {
	if(this != &headers) {
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		std::lock_guard<std::recursive_mutex> otherLock(headers.m_mutex);

		m_headers = std::move(headers.m_headers);
		m_headerCase = headers.m_headerCase;
		m_readOnly = headers.m_readOnly;
	}

	return *this;
}

HTTPHeaders & HTTPHeaders::operator = (const HTTPHeaders & headers) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	std::lock_guard<std::recursive_mutex> otherLock(headers.m_mutex);

	m_headers = headers.m_headers;
	m_headerCase = headers.m_headerCase;
	m_readOnly = headers.m_readOnly;

	return *this;
}

HTTPHeaders::~HTTPHeaders() { }

bool HTTPHeaders::isReadOnly() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_readOnly;
}

void HTTPHeaders::setReadOnly() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_readOnly = true;
}

bool HTTPHeaders::hasHeaders() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return !m_headers.empty();
}

size_t HTTPHeaders::numberOfHeaders() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_headers.size();
}

bool HTTPHeaders::hasHeader(const std::string & headerName) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(headerName.empty()) {
		return false;
	}

	return m_headers.find(formatHeaderName(headerName)) != m_headers.end();
}

std::string HTTPHeaders::getHeaderValue(const std::string & headerName) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(headerName.empty()) {
		return {};
	}

	HeaderMap::const_iterator header(m_headers.find(formatHeaderName(headerName)));

	if(header == m_headers.end()) {
		return {};
	}

	return header->second;
}

const HTTPHeaders::HeaderMap & HTTPHeaders::getHeaders() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_headers;
}

bool HTTPHeaders::setHeader(const std::string & headerName, const std::string & headerValue) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly) {
		return false;
	}

	std::string formattedHeaderName(formatHeaderName(headerName));

	if(formattedHeaderName.empty()) {
		return false;
	}

	m_headers[formattedHeaderName] = headerValue;

	return true;
}

bool HTTPHeaders::isJSON() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return Utilities::compareStringsIgnoreCase(getContentMediaType(), APPLICATION_JSON_CONTENT_TYPE) == 0;
}

bool HTTPHeaders::isXML() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::string contentMediaType(getContentMediaType());

	return Utilities::compareStringsIgnoreCase(contentMediaType, APPLICATION_XML_CONTENT_TYPE) == 0 ||
		   Utilities::compareStringsIgnoreCase(contentMediaType, TEXT_XML_CONTENT_TYPE) == 0;
}

std::string HTTPHeaders::getRawContentType() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return getHeaderValue(CONTENT_TYPE_HEADER_NAME);
}

std::string HTTPHeaders::getContentMediaType() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::string contentType(getRawContentType());

	size_t mediaTypeSeparatorIndex = contentType.find_first_of(";");

	if(mediaTypeSeparatorIndex == std::string::npos) {
		return contentType;
	}

	return std::string(contentType.data(), mediaTypeSeparatorIndex);
}

std::string HTTPHeaders::getContentCharacterSet() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::string contentType(getRawContentType());

	size_t mediaTypeSeparatorIndex = contentType.find_first_of(";");

	if(mediaTypeSeparatorIndex == std::string::npos) {
		return {};
	}

	size_t characterSetStartIndex = contentType.find_first_not_of(" \t", mediaTypeSeparatorIndex + 1);

	if(characterSetStartIndex == std::string::npos) {
		return {};
	}

	std::string_view characterSetData(contentType.data() + characterSetStartIndex, contentType.length() - characterSetStartIndex);

	if(characterSetData.find("charset") != 0) {
		return {};
	}

	size_t characterSetDataSeparatorIndex = characterSetData.find_first_of("=");

	if(characterSetDataSeparatorIndex == std::string::npos) {
		return {};
	}

	return std::string(characterSetData.data() + characterSetDataSeparatorIndex + 1, characterSetData.length() - characterSetDataSeparatorIndex - 1);
}

bool HTTPHeaders::setContentType(const std::string & contentType) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return setHeader(CONTENT_TYPE_HEADER_NAME, contentType);
}

bool HTTPHeaders::clearContentType() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return removeHeader(CONTENT_TYPE_HEADER_NAME);
}

std::optional<uint64_t> HTTPHeaders::getContentLength() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!hasHeader(CONTENT_LENGTH_HEADER_NAME)) {
		return {};
	}

	return Utilities::parseUnsignedLong(getHeaderValue(CONTENT_LENGTH_HEADER_NAME));
}

bool HTTPHeaders::hasUserAgent() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return hasHeader(USER_AGENT_HEADER_NAME);
}

std::string HTTPHeaders::getUserAgent() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return getHeaderValue(USER_AGENT_HEADER_NAME);
}

bool HTTPHeaders::setUserAgent(const std::string & userAgent) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return setHeader(USER_AGENT_HEADER_NAME, userAgent);
}

bool HTTPHeaders::clearUserAgent() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return removeHeader(USER_AGENT_HEADER_NAME);
}

bool HTTPHeaders::hasAuthorization() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return hasHeader(AUTHORIZATION_HEADER_NAME);
}

std::string HTTPHeaders::getAuthorization() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return getHeaderValue(AUTHORIZATION_HEADER_NAME);
}

bool HTTPHeaders::setAuthorization(const std::string & token) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return setHeader(AUTHORIZATION_HEADER_NAME, token);
}

bool HTTPHeaders::setAuthorization(const std::string & userName, const std::string & password) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return setHeader(AUTHORIZATION_HEADER_NAME, createBasicAuthenticationToken(userName, password));
}

bool HTTPHeaders::clearAuthorization() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return removeHeader(AUTHORIZATION_HEADER_NAME);
}

std::string HTTPHeaders::getContentEncodingType() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return getHeaderValue(CONTENT_ENCODING_HEADER_NAME);
}

bool HTTPHeaders::removeHeader(const std::string & headerName) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly || headerName.empty()) {
		return false;
	}

	return m_headers.erase(formatHeaderName(headerName)) != 0;
}

bool HTTPHeaders::clearHeaders() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly) {
		return false;
	}

	m_headers.clear();

	return true;
}

HTTPHeaders::HeaderCase HTTPHeaders::getHeaderCase() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_headerCase;
}

bool HTTPHeaders::setHeaderCase(HeaderCase headerCase) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly || m_headerCase == headerCase) {
		return false;
	}

	m_headerCase = headerCase;

	HeaderMap updatedHeaders;

	for(HeaderMap::const_iterator i = m_headers.begin(); i != m_headers.end(); ++i) {
		updatedHeaders[formatHeaderName(i->first)] = i->second;
	}

	m_headers = std::move(updatedHeaders);

	return true;
}

std::string HTTPHeaders::formatETagValue(const std::string & eTag) {
	return fmt::format("\"{}\"", eTag);
}

std::string HTTPHeaders::extractETagValue(const std::string & rawETag) {
	if(rawETag.empty()) {
		return {};
	}

	size_t eTagStartIndex = rawETag.find_first_not_of("\"");
	size_t eTagEndIndex = rawETag.find_last_not_of("\"");

	if(eTagStartIndex == std::string::npos || eTagEndIndex == std::string::npos) {
		return {};
	}

	return std::string(rawETag.data() + eTagStartIndex, eTagEndIndex - eTagStartIndex + 1);
}

std::string HTTPHeaders::createBasicAuthenticationToken(const std::string & userName, const std::string & password) {
	static constexpr const char * BASIC_AUTHORIZATION_PREFIX = "Basic ";

	return BASIC_AUTHORIZATION_PREFIX + ByteBuffer(userName + ": " + password).toBase64();
}

bool HTTPHeaders::operator == (const HTTPHeaders & headers) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_headers == headers.m_headers;
}

bool HTTPHeaders::operator != (const HTTPHeaders & headers) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return !operator == (headers);
}

std::string HTTPHeaders::formatHeaderName(const std::string & headerName) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(headerName.empty()) {
		return {};
	}

	std::string formattedHeaderName;

	switch(m_headerCase) {
		case HeaderCase::OriginalCase: {
			formattedHeaderName = headerName;
			break;
		}

		case HeaderCase::UpperCase: {
			formattedHeaderName = Utilities::toUpperCase(headerName);
			break;
		}

		case HeaderCase::LowerCase: {
			formattedHeaderName = Utilities::toLowerCase(headerName);
			break;
		}

		case HeaderCase::HeaderCase: {
			formattedHeaderName = Utilities::toHeaderCase(headerName);
			break;
		}
	}

	return Utilities::trimString(formattedHeaderName);
}

bool HTTPHeaders::HeaderComparator::operator () (const std::string & headerA, const std::string & headerB) const {
	return std::lexicographical_compare(headerA.begin(), headerA.end(), headerB.begin(), headerB.end(), [](unsigned char a, unsigned char b) {
		return std::tolower(a) < std::tolower(b);
	});
}
