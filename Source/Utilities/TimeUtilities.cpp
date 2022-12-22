#include "TimeUtilities.h"

#include <date/date.h>

#include <array>
#include <ctime>
#include <iomanip>
#include <map>
#include <sstream>

namespace Utilities {

	static const std::string DEFAULT_TIME_FORMAT_STRING("%B %e, %Y %X");
	static const std::string ISO8601_TIME_FORMAT_STRING("%Y-%m-%dT%H:%M:%SZ");

	static const std::map<TimeFormat, std::string> TIME_FORMAT_STRINGS = {
		{ TimeFormat::Default, DEFAULT_TIME_FORMAT_STRING },
		{ TimeFormat::ISO8601, ISO8601_TIME_FORMAT_STRING }
	};

#if !defined(WINDOWS)

	std::tm getLocalTime(bool * error) {
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

	std::tm getUTCTime(bool * error) {
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

	std::optional<std::tm> getLocalTime() {
		bool error = false;

		std::tm localTime = getLocalTime(&error);

		if(error) {
			return {};
		}

		return localTime;
	}

	std::optional<std::tm> getUTCTime() {
		bool error = false;

		std::tm utcTime = getUTCTime(&error);

		if(error) {
			return {};
		}

		return utcTime;
	}

	static std::chrono::time_point<std::chrono::system_clock> parseTimePointFromString(const std::string & time, const std::string & formatString, bool * error) {
		std::chrono::time_point<std::chrono::system_clock> systemTime;
		std::istringstream timeInput(time);
		timeInput >> date::parse(formatString, systemTime);

		if(timeInput.fail() || timeInput.bad()) {
			if(error != nullptr) {
				*error = true;
			}

			return {};
		}

		return systemTime;
	}

	std::chrono::time_point<std::chrono::system_clock> parseTimePointFromString(const std::string & time, std::optional<TimeFormat> formatHint, bool * error) {
		if(formatHint.has_value()) {
			std::map<TimeFormat, std::string>::const_iterator timeFormatStringIterator(TIME_FORMAT_STRINGS.find(formatHint.value()));

			if(timeFormatStringIterator != TIME_FORMAT_STRINGS.end()) {
				return parseTimePointFromString(time, timeFormatStringIterator->second, error);
			}
		}

		std::chrono::time_point<std::chrono::system_clock> systemTime;
		bool currentError = false;

		for(std::map<TimeFormat, std::string>::const_iterator i = TIME_FORMAT_STRINGS.cbegin(); i != TIME_FORMAT_STRINGS.cend(); ++i) {
			systemTime = parseTimePointFromString(time, i->second, &currentError);

			if(!currentError) {
				return systemTime;
			}

			currentError = false;
		}

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	std::optional<std::chrono::time_point<std::chrono::system_clock>> parseTimePointFromString(const std::string & time, std::optional<TimeFormat> formatHint) {
		bool error = false;

		std::chrono::time_point<std::chrono::system_clock> timePoint = parseTimePointFromString(time, formatHint, &error);

		if(error) {
			return {};
		}

		return timePoint;
	}

	std::string timePointToString(std::chrono::time_point<std::chrono::system_clock> timePoint, TimeFormat format) {
		std::map<TimeFormat, std::string>::const_iterator timeFormatStringIterator(TIME_FORMAT_STRINGS.find(format));

		if(timeFormatStringIterator == TIME_FORMAT_STRINGS.end()) {
			return {};
		}

		return date::format(timeFormatStringIterator->second, timePoint);
	}

}
