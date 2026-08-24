#include "LogSystemDefault.h"

LogSystemDefault::LogSystemDefault()
	: LogSystem(createLogger({
		createConsoleLogSink()
	})) { }

LogSystemDefault::~LogSystemDefault() { }
