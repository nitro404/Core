#include "Utilities/Windows/TimeUtilitiesWindows.h"

#include <chrono>
#include <ctime>

std::tm Utilities::getLocalTime(bool * error) {
	std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::tm localTime;

	if(!localtime_s(&localTime, &currentTime)) {
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

	if(!gmtime_s(&utcTime, &currentTime)) {
		return utcTime;
	}

	if(error != nullptr) {
		*error = true;
	}

	std::tm invalidTime;
	memset(&invalidTime, 0, sizeof(invalidTime));
	return invalidTime;
}

std::chrono::time_point<std::chrono::system_clock> Utilities::fileTimeToSystemClockTime(const FILETIME & fileTime) {
	return std::chrono::system_clock::from_time_t(time_t{0}) + std::chrono::milliseconds(((fileTime.dwLowDateTime | (static_cast<uint64_t>(fileTime.dwHighDateTime) << 32)) / 10000ULL) - 11644473600000ULL);
}
