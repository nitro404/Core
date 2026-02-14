#include "CDIOUtilities.h"

spdlog::level::level_enum Utilities::cdioLogLevelToSpdlogLogLevel(cdio_log_level_t logLevel) {
	switch(logLevel) {
		case CDIO_LOG_DEBUG:
			return spdlog::level::level_enum::debug;
		case CDIO_LOG_INFO:
			return spdlog::level::level_enum::info;
		case CDIO_LOG_WARN:
			return spdlog::level::level_enum::warn;
		case CDIO_LOG_ERROR:
			return spdlog::level::level_enum::err;
		case CDIO_LOG_ASSERT:
			return spdlog::level::level_enum::critical;
	}

	return spdlog::level::level_enum::debug;
}

cdio_log_level_t Utilities::spdlogLogLevelToCDIOLogLevel(spdlog::level::level_enum logLevel) {
	switch(logLevel) {
		case spdlog::level::level_enum::trace:
		case spdlog::level::level_enum::debug:
			return CDIO_LOG_DEBUG;
		case spdlog::level::level_enum::info:
			return CDIO_LOG_INFO;
		case spdlog::level::level_enum::warn:
			return CDIO_LOG_WARN;
		case spdlog::level::level_enum::err:
			return CDIO_LOG_ERROR;
		case spdlog::level::level_enum::critical:
			return CDIO_LOG_ASSERT;
		case spdlog::level::level_enum::off:
		case spdlog::level::level_enum::n_levels:
			break;
	}

	return CDIO_LOG_DEBUG;
}

std::string Utilities::logLevelToString(cdio_log_level_t logLevel) {
	switch(logLevel) {
		case CDIO_LOG_DEBUG:
			return "Debug";
		case CDIO_LOG_INFO:
			return "Info";
		case CDIO_LOG_WARN:
			return "Warning";
		case CDIO_LOG_ERROR:
			return "Error";
		case CDIO_LOG_ASSERT:
			return "Assert";
	}

	return "";
}
