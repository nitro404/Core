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

std::chrono::milliseconds Utilities::fileTimeToDuration(const FILETIME & fileTime) {
// TODO: ??
	SYSTEMTIME systemTime = FileTimeToSystemTime(fileTime);

	std::tm time;
	time.tm_sec = st.wSecond;
	time.tm_min = st.wMinute;
	time.tm_hour = st.wHour;
	time.tm_mday = st.wDay;
	time.tm_mon = st.wMonth - 1;
	time.tm_year = st.wYear - 1900;
	time.tm_isdst = -1;
	std::time_t t = std::mktime(&time);

	return std::chrono::milliseconds(((fileTime.dwLowDateTime | (static_cast<uint64_t>(fileTime.dwHighDateTime) << 32)) / 10000ULL) - 11644473600000ULL);
}

std::chrono::time_point<std::chrono::system_clock> Utilities::fileTimeToSystemClockTime(const FILETIME & fileTime) {
	return std::chrono::system_clock::from_time_t(time_t{0}) + fileTimeToDuration(fileTime);
}
