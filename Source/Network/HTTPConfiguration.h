#ifndef _HTTP_CONFIGURATION_H_
#define _HTTP_CONFIGURATION_H_

#include <chrono>
#include <optional>
#include <string>

struct HTTPConfiguration {
	std::string certificateAuthorityCertificateStoreDirectoryPath;
	std::string baseURL;
	std::optional<std::chrono::seconds> connectionTimeout;
	std::optional<std::chrono::seconds> networkTimeout;
	std::optional<std::chrono::seconds> transferTimeout;
	std::optional<int64_t> maximumRedirects;
};

#endif // _HTTP_CONFIGURATION_H_
