#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include "BitmaskOperators.h"
#include "HTTPConfiguration.h"
#include "HTTPResponse.h"
#include "HTTPTimeout.h"
#include "HTTPTransfer.h"
#include "HTTPUtilities.h"

#include <chrono>
#include <memory>
#include <optional>
#include <string>

class HTTPService;

class HTTPRequest final : public HTTPTransfer,
                          public HTTPTimeout {
	friend class HTTPResponse;
	friend class HTTPService;

public:
	enum class Method {
		Head,
		Get,
		Patch,
		Put,
		Post,
		Delete
	};

	enum class EncodingTypes : uint8_t {
		Disabled = 0,
		Identity = 1,
		Deflate = 1 << 1,
		GZip = 1 << 2,
		Brotli = 1 << 3,
		Zstandard = 1 << 4,
		All = Identity | Deflate | GZip | Brotli | Zstandard
	};

	HTTPRequest(HTTPRequest && request) noexcept;
	HTTPRequest(const HTTPRequest & request);
	HTTPRequest & operator = (HTTPRequest && request) noexcept;
	HTTPRequest & operator = (const HTTPRequest & request);
	virtual ~HTTPRequest();

	Method getMethod() const;
	void setMethod(Method method);
	const std::string & getUrl() const;
	void setUrl(const std::string & url);
	bool hasAnyAcceptedEncodingTypes() const;
	EncodingTypes getAcceptedEncodingTypes() const;
	std::string getAcceptedEncodingTypesAsString() const;
	bool setAcceptedEncodingTypes(EncodingTypes encodingTypes);
	bool clearAcceptedEncodingTypes();
	static std::string getEncodingTypeName(EncodingTypes encodingType);
	bool hasIfNoneMatchETag() const;
	std::string getIfNoneMatchETag() const;
	bool setIfNoneMatchETag(const std::string & eTag);
	bool clearIfNoneMatchETag();
	bool hasIfMatchETag() const;
	std::string getIfMatchETag() const;
	bool setIfMatchETag(const std::string & eTag);
	bool clearIfMatchETag();
	bool isRequestInitiated() const;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> getRequestInitiatedTimePoint() const;
	bool isTransferStarted() const;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> getTransferStartedTimePoint() const;
	std::shared_ptr<HTTPResponse> getResponse() const;

	// HTTPResponse aliases
	HTTPResponse::State getState() const;
	bool isConnecting() const;
	bool isReceiving() const;
	bool isCompleted() const;
	bool isAborted() const;
	bool canAbort() const;
	bool isTimedOut() const;
	bool isFailure() const;
	bool isDone() const;
	bool isConnectionInitiated() const;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> getConnectionInitiatedTimePoint() const;
	bool isConnectionEstablished() const;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> getConnectionEstablishedTimePoint() const;
	bool isTransferCompleted() const;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> getTransferCompletedTimePoint() const;
	std::optional<std::chrono::milliseconds> getConnectionTimeElapsed() const;
	std::optional<std::chrono::milliseconds> getDataTransferTimeElapsed() const;
	std::optional<std::chrono::milliseconds> getRequestDuration() const;
	bool hasErrorMessage() const;
	const std::string & getErrorMessage() const;

	bool operator == (const HTTPRequest & request) const;
	bool operator != (const HTTPRequest & request) const;

	static const EncodingTypes DEFAULT_ACCEPTED_ENCODING_TYPES;

private:
	HTTPRequest(Method method, const std::string & url, HTTPService * service);

	bool setResponse(std::shared_ptr<HTTPResponse> response);
	bool startTransfer(HTTPConfiguration & configuration, HTTPUtilities::CURLMultiHandle & curlMultiHandle);
	HTTPUtilities::CURLEasyHandle & getCURLEasyHandle();

	Method m_method;
	std::string m_url;
	EncodingTypes m_acceptedEncodingTypes;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> m_requestInitiatedTimePoint;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> m_transferStartedTimePoint;
	HTTPUtilities::CURLStringList m_rawHTTPHeaderList;
	std::shared_ptr<HTTPResponse> m_response;
	HTTPUtilities::CURLEasyHandle m_curlEasyHandle;
};

template<>
struct BitmaskOperators<HTTPRequest::EncodingTypes> {
	static const bool enabled = true;
};

#endif // _HTTP_REQUEST_H_
