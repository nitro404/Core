#include "HTTPService.h"

#include "Platform/DeviceInformationBridge.h"
#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/ThreadUtilities.h"

#include <spdlog/spdlog.h>

#include <filesystem>

using namespace std::chrono_literals;

const size_t HTTPService::DEFAULT_MAXIMUM_ACTIVE_REQUESTS = 8u;
const std::string HTTPService::CERTIFICATE_AUTHORITY_CERTIFICATE_PAGE_BASE_URL("https://curl.se/ca");
const std::string HTTPService::CERTIFICATE_AUTHORITY_CERTIFICATE_STORE_FILE_NAME("cacert.pem");
const std::string HTTPService::CERTIFICATE_AUTHORITY_CERTIFICATE_STORE_SHA256_FILE_NAME_SUFFIX(".sha256");
const std::string HTTPService::INTERNET_CONNECTIVITY_CHECK_URL("http://connectivitycheck.gstatic.com/generate_204");
const std::chrono::seconds HTTPService::DEFAULT_INTERNET_CONNECTIVITY_CHECK_INTERVAL(15s);
const std::chrono::seconds HTTPService::DEFAULT_INTERNET_CONNECTIVITY_CHECK_TIMEOUT(1s);

HTTPService::HTTPService()
	: HTTPRequestSettings()
	, m_initialized(false)
	, m_running(false)
	, m_stopRequested(false)
	, m_maximumActiveRequests(DEFAULT_MAXIMUM_ACTIVE_REQUESTS)
	, m_internetConnectivityCheckInterval(DEFAULT_INTERNET_CONNECTIVITY_CHECK_INTERVAL)
	, m_internetConnectivityCheckTimeout(DEFAULT_INTERNET_CONNECTIVITY_CHECK_TIMEOUT)
	, m_updatingCertificateAuthorityCertificateStoreFile(false) { }

HTTPService::~HTTPService() {
	stop();

	for(std::weak_ptr<HTTPRequest> weakRequest : m_requests) {
		if(weakRequest.expired()) {
			continue;
		}

		std::shared_ptr<HTTPRequest> sharedRequest(weakRequest.lock());
		sharedRequest->clearService();
	}

	curl_global_cleanup();
}

bool HTTPService::isInitialized() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_initialized;
}

bool HTTPService::initialize(const HTTPConfiguration & configuration, bool autoStart) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_initialized) {
		return true;
	}

	if(!HTTPUtilities::isSuccess(curl_global_init(CURL_GLOBAL_DEFAULT), "Failed to initialize cURL")) {
		return false;
	}

	setConfiguration(configuration);

	m_initialized = true;

	if(autoStart) {
		start();
	}

	return true;
}

const HTTPConfiguration & HTTPService::getConfiguration() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_configuration;
}

void HTTPService::setConfiguration(const HTTPConfiguration & configuration) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_configuration = configuration;

	if(configuration.connectionTimeout.has_value() && configuration.connectionTimeout.value() > std::chrono::seconds::zero()) {
		m_connectionTimeout = configuration.connectionTimeout.value();
	}

	if(configuration.networkTimeout.has_value() && configuration.networkTimeout.value() > std::chrono::seconds::zero()) {
		m_networkTimeout = configuration.networkTimeout.value();
	}

	if(configuration.transferTimeout.has_value() && configuration.transferTimeout.value() > std::chrono::seconds::zero()) {
		m_transferTimeout = configuration.transferTimeout.value();
	}

	if(configuration.maximumRedirects.has_value()) {
		m_maximumRedirects = configuration.maximumRedirects.value();
	}

	{
		std::lock_guard<std::mutex> internetConnectivityLock(m_internetConnectivityMutex);

		if(configuration.internetConnectivityCheckInterval.has_value()) {
			m_internetConnectivityCheckInterval = configuration.internetConnectivityCheckInterval.value();
		}

		if(configuration.internetConnectivityCheckTimeout.has_value()) {
			m_internetConnectivityCheckTimeout = configuration.internetConnectivityCheckTimeout.value();
		}
	}
}

