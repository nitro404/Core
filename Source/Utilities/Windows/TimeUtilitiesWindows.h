#ifndef _TIME_UTILITIES_WINDOWS_H_
#define _TIME_UTILITIES_WINDOWS_H_

#include "Utilities/TimeUtilities.h"

#include <Windows.h>

#include <chrono>

namespace Utilities {

	std::chrono::time_point<std::chrono::system_clock> fileTimeToSystemClockTime(const FILETIME & fileTime);

}

#endif // _TIME_UTILITIES_WINDOWS_H_
