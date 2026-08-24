#ifndef _LOG_SYSTEM_DEFAULT_H_
#define _LOG_SYSTEM_DEFAULT_H_

#include "Logging/LogSystem.h"

class LogSystemDefault final : public LogSystem {
	friend class FactoryRegistry;

public:
	~LogSystemDefault() override;

private:
	LogSystemDefault();

	LogSystemDefault(const LogSystemDefault &) = delete;
	const LogSystemDefault & operator = (const LogSystemDefault &) = delete;
};

#endif // _LOG_SYSTEM_DEFAULT_H_
