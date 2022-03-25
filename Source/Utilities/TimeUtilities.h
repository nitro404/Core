#ifndef _TIME_UTILITIES_H_
#define _TIME_UTILITIES_H_

#include <chrono>
#include <optional>
#include <time.h>

namespace Utilities {

	std::tm getLocalTime(bool * error);
	std::tm getUTCTime(bool * error);
	std::optional<std::tm> getLocalTime();
	std::optional<std::tm> getUTCTime();
	std::string timePointToString(std::chrono::time_point<std::chrono::system_clock> timePoint);

}

#endif // _TIME_UTILITIES_H_
