#ifndef _LOG_SYSTEM_H_
#define _LOG_SYSTEM_H_

#include "Singleton/Singleton.h"

#include <boost/signals2.hpp>
#include <spdlog/spdlog.h>

#include <memory>
#include <mutex>
#include <optional>
#include <string>

class LogSystem : public Singleton<LogSystem> {
public:
	virtual ~LogSystem();

	bool isEnabled() const;
	void enable();
	void disable();
	void setEnabled(bool enabled);
	spdlog::level::level_enum getLevel() const;
	void setLevel(spdlog::level::level_enum level);
	spdlog::level::level_enum getFlushLevel() const;
	void setFlushLevel(spdlog::level::level_enum level);
	void setPattern(const std::string & pattern);
	void flush();

	size_t numberOfLogSinks() const;
	bool hasLogSink(const std::shared_ptr<spdlog::sinks::sink> & logSink) const;
	bool addLogSink(std::shared_ptr<spdlog::sinks::sink> logSink);
	void removeLogSink(const std::shared_ptr<spdlog::sinks::sink> & logSink);
	void clearLogSinks();

	boost::signals2::signal<void (spdlog::level::level_enum /* logLevel */)> logLevelChanged;
	boost::signals2::signal<void (bool /* enabled */)> statusChanged;

	static const spdlog::level::level_enum DEFAULT_LEVEL;
	static const char * DEFAULT_PATTERN;

protected:
	LogSystem(std::shared_ptr<spdlog::logger> logger);

	static std::shared_ptr<spdlog::sinks::sink> createConsoleLogSink();
	static std::shared_ptr<spdlog::logger> createLogger(spdlog::sinks_init_list logSinks);
	static std::shared_ptr<spdlog::logger> createLogger(const std::string & name, spdlog::sinks_init_list logSinks);

private:
	std::shared_ptr<spdlog::logger> m_logger;
	std::optional<spdlog::level::level_enum> m_previousLevel;

	mutable std::mutex m_mutex;

	LogSystem() = delete;
	LogSystem(const LogSystem &) = delete;
	const LogSystem & operator = (const LogSystem &) = delete;
};

#endif // _LOG_SYSTEM_H_