bool HTTPService::isRunning() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_running;
}

bool HTTPService::start() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized || m_running) {
		return false;
	}

	m_running = true;

	m_httpThread = HTTPThread(new std::thread(&HTTPService::run, this), [this](std::thread * httpThread) {
		m_running = false;

		if(httpThread != nullptr) {
			if(httpThread->joinable()) {
				httpThread->join();
			}

			delete httpThread;
		}
	});

	Utilities::setThreadName(*m_httpThread, "HTTP Thread");

	return true;
}

void HTTPService::stop() {
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);

		m_stopRequested = true;
	}

	m_waitCondition.notify_one();

	m_caCertUpdateThread.reset();

	m_httpThread.reset();

	std::lock_guard<std::mutex> internetConnectivityLock(m_internetConnectivityMutex);

	m_connectedToInternet.reset();
	m_lastInternetConnectivityCheckTimePoint.reset();
}

bool HTTPService::hasMaximumActiveRequests() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_activeRequests.size() >= m_maximumActiveRequests;
}

size_t HTTPService::numberOfActiveRequests() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_activeRequests.size();
}

size_t HTTPService::getMaximumActiveRequests() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_maximumActiveRequests;
}

bool HTTPService::setMaximumActiveRequests(size_t maximumActiveRequests) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(maximumActiveRequests < 1) {
		return false;
	}

	m_maximumActiveRequests = maximumActiveRequests;

	return true;
}

bool HTTPService::hasBaseURL() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return !m_configuration.baseURL.empty();
}

std::string HTTPService::getBaseURL() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_configuration.baseURL;
}

void HTTPService::setBaseURL(const std::string & baseURL) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_configuration.baseURL = Utilities::trimString(baseURL);
}

void HTTPService::clearBaseURL() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_configuration.baseURL.clear();
}

bool HTTPService::hasUserAgent() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return !m_userAgent.empty();
}

const std::string & HTTPService::getUserAgent() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_userAgent;
}

void HTTPService::setUserAgent(const std::string & userAgent) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_userAgent = Utilities::trimString(userAgent);
}

void HTTPService::clearUserAgent() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_userAgent.clear();
}

bool HTTPService::hasAuthorizationToken() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return !m_authorizationToken.empty();
}

std::string HTTPService::getAuthorizationToken() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return  m_authorizationToken;
}

void HTTPService::setAuthorization(const std::string & token) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_authorizationToken = token;
}

void HTTPService::setAuthorization(const std::string & userName, const std::string & password) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	setAuthorization(HTTPHeaders::createBasicAuthenticationToken(userName, password));
}

void HTTPService::clearAuthorization() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_authorizationToken.clear();
}

bool HTTPService::checkForInternetConnectivity(bool force) {
	std::lock_guard<std::mutex> internetConnectivityLock(m_internetConnectivityMutex);

	if(!DeviceInformationBridge::getInstance()->isConnectedToNetwork()) {
		m_connectedToInternet = false;
		return false;
	}

	if(!force && m_connectedToInternet.has_value() && m_lastInternetConnectivityCheckTimePoint.has_value() && std::chrono::steady_clock::now() - m_lastInternetConnectivityCheckTimePoint.value() < m_internetConnectivityCheckInterval) {
		return m_connectedToInternet.value();
	}

	std::shared_ptr<HTTPRequest> internetConnectivityCheckRequest(createRequest(HTTPRequest::Method::Get, INTERNET_CONNECTIVITY_CHECK_URL));
	internetConnectivityCheckRequest->setConnectionTimeout(m_internetConnectivityCheckTimeout);

	std::shared_ptr<HTTPResponse> internetConnectivityCheckResponse(sendRequestAndWait(internetConnectivityCheckRequest));

	bool connectedToInternet = static_cast<HTTPStatusCode>(internetConnectivityCheckResponse->getStatusCode()) == HTTPStatusCode::NoContent;

	m_connectedToInternet = connectedToInternet;

	return connectedToInternet;
}

