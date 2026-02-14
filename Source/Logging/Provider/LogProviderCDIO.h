#ifndef _LOG_PROVIDER_CDIO_H_
#define _LOG_PROVIDER_CDIO_H_

#include "Singleton/Singleton.h"

#include <boost/signals2.hpp>
#include <spdlog/spdlog.h>

class FactoryRegistry;

class LogProviderCDIO final : public Singleton<LogProviderCDIO> {
	friend class FactoryRegistry;

public:
	~LogProviderCDIO() override;

	void initialize();

private:
	LogProviderCDIO();

	void onLogLevelChanged(spdlog::level::level_enum logLevel);

	bool m_initialized;
	boost::signals2::connection m_logLevelChangedConnection;

	LogProviderCDIO(const LogProviderCDIO &) = delete;
	const LogProviderCDIO & operator = (const LogProviderCDIO &) = delete;
};

#endif // _LOG_PROVIDER_CDIO_H_
