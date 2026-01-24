#ifndef _LOG_SINK_WINDOWS_H_
#define _LOG_SINK_WINDOWS_H_

#include <fmt/format.h>
#include <spdlog/sinks/base_sink.h>

#include <mutex>

class LogSinkWindows : public spdlog::sinks::base_sink<std::mutex> {
public:
	LogSinkWindows();
	~LogSinkWindows() override;

protected:
	virtual void sink_it_(const spdlog::details::log_msg & logMessage) override;
	virtual void flush_() override;

private:
	spdlog::memory_buf_t m_formatBuffer;
};

#endif // _LOG_SINK_WINDOWS_H_
