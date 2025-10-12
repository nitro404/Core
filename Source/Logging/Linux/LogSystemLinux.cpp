#include "LogSystemLinux.h"

LogSystemLinux::LogSystemLinux()
	: LogSystem(createLogger({
		createConsoleLogSink()
	})) { }

LogSystemLinux::~LogSystemLinux() { }
