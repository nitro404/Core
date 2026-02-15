#include "CDIOUtilities.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

driver_id_t Utilities::getCDIODriverIDFromFileExtension(const std::string_view filePathOrExtension) {
	if(filePathOrExtension.empty()) {
		return DRIVER_UNKNOWN;
	}

	std::string_view fileExtension(Utilities::getFileExtension(filePathOrExtension));

	if(fileExtension.empty()) {
		return DRIVER_UNKNOWN;
	}

	if(Utilities::areStringsEqualIgnoreCase(fileExtension, "cue")) {
		return DRIVER_BINCUE;
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "nrg")) {
		return DRIVER_NRG;
	}

	return DRIVER_UNKNOWN;
}

std::string Utilities::cdioDriverIDToString(driver_id_t driverID) {
	switch(driverID) {
		case DRIVER_UNKNOWN:
			return "Unknown";
		case DRIVER_AIX:
			return "AIX";
		case DRIVER_FREEBSD:
			return "FreeBSD";
		case DRIVER_NETBSD:
			return "NetBSD";
		case DRIVER_LINUX:
			return "Linux";
		case DRIVER_SOLARIS:
			return "Solaris";
		case DRIVER_OSX:
			return "OSX";
		case DRIVER_WIN32:
			return "Win32";
		case DRIVER_CDRDAO:
			return "CDRDAO";
		case DRIVER_BINCUE:
			return "BinCue";
		case DRIVER_NRG:
			return "NRG";
		case DRIVER_DEVICE:
			return "Device";
	}

	return "";
}

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

std::string Utilities::cdioLogLevelToString(cdio_log_level_t logLevel) {
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
