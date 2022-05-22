#ifndef _LOG_SYSTEM_WINDOWS_H_
#define _LOG_SYSTEM_WINDOWS_H_

#include "Logging/LogSystem.h"

class LogSystemWindows final : public LogSystem {
public:
	LogSystemWindows();
	virtual ~LogSystemWindows();

protected:
	LogSystemWindows(const LogSystemWindows &) = delete;
	LogSystemWindows(LogSystemWindows &&) noexcept = delete;
	const LogSystemWindows & operator = (const LogSystemWindows &) = delete;
	const LogSystemWindows & operator = (LogSystemWindows &&) noexcept = delete;
};

#endif // _LOG_SYSTEM_WINDOWS_H_
