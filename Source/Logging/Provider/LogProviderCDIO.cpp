#include "LogProviderCDIO.h"

#include "Logging/LogSystem.h"
#include "Utilities/CDIOUtilities.h"

#include <cdio/logging.h>

LogProviderCDIO::LogProviderCDIO()
	: m_initialized(false) { }

LogProviderCDIO::~LogProviderCDIO() {
	cdio_log_set_handler(nullptr);

	m_logLevelChangedConnection.disconnect();
}

void LogProviderCDIO::initialize() {
	if(m_initialized) {
		return;
	}

	LogSystem * logSystem = LogSystem::getInstance();

	m_logLevelChangedConnection = logSystem->logLevelChanged.connect(std::bind(&LogProviderCDIO::onLogLevelChanged, this, std::placeholders::_1));

	cdio_loglevel_default = Utilities::spdlogLogLevelToCDIOLogLevel(logSystem->getLevel());

	cdio_log_set_handler([](cdio_log_level_t level, const char message[]) {
		spdlog::log(Utilities::cdioLogLevelToSpdlogLogLevel(level), "CDIO: {}", message);
	});

	m_initialized = true;
}

void LogProviderCDIO::onLogLevelChanged(spdlog::level::level_enum logLevel) {
	cdio_log_level_t newLogLevel = Utilities::spdlogLogLevelToCDIOLogLevel(logLevel);

	spdlog::debug("Updating CDIO log level from '{}' to '{}'.", Utilities::cdioLogLevelToString(cdio_loglevel_default), Utilities::cdioLogLevelToString(newLogLevel));

	cdio_loglevel_default = newLogLevel;
}
