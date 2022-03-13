#include "TimeUtilities.h"

#include <chrono>
#include <ctime>

#if !defined(WINDOWS)

std::tm Utilities::getLocalTime(bool * error) {
	std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::tm localTime;

	if(localtime_r(&currentTime, &localTime)) {
		return localTime;
	}

	if(error != nullptr) {
		*error = true;
	}

	std::tm invalidTime;
	memset(&invalidTime, 0, sizeof(invalidTime));
	return invalidTime;
}

std::tm Utilities::getUTCTime(bool * error) {
	std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::tm utcTime;

	if(gmtime_r(&currentTime, &utcTime)) {
		return utcTime;
	}

	if(error != nullptr) {
		*error = true;
	}

	std::tm invalidTime;
	memset(&invalidTime, 0, sizeof(invalidTime));
	return invalidTime;
}

#endif // WINDOWS

std::optional<std::tm> Utilities::getLocalTime() {
	bool error = false;

	std::tm localTime = Utilities::getLocalTime(&error);

	if(error) {
		return {};
	}

	return localTime;
}

std::optional<std::tm> Utilities::getUTCTime() {
	bool error = false;

	std::tm utcTime = Utilities::getUTCTime(&error);

	if(error) {
		return {};
	}

	return utcTime;
}
