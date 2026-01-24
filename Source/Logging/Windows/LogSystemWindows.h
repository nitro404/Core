#ifndef _LOG_SYSTEM_WINDOWS_H_
#define _LOG_SYSTEM_WINDOWS_H_

#include "Logging/LogSystem.h"

class LogSystemWindows final : public LogSystem {
public:
	LogSystemWindows();
	~LogSystemWindows() override;

protected:
	LogSystemWindows(const LogSystemWindows &) = delete;
	const LogSystemWindows & operator = (const LogSystemWindows &) = delete;
};

#endif // _LOG_SYSTEM_WINDOWS_H_
