#ifndef _HTTP_HEADERS_H_
#define _HTTP_HEADERS_H_

#include "ByteBuffer.h"

#include <map>
#include <mutex>
#include <optional>
#include <string>

class HTTPHeaders {
public:
	enum class HeaderCase {
		OriginalCase,
		UpperCase,
		LowerCase,
		HeaderCase
	};

	struct HeaderComparator {
	public:
		bool operator () (const std::string & headerA, const std::string & headerB) const;
	};

	using HeaderMap = std::map<std::string, std::string, HeaderComparator>;

	HTTPHeaders(HeaderCase headerCase = DEFAULT_HEADER_CASE);
	HTTPHeaders(HTTPHeaders && request) noexcept;
	HTTPHeaders(const HTTPHeaders & request);
	HTTPHeaders & operator = (HTTPHeaders && request) noexcept;
	HTTPHeaders & operator = (const HTTPHeaders & request);
	virtual ~HTTPHeaders();

	bool isReadOnly() const;
	void setReadOnly();
	bool hasHeaders() const;
	size_t numberOfHeaders() const;
	bool hasHeader(const std::string & headerName) const;
	std::string getHeaderValue(const std::string & headerName) const;
	const HeaderMap & getHeaders() const;
	bool setHeader(const std::string & headerName, const std::string & headerValue);
	bool isJSON() const;
	bool isXML() const;
	std::string getRawContentType() const;
	std::string getContentMediaType() const;
	std::string getContentCharacterSet() const;
	bool setContentType(const std::string & contentType);
	bool clearContentType();
	std::optional<uint64_t> getContentLength() const;
	bool hasUserAgent() const;
	std::string getUserAgent() const;
	bool setUserAgent(const std::string & userAgent);
	bool clearUserAgent();
	bool hasAuthorization() const;
	std::string getAuthorization() const;
	bool setAuthorization(const std::string & token);
	bool setAuthorization(const std::string & userName, const std::string & password);
	bool clearAuthorization();
	std::string getContentEncodingType() const;
	bool removeHeader(const std::string & headerName);
	bool clearHeaders();
	HeaderCase getHeaderCase() const;
	bool setHeaderCase(HeaderCase headerCase);

	static std::string formatETagValue(const std::string & eTag);
	static std::string extractETagValue(const std::string & rawETag);
	static std::string createBasicAuthenticationToken(const std::string & userName, const std::string & password);

	bool operator == (const HTTPHeaders & headers) const;
	bool operator != (const HTTPHeaders & headers) const;

	static const HeaderCase DEFAULT_HEADER_CASE;
	static const std::string CONTENT_TYPE_HEADER_NAME;
	static const std::string CONTENT_LENGTH_HEADER_NAME;
	static const std::string CONTENT_ENCODING_HEADER_NAME;
	static const std::string DATE_HEADER_NAME;
	static const std::string LAST_MODIFIED_HEADER_NAME;
	static const std::string AGE_HEADER_NAME;
	static const std::string EXPIRES_HEADER_NAME;
	static const std::string USER_AGENT_HEADER_NAME;
	static const std::string AUTHORIZATION_HEADER_NAME;
	static const std::string ETAG_HEADER_NAME;
	static const std::string IF_NONE_MATCH_HEADER_NAME;
	static const std::string IF_MATCH_HEADER_NAME;
	static const std::string APPLICATION_JSON_CONTENT_TYPE;
	static const std::string APPLICATION_XML_CONTENT_TYPE;
	static const std::string TEXT_XML_CONTENT_TYPE;

protected:
	std::string formatHeaderName(const std::string & headerName) const;

	HeaderMap m_headers;
	HeaderCase m_headerCase;
	bool m_readOnly;
	mutable std::recursive_mutex m_mutex;
};

#endif // _HTTP_HEADERS_H_
