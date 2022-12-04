#ifndef _HTTP_REQUEST_SETTINGS_H_
#define _HTTP_REQUEST_SETTINGS_H_

#include <chrono>
#include <mutex>

class HTTPRequestSettings {
public:
	HTTPRequestSettings(std::chrono::seconds connectionTimeout = std::chrono::seconds(0), std::chrono::seconds networkTimeout = std::chrono::seconds(0));
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
	int64_t getMaximumRedirects() const;
	void setMaximumRedirects(int64_t maximumRedirects);

	bool operator == (const HTTPRequestSettings & timeout) const;
	bool operator != (const HTTPRequestSettings & timeout) const;

	static const int64_t DEFAULT_MAXIMUM_REDIRECTS;

protected:
	std::chrono::seconds m_connectionTimeout;
	std::chrono::seconds m_networkTimeout;
	int64_t m_maximumRedirects;
	mutable std::recursive_mutex m_requestSettingsMutex;
};

#endif // _HTTP_REQUEST_SETTINGS_H_