std::string HTTPService::getCertificateAuthorityCertificateStoreFilePath() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return Utilities::joinPaths(m_configuration.certificateAuthorityCertificateStoreDirectoryPath, CERTIFICATE_AUTHORITY_CERTIFICATE_STORE_FILE_NAME);
}

bool HTTPService::hasCertificateAuthorityCertificateStoreFile() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return std::filesystem::is_regular_file(std::filesystem::path(getCertificateAuthorityCertificateStoreFilePath()));
}

std::future<bool> HTTPService::updateCertificateAuthorityCertificateStoreFile(bool force) {
	if(!checkForInternetConnectivity() && hasCertificateAuthorityCertificateStoreFile()) {
		return {};
	}

	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_caCertUpdateThread != nullptr && m_caCertUpdateThread->joinable()) {
		m_updatingCertificateAuthorityCertificateStoreFile = false;
		m_caCertUpdateThread->join();
	}

	std::shared_ptr<std::promise<bool>> caCertUpdatedPromise(std::make_shared<std::promise<bool>>());

	m_caCertUpdateThread = CACertUpdateThread(new std::thread(&HTTPService::runCertificateAuthorityCertificateStoreFileUpdate, this, caCertUpdatedPromise, getCertificateAuthorityCertificateStoreFilePath(), force), [this](std::thread * caCertUpdateThread) {
		m_updatingCertificateAuthorityCertificateStoreFile = false;

		if(caCertUpdateThread != nullptr) {
			if(caCertUpdateThread->joinable()) {
				caCertUpdateThread->join();
			}

			delete caCertUpdateThread;
		}
	});

	Utilities::setThreadName(*m_caCertUpdateThread, "CACert Update Thread");

	return caCertUpdatedPromise->get_future();
}

bool HTTPService::updateCertificateAuthorityCertificateStoreFileAndWait(bool force) {
	std::future<bool> caCertUpdateFuture(updateCertificateAuthorityCertificateStoreFile());

	if(!caCertUpdateFuture.valid()) {
		return false;
	}

	caCertUpdateFuture.wait();

	return caCertUpdateFuture.get();
}

std::shared_ptr<HTTPRequest> HTTPService::createRequest(HTTPRequest::Method method, const std::string & url) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::shared_ptr<HTTPRequest> newRequest(new HTTPRequest(method, url, this));
	newRequest->setConnectionTimeout(m_connectionTimeout);
	newRequest->setNetworkTimeout(m_networkTimeout);
	newRequest->setMaximumRedirects(m_maximumRedirects);
	newRequest->setVerboseLoggingEnabled(m_verboseLoggingEnabled);

	if(hasUserAgent()) {
		newRequest->setUserAgent(m_userAgent);
	}

	if(hasAuthorizationToken()) {
		newRequest->setAuthorization(m_authorizationToken);
	}

	m_requests.emplace_back(newRequest);

	return newRequest;
}

std::future<std::shared_ptr<HTTPResponse>> HTTPService::sendRequest(std::shared_ptr<HTTPRequest> request) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized ||
	   !m_running ||
	   request == nullptr ||
	   request->getService() != this ||
	   request->isRequestInitiated()) {
		return {};
	}

	std::shared_ptr<HTTPResponse> response(createResponse(request));
	request->setResponse(response);
	m_pendingRequests.push_back(request);
	m_waitCondition.notify_one();

	return response->getFuture();
}

std::shared_ptr<HTTPResponse> HTTPService::sendRequestAndWait(std::shared_ptr<HTTPRequest> request) {
	std::future<std::shared_ptr<HTTPResponse>> futureResponse(sendRequest(request));

	if(!futureResponse.valid()) {
		return nullptr;
	}

	futureResponse.wait();

	return futureResponse.get();
}

