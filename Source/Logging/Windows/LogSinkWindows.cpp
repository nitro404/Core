#include "LogSinkWindows.h"

#include "Logging/LogSystem.h"

#define NOMINMAX
#include <windows.h>

LogSinkWindows::LogSinkWindows() {
	set_pattern(LogSystem::DEFAULT_PATTERN);
}

LogSinkWindows::~LogSinkWindows() { }

void LogSinkWindows::sink_it_(const spdlog::details::log_msg & logMessage) {
	m_formatBuffer.clear();
	formatter_->format(logMessage, m_formatBuffer);
	m_formatBuffer.push_back('\0');

	OutputDebugStringA(m_formatBuffer.data());
}

void LogSinkWindows::flush_() { }
