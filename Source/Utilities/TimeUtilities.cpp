#include "TimeUtilities.h"

#include <fmt/core.h>

#include <ctime>
#include <iomanip>
#include <sstream>

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

std::string Utilities::timePointToString(std::chrono::time_point<std::chrono::system_clock> timePoint, TimePointFormat format) {
	std::time_t dateTime = std::chrono::system_clock::to_time_t(timePoint);
	std::tm localTime = *std::localtime(&dateTime);

	std::string formatString;
	std::stringstream timePointStringStream;

	switch(format) {
		case TimePointFormat::Default: {
			formatString = "%B %e, %Y %X";
			break;
		}

		case TimePointFormat::ISO8601: {
			formatString = "%FT%T";
			break;
		}
	}

	timePointStringStream << std::put_time(&localTime, formatString.c_str());

	if(format == TimePointFormat::ISO8601) {
		timePointStringStream << fmt::format(".{}Z", std::chrono::time_point_cast<std::chrono::milliseconds>(timePoint).time_since_epoch().count() % 1000);
	}

	return timePointStringStream.str();
}
