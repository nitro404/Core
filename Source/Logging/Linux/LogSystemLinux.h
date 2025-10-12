#ifndef _LOG_SYSTEM_LINUX_H_
#define _LOG_SYSTEM_LINUX_H_

#include "Logging/LogSystem.h"

class LogSystemLinux final : public LogSystem {
public:
	LogSystemLinux();
	virtual ~LogSystemLinux();

protected:
	LogSystemLinux(const LogSystemLinux &) = delete;
	const LogSystemLinux & operator = (const LogSystemLinux &) = delete;
};

#endif // _LOG_SYSTEM_LINUX_H_
