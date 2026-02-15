#ifndef _CDIO_UTILITIES_H_
#define _CDIO_UTILITIES_H_

#include <cdio/logging.h>
#include <spdlog/spdlog.h>

#include <string>

namespace Utilities {

	driver_id_t getCDIODriverIDFromFileExtension(const std::string_view filePathOrExtension);
	std::string cdioDriverIDToString(driver_id_t driverID);
	spdlog::level::level_enum cdioLogLevelToSpdlogLogLevel(cdio_log_level_t logLevel);
	cdio_log_level_t spdlogLogLevelToCDIOLogLevel(spdlog::level::level_enum logLevel);
	std::string cdioLogLevelToString(cdio_log_level_t logLevel);

}

#endif // _CDIO_UTILITIES_H_
