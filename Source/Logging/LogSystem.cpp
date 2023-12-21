#include "LogSystem.h"

#include "Utilities/StringUtilities.h"

#include <spdlog/sinks/stdout_color_sinks.h>

#if _DEBUG
const spdlog::level::level_enum LogSystem::DEFAULT_LEVEL = spdlog::level::level_enum::trace;
#else
const spdlog::level::level_enum LogSystem::DEFAULT_LEVEL = spdlog::level::level_enum::info;
#endif // _DEBUG

#if _DEBUG
const char * LogSystem::DEFAULT_PATTERN = "%^%T.%e [%t] %L: %v%$";
#else
const char * LogSystem::DEFAULT_PATTERN = "%^%T.%e %L: %v%$";
#endif // _DEBUG

LogSystem::LogSystem(std::shared_ptr<spdlog::logger> logger)
	: m_logger(logger) {
	spdlog::set_default_logger(m_logger);
}

LogSystem::~LogSystem() { }

bool LogSystem::isEnabled() const {
	return !m_previousLevel.has_value();
}

void LogSystem::enable() {
	setEnabled(true);
}

void LogSystem::disable() {
	setEnabled(false);
}

void LogSystem::setEnabled(bool enabled) {
	std::lock_guard lock(m_mutex);

	if(isEnabled() == enabled) {
		return;
	}

	if(enabled) {
		if(m_previousLevel.has_value()) {
			m_logger->set_level(m_previousLevel.value());
			m_previousLevel.reset();
		}
	}
	else {
		if(!m_previousLevel.has_value()) {
			m_previousLevel = m_logger->level();
			m_logger->set_level(spdlog::level::level_enum::off);
		}
	}

	statusChanged(enabled);
}

spdlog::level::level_enum LogSystem::getLevel() const {
	return m_logger->level();
}

void LogSystem::setLevel(spdlog::level::level_enum level) {
	std::unique_lock lock(m_mutex);

	if(level == spdlog::level::level_enum::n_levels) {
		return;
	}

	if(m_previousLevel.has_value()) {
		m_previousLevel = level;
	}
	else {
		m_logger->set_level(level);
	}

	lock.unlock();

	logLevelChanged(level);
}

spdlog::level::level_enum LogSystem::getFlushLevel() const {
	return m_logger->flush_level();
}

void LogSystem::setFlushLevel(spdlog::level::level_enum level) {
	if(level == spdlog::level::level_enum::n_levels) {
		return;
	}

	m_logger->flush_on(level);
}

void LogSystem::setPattern(const std::string & pattern) {
	m_logger->set_pattern(pattern);
}

void LogSystem::flush() {
	m_logger->flush();
}

size_t LogSystem::numberOfLogSinks() const {
	return m_logger->sinks().size();
}

bool LogSystem::hasLogSink(const std::shared_ptr<spdlog::sinks::sink> & logSink) const {
	const std::vector<spdlog::sink_ptr> & logSinks(m_logger->sinks());

	return std::find(std::begin(logSinks), std::end(logSinks), logSink) != std::end(logSinks);
}

bool LogSystem::addLogSink(std::shared_ptr<spdlog::sinks::sink> logSink) {
	if(logSink == nullptr) {
		return false;
	}

	m_logger->sinks().push_back(logSink);

	return true;
}

void LogSystem::removeLogSink(const std::shared_ptr<spdlog::sinks::sink> & logSink) {
	if(logSink == nullptr) {
		return;
	}

	std::vector<spdlog::sink_ptr> & logSinks(m_logger->sinks());
	logSinks.erase(std::remove(logSinks.begin(), logSinks.end(), logSink), logSinks.end());
}

void LogSystem::clearLogSinks() {
	m_logger->sinks().clear();
}

std::shared_ptr<spdlog::sinks::sink> LogSystem::createConsoleLogSink() {
	std::shared_ptr<spdlog::sinks::sink> consoleLogSink(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	consoleLogSink->set_pattern(DEFAULT_PATTERN);

	return consoleLogSink;
}

std::shared_ptr<spdlog::logger> LogSystem::createLogger(spdlog::sinks_init_list logSinks) {
	return createLogger(Utilities::emptyString, logSinks);
}

std::shared_ptr<spdlog::logger> LogSystem::createLogger(const std::string & name, spdlog::sinks_init_list logSinks) {
	std::shared_ptr<spdlog::logger> logger(std::make_shared<spdlog::logger>(name, logSinks));
	logger->set_level(DEFAULT_LEVEL);

	return logger;
}
