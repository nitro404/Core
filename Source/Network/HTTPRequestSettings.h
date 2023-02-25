#ifndef _HTTP_REQUEST_SETTINGS_H_
#define _HTTP_REQUEST_SETTINGS_H_

#include <chrono>
#include <mutex>

class HTTPRequestSettings {
public:
	HTTPRequestSettings(std::chrono::seconds connectionTimeout = std::chrono::seconds(0), std::chrono::seconds networkTimeout = std::chrono::seconds(0), std::chrono::seconds transferTimeout = std::chrono::seconds(0), int64_t maximumRedirects = DEFAULT_MAXIMUM_REDIRECTS, bool verboseLoggingEnabled = false);
	HTTPRequestSettings(HTTPRequestSettings && timeout) noexcept;
	HTTPRequestSettings(const HTTPRequestSettings & timeout);
	HTTPRequestSettings & operator = (HTTPRequestSettings && timeout) noexcept;
	HTTPRequestSettings & operator = (const HTTPRequestSettings & timeout);
	virtual ~HTTPRequestSettings();

	bool hasConnectionTimeout() const;
	std::chrono::seconds getConnectionTimeout() const;
	void setConnectionTimeout(std::chrono::seconds timeout);
	bool hasNetworkTimeout() const;
	std::chrono::seconds getNetworkTimeout() const;
	void setNetworkTimeout(std::chrono::seconds timeout);
	bool hasTransferTimeout() const;
	std::chrono::seconds getTransferTimeout() const;
	void setTransferTimeout(std::chrono::seconds timeout);
	int64_t getMaximumRedirects() const;
	void setMaximumRedirects(int64_t maximumRedirects);
	bool isVerboseLoggingEnabled() const;
	void setVerboseLoggingEnabled(bool enabled);

	bool operator == (const HTTPRequestSettings & timeout) const;
	bool operator != (const HTTPRequestSettings & timeout) const;

	static const int64_t DEFAULT_MAXIMUM_REDIRECTS;

protected:
	std::chrono::seconds m_connectionTimeout;
	std::chrono::seconds m_networkTimeout;
	std::chrono::seconds m_transferTimeout;
	int64_t m_maximumRedirects;
	bool m_verboseLoggingEnabled;
	mutable std::recursive_mutex m_requestSettingsMutex;
};

#endif // _HTTP_REQUEST_SETTINGS_H_