bool HTTPService::abortRequest(HTTPRequest & request) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::map<CURL *, std::shared_ptr<HTTPRequest>>::const_iterator activeRequestIterator(std::find_if(m_activeRequests.cbegin(), m_activeRequests.cend(), [&request](const std::pair<CURL *, std::shared_ptr<HTTPRequest>> & activeRequest) {
		return activeRequest.second.get() == &request;
	}));

	if(activeRequestIterator != m_activeRequests.cend()) {
		return abortRequest(activeRequestIterator->second);
	}

	std::deque<std::shared_ptr<HTTPRequest>>::const_iterator pendingRequestIterator(std::find_if(m_pendingRequests.cbegin(), m_pendingRequests.cend(), [&request](const std::shared_ptr<HTTPRequest> & pendingRequest) {
		return pendingRequest.get() == &request;
	}));

	if(pendingRequestIterator != m_pendingRequests.cend()) {
		return abortRequest(*pendingRequestIterator);
	}

	return false;
}

bool HTTPService::abortRequest(std::shared_ptr<HTTPRequest> request) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(request == nullptr || !request->isRequestInitiated() || !request->getResponse()->setState(HTTPResponse::State::Aborted)) {
		return false;
	}

	m_pendingRequests.erase(std::remove(m_pendingRequests.begin(), m_pendingRequests.end(), request), m_pendingRequests.end());
	m_activeRequests.erase(request->getCURLEasyHandle().get());
	m_abortedRequests.push_back(request);
	m_waitCondition.notify_one();

	return true;
}

std::shared_ptr<HTTPResponse> HTTPService::createResponse(std::shared_ptr<HTTPRequest> request) {
	if(request == nullptr || request->getService() != this) {
		return nullptr;
	}

	std::shared_ptr<HTTPResponse> response(new HTTPResponse(this, request));

	return response;
}

