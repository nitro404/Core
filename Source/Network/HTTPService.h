#ifndef _HTTP_SERVICE_H_
#define _HTTP_SERVICE_H_

#include "HTTPConfiguration.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "HTTPStatusCode.h"
#include "HTTPTimeout.h"
#include "HTTPUtilities.h"

#define NOMINMAX
#include <curl/curl.h>

#include <cstdint>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class HTTPService final : public HTTPTimeout {
public:
	HTTPService();
	virtual ~HTTPService();

	bool isInitialized() const;
	bool initialize(const HTTPConfiguration & configuration = {}, bool autoStart = true);
	const HTTPConfiguration & getConfiguration() const;
	void setConfiguration(const HTTPConfiguration & configuration);
	bool isRunning() const;
	bool start();
	void stop();
	int64_t getMaximumRedirects() const;
	void setMaximumRedirects(int64_t maximumRedirects);
	bool hasMaximumActiveRequests() const;
	size_t numberOfActiveRequests() const;
	size_t getMaximumActiveRequests() const;
	bool setMaximumActiveRequests(size_t maximumActiveRequests);
	bool hasBaseURL() const;
	std::string getBaseURL() const;
	void setBaseURL(const std::string & baseURL);
	void clearBaseURL();
	bool hasUserAgent() const;
	const std::string & getUserAgent() const;
	void setUserAgent(const std::string & userAgent);
	void clearUserAgent();
	bool hasAuthorizationToken() const;
	std::string getAuthorizationToken() const;
	void setAuthorization(const std::string & bearerToken);
	void setAuthorization(const std::string & userName, const std::string & password);
	void clearAuthorization();
	std::shared_ptr<HTTPRequest> createRequest(HTTPRequest::Method method, const std::string & url);
	std::future<std::shared_ptr<HTTPResponse>> sendRequest(std::shared_ptr<HTTPRequest> request);
	bool abortRequest(std::shared_ptr<HTTPRequest> request);

	static const int64_t DEFAULT_MAXIMUM_REDIRECTS;
	static const size_t DEFAULT_MAXIMUM_ACTIVE_REQUESTS;

private:
	using HTTPThread = std::unique_ptr<std::thread, std::function<void (std::thread *)>>;

	void run();
	std::shared_ptr<HTTPResponse> createResponse(std::shared_ptr<HTTPRequest> request);

	bool m_initialized;
	bool m_running;
	bool m_stopRequested;
	HTTPConfiguration m_configuration;
	int64_t m_maximumRedirects;
	size_t m_maximumActiveRequests;
	std::string m_baseURL;
	std::string m_userAgent;
	std::string m_authorizationToken;
	HTTPThread m_httpThread;
	std::vector<std::weak_ptr<HTTPRequest>> m_requests;
	std::deque<std::shared_ptr<HTTPRequest>> m_pendingRequests;
	std::deque<std::shared_ptr<HTTPRequest>> m_abortedRequests;
	std::map<CURL *, std::shared_ptr<HTTPRequest>> m_activeRequests;
	mutable std::recursive_mutex m_mutex;
	mutable std::condition_variable_any m_waitCondition;
};

#endif // _HTTP_SERVICE_H_
