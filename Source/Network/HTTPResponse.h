#ifndef _HTTP_RESPONSE_H_
#define _HTTP_RESPONSE_H_

#include "BitmaskOperators.h"
#include "HTTPTransfer.h"

#include <chrono>
#include <future>
#include <memory>
#include <optional>

class HTTPRequest;

namespace tinyxml2 {
	class XMLElement;
}

class HTTPResponse final : public HTTPTransfer {
friend class HTTPService;

public:
	enum class State : uint16_t {
		None = 0,
		Connecting = 1,
		ReceivingHeaders = 1 << 1,
		ReceivingData = 1 << 2,
		Completed = 1 << 3,
		Aborted = 1 << 4,
		ConnectionTimedOut = 1 << 5,
		NetworkTimedOut = 1 << 6,
		TransferTimedOut = 1 << 7,
		Error = 1 << 8,
		Receiving = ReceivingData | ReceivingHeaders,
		TimedOut = ConnectionTimedOut | NetworkTimedOut | TransferTimedOut,
		Failed = Aborted | TimedOut | Error,
		Done = Completed | Failed
	};

	HTTPResponse(HTTPResponse && response) noexcept;
	HTTPResponse(const HTTPResponse & response);
	HTTPResponse & operator = (HTTPResponse && response) noexcept;
	HTTPResponse & operator = (const HTTPResponse & response);
	virtual ~HTTPResponse();

	size_t getSize() const;
	size_t getExpectedSize() const;
	bool isSuccessStatusCode() const;
	bool isFailureStatusCode() const;
	uint16_t getStatusCode() const;
	void setStatusCode(uint16_t statusCode);
	State getState() const;
	bool isConnecting() const;
	bool isReceiving() const;
	bool isCompleted() const;
	bool isAborted() const;
	bool canAbort() const;
	bool isTimedOut() const;
	bool isFailure() const;
	bool isDone() const;
	bool isConnectionInitiated() const;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> getConnectionInitiatedSystemTimePoint() const;
	std::optional<std::chrono::time_point<std::chrono::steady_clock>> getConnectionInitiatedSteadyTimePoint() const;
	bool isConnectionEstablished() const;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> getConnectionEstablishedSystemTimePoint() const;
	std::optional<std::chrono::time_point<std::chrono::steady_clock>> getConnectionEstablishedSteadyTimePoint() const;
	bool isTransferCompleted() const;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> getTransferCompletedSystemTimePoint() const;
	std::optional<std::chrono::time_point<std::chrono::steady_clock>> getTransferCompletedSteadyTimePoint() const;
	std::optional<std::chrono::milliseconds> getConnectionTimeElapsed() const;
	std::optional<std::chrono::milliseconds> getDataTransferTimeElapsed() const;
	std::optional<std::chrono::milliseconds> getRequestDuration() const;
	bool hasErrorMessage() const;
	const std::string & getErrorMessage() const;
	bool hasDate() const;
	std::string getDate() const;
	bool hasAge() const;
	std::string getAge() const;
	bool hasExpiresDate() const;
	std::string getExpiresDate() const;
	bool hasLastModifiedDate() const;
	std::string getLastModifiedDate() const;
	bool hasETag() const;
	std::string getETag() const;
	std::shared_ptr<HTTPRequest> getRequest() const;
	std::future<std::shared_ptr<HTTPResponse>> getFuture() const;

	// cURL callbacks
	static size_t receiveData(const char * data, size_t size, size_t numberOfBytes, void * context);
	static size_t receiveHeader(const char * data, size_t size, size_t numberOfBytes, void * context);

	bool operator == (const HTTPResponse & response) const;
	bool operator != (const HTTPResponse & response) const;

private:
	HTTPResponse(HTTPService * service, std::shared_ptr<HTTPRequest> request);

	bool setState(State state);
	bool checkTimeouts();
	bool appendHeader(const char * data, size_t size);
	bool appendData(const char * data, size_t size);
	void incrementTotalRawHeaderSizeBy(size_t size);
	void setTotalRawHeaderSize(size_t size);
	void resetTotalRawHeaderSize();
	void notifyProgress();
	void notifyCompleted();
	void notifyFailed();
	bool onTransferCompleted(bool success);
	bool onConnectionTimedOut();
	bool onNetworkTimedOut();
	bool onTransferTimedOut();
	bool onTransferAborted();
	bool onTransferError(const std::string & errorMessage);

	uint16_t m_statusCode;
	State m_state;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> m_connectionInitiatedSystemTimePoint;
	std::optional<std::chrono::time_point<std::chrono::steady_clock>> m_connectionInitiatedSteadyTimePoint;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> m_connectionEstablishedSystemTimePoint;
	std::optional<std::chrono::time_point<std::chrono::steady_clock>> m_connectionEstablishedSteadyTimePoint;
	std::optional<std::chrono::time_point<std::chrono::steady_clock>> m_lastDataReceivedSteadyTimePoint;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> m_transferCompletedSystemTimePoint;
	std::optional<std::chrono::time_point<std::chrono::steady_clock>> m_transferCompletedSteadyTimePoint;
	std::string m_localIPAddress;
	std::string m_primaryIPAddress;
	std::string m_lastReceivedHeaderName;
	size_t m_totalRawHeadersSize;
	size_t m_expectedSize;
	std::string m_errorMessage;
	std::weak_ptr<HTTPRequest> m_request;
	mutable std::promise<std::shared_ptr<HTTPResponse>> m_promise;
};

template<>
struct BitmaskOperators<HTTPResponse::State> {
	static const bool enabled = true;
};

#endif // _HTTP_RESPONSE_H_