void HTTPService::runCertificateAuthorityCertificateStoreFileUpdate(std::shared_ptr<std::promise<bool>> promise, const std::string & caCertFilePath, bool force) {
	if(!m_initialized || !m_running) {
		promise->set_value(false);
		return;
	}

	m_updatingCertificateAuthorityCertificateStoreFile = true;

	std::shared_ptr<HTTPRequest> caCertSHA256FileRequest(createRequest(HTTPRequest::Method::Get, Utilities::joinPaths(CERTIFICATE_AUTHORITY_CERTIFICATE_PAGE_BASE_URL, CERTIFICATE_AUTHORITY_CERTIFICATE_STORE_FILE_NAME + CERTIFICATE_AUTHORITY_CERTIFICATE_STORE_SHA256_FILE_NAME_SUFFIX)));

	std::shared_ptr<HTTPResponse> caCertSHA256FileResponse(sendRequestAndWait(caCertSHA256FileRequest));

	if(caCertSHA256FileResponse->isFailure()) {
		spdlog::error("Failed to download certificate authority certificate store SHA256 file with error: {}", caCertSHA256FileResponse->getErrorMessage());
		promise->set_value(false);
		return;
	}
	else if(caCertSHA256FileResponse->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(caCertSHA256FileResponse->getStatusCode()));
		spdlog::error("Failed to download certificate authority certificate store SHA256 file ({}{})!", caCertSHA256FileResponse->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		promise->set_value(false);
		return;
	}

	if(!m_updatingCertificateAuthorityCertificateStoreFile) {
		promise->set_value(false);
		return;
	}

	std::string expectedCACertSHA256Data(caCertSHA256FileResponse->getBodyAsString());

	if(expectedCACertSHA256Data.empty()) {
		spdlog::error("Certificate authority certificate store SHA256 file is empty.");
		promise->set_value(false);
		return;
	}

	size_t fileNameSeparatorIndex = expectedCACertSHA256Data.find_first_of(" ");

	if(fileNameSeparatorIndex == std::string::npos) {
		fileNameSeparatorIndex = expectedCACertSHA256Data.length();

		spdlog::warn("Certificate authority certificate store SHA256 file missing file name separator whitespace.");
	}

	std::string_view expectedCACertSHA256(expectedCACertSHA256Data.data(), fileNameSeparatorIndex);

	caCertSHA256FileResponse.reset();

	if(!m_updatingCertificateAuthorityCertificateStoreFile) {
		promise->set_value(false);
		return;
	}

	std::string currentCACertSHA256(Utilities::getFileSHA256Hash(caCertFilePath));

	if(!force && Utilities::areStringsEqual(currentCACertSHA256, expectedCACertSHA256)) {
		spdlog::info("Certificate authority certificate store file is already up to date.");
		promise->set_value(true);
		return;
	}

	if(!m_updatingCertificateAuthorityCertificateStoreFile) {
		promise->set_value(false);
		return;
	}

	std::shared_ptr<HTTPRequest> caCertFileRequest(createRequest(HTTPRequest::Method::Get, Utilities::joinPaths(CERTIFICATE_AUTHORITY_CERTIFICATE_PAGE_BASE_URL, CERTIFICATE_AUTHORITY_CERTIFICATE_STORE_FILE_NAME)));

	std::shared_ptr<HTTPResponse> caCertFileResponse(sendRequestAndWait(caCertFileRequest));

	if(caCertFileResponse->isFailure()) {
		spdlog::error("Failed to download certificate authority certificate store file with error: {}", caCertFileResponse->getErrorMessage());
		promise->set_value(false);
		return;
	}
	else if(caCertFileResponse->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(caCertFileResponse->getStatusCode()));
		spdlog::error("Failed to download certificate authority certificate store file ({}{})!", caCertFileResponse->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		promise->set_value(false);
		return;
	}

	if(!m_updatingCertificateAuthorityCertificateStoreFile) {
		promise->set_value(false);
		return;
	}

	std::string calculcatedCACertFileSHA256(caCertFileResponse->getBodySHA256());

	if(!Utilities::areStringsEqual(calculcatedCACertFileSHA256, expectedCACertSHA256)) {
		spdlog::error("Certificate authority certificate store file SHA256 verification failed. Calculated '{}', but expected: '{}'.", calculcatedCACertFileSHA256, expectedCACertSHA256);
		promise->set_value(false);
		return;
	}

	spdlog::debug("Certificate authority certificate store file data download and validated.");

	if(!m_updatingCertificateAuthorityCertificateStoreFile) {
		promise->set_value(false);
		return;
	}

	if(!caCertFileResponse->getBody()->writeTo(caCertFilePath, true)) {
		spdlog::error("Failed to write certificate authority certificate store data to file: '{}'.", caCertFilePath);
		promise->set_value(false);
		return;
	}

	spdlog::info("Certificate authority certificate store file updated successfully!");

	promise->set_value(true);
}

