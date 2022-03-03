#ifndef _HTTP_TIMEOUT_H_
#define _HTTP_TIMEOUT_H_

#include <chrono>
#include <mutex>

class HTTPTimeout {
public:
	HTTPTimeout(std::chrono::seconds connectionTimeout = std::chrono::seconds(0), std::chrono::seconds networkTimeout = std::chrono::seconds(0));
	HTTPTimeout(HTTPTimeout && timeout) noexcept;
	HTTPTimeout(const HTTPTimeout & timeout);
	HTTPTimeout & operator = (HTTPTimeout && timeout) noexcept;
	HTTPTimeout & operator = (const HTTPTimeout & timeout);
	virtual ~HTTPTimeout();

	bool hasConnectionTimeout() const;
	std::chrono::seconds getConnectionTimeout() const;
	void setConnectionTimeout(std::chrono::seconds timeout);
	bool hasNetworkTimeout() const;
	std::chrono::seconds getNetworkTimeout() const;
	void setNetworkTimeout(std::chrono::seconds timeout);

	bool operator == (const HTTPTimeout & timeout) const;
	bool operator != (const HTTPTimeout & timeout) const;

protected:
	std::chrono::seconds m_connectionTimeout;
	std::chrono::seconds m_networkTimeout;
	mutable std::recursive_mutex m_timeoutMutex;
};

#endif // _HTTP_TIMEOUT_H_
