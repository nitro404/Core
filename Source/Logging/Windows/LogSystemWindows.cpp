#include "LogSystemWindows.h"

#include "LogSinkWindows.h"

LogSystemWindows::LogSystemWindows()
	: LogSystem(createLogger({
		createConsoleLogSink(),
		std::make_shared<LogSinkWindows>()
	})) { }

LogSystemWindows::~LogSystemWindows() { }