void HTTPService::run() {
	if(!m_initialized || !m_running) {
		return;
	}

	HTTPUtilities::CURLMultiHandle curlMultiHandle;
	HTTPUtilities::CURLSharedHandle curlSharedHandle;

	if((curlMultiHandle = HTTPUtilities::createCURLMultiHandle()) == nullptr) {
		m_running = false;
		return;
	}

	if((curlSharedHandle = HTTPUtilities::createCURLSharedHandle()) == nullptr) {
		m_running = false;
		return;
	}

	std::shared_ptr<HTTPRequest> pendingRequest;
	std::shared_ptr<HTTPRequest> abortedRequest;
	int32_t numberOfRunningHandles = 0;
	int32_t totalNumberOfFileDescriptors = 0;

	while(true) {
		std::unique_lock<std::recursive_mutex> lock(m_mutex);

		if(m_stopRequested) {
			while(!m_pendingRequests.empty()) {
				pendingRequest = m_pendingRequests.front();
				m_pendingRequests.pop_front();
				abortRequest(pendingRequest);
				pendingRequest.reset();
			}

			for(std::map<CURL *, std::shared_ptr<HTTPRequest>>::iterator i = m_activeRequests.begin(); i != m_activeRequests.end(); ++i) {
				abortRequest(i->second);
			}

			m_activeRequests.clear();
		}

		std::vector<std::weak_ptr<HTTPRequest>>::const_iterator newRequestEnd(std::remove_if(m_requests.begin(), m_requests.end(), [](const std::weak_ptr<HTTPRequest> & request) {
			return request.expired();
		}));

		while(!m_pendingRequests.empty() && !hasMaximumActiveRequests()) {
			pendingRequest = m_pendingRequests.front();
			m_pendingRequests.pop_front();
			pendingRequest->getResponse()->setState(HTTPResponse::State::Connecting);
			pendingRequest->startTransfer(m_configuration, curlMultiHandle);
			m_activeRequests[pendingRequest->getCURLEasyHandle().get()] = pendingRequest;
			pendingRequest.reset();
		}

		while(!m_abortedRequests.empty()) {
			abortedRequest = m_abortedRequests.front();
			m_abortedRequests.pop_front();

			if(!HTTPUtilities::isSuccess(curl_multi_remove_handle(curlMultiHandle.get(), abortedRequest->getCURLEasyHandle().get()))) {
				spdlog::error("Failed to remove CURL easy handle from multi handle.");
			}

			abortedRequest->getResponse()->onTransferAborted();

			abortedRequest->getCURLEasyHandle().reset();
			abortedRequest.reset();
		}

		if(m_stopRequested) {
			return;
		}

		curl_multi_wait(curlMultiHandle.get(), nullptr, 0, 100, &totalNumberOfFileDescriptors);

		if(!m_activeRequests.empty()) {
			if(!HTTPUtilities::isSuccess(curl_multi_perform(curlMultiHandle.get(), &numberOfRunningHandles))) {
				spdlog::error("Failed to execute 'curl_multi_perform'.");
			}

			CURLMsg * curlMessage = nullptr;
			int32_t numberOfMessagesInQueue = 0;

			while(true) {
				curlMessage = curl_multi_info_read(curlMultiHandle.get(), &numberOfMessagesInQueue);

				if(curlMessage == nullptr) {
					break;
				}

				if(curlMessage->msg != CURLMSG_DONE) {
					continue;
				}

				std::map<CURL *, std::shared_ptr<HTTPRequest>>::iterator completedRequestIterator(m_activeRequests.find(curlMessage->easy_handle));

				if(completedRequestIterator == m_activeRequests.end()) {
					continue;
				}

				std::shared_ptr<HTTPRequest> completedRequest(completedRequestIterator->second);

				if(completedRequest == nullptr) {
					spdlog::error("Could not find completed request using cURL easy handle!");
					continue;
				}

				completedRequest->getResponse()->onTransferCompleted(HTTPUtilities::isSuccess(curlMessage->data.result), HTTPUtilities::getCURLErrorCodeName(curlMessage->data.result));

				if(!HTTPUtilities::isSuccess(curl_multi_remove_handle(curlMultiHandle.get(), curlMessage->easy_handle))) {
					spdlog::error("Failed to remove CURL easy handle from multi handle.");
				}

				m_activeRequests.erase(curlMessage->easy_handle);

				completedRequest->getCURLEasyHandle().reset();
			}
		}

		if(!m_activeRequests.empty()) {
			std::vector<CURL *> timedOutRequestCURLEasyHandles;

			for(std::map<CURL *, std::shared_ptr<HTTPRequest>>::iterator i = m_activeRequests.begin(); i != m_activeRequests.end(); ++i) {
				if(i->second->getResponse()->checkTimeouts()) {
					timedOutRequestCURLEasyHandles.push_back(i->first);
				}
			}

			for(CURL * curlEasyHandle : timedOutRequestCURLEasyHandles) {
				m_activeRequests.erase(curlEasyHandle);
			}
		}

		if(m_activeRequests.empty() && m_pendingRequests.empty() && m_abortedRequests.empty()) {
			m_waitCondition.wait(lock);
		}
		else if(!m_pendingRequests.empty() || !m_abortedRequests.empty()) {
			std::this_thread::sleep_for(10ms);
		}
		else {
			std::this_thread::sleep_for(30ms);
		}
	}
}
