#include "Factory/FactoryRegistry.h"

#include "Logging/Windows/LogSystemWindows.h"
#include "Platform/Windows/DeviceInformationBridgeWindows.h"
#include "Platform/Windows/ProcessCreatorWindows.h"
#include "Platform/Windows/SystemInformationBridgeWindows.h"
#include "Platform/Windows/TimeZoneDataManagerWindows.h"

void FactoryRegistry::assignPlatformFactories() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	setFactory<LogSystem>([]() {
		return std::make_unique<LogSystemWindows>();
	});

	setFactory<DeviceInformationBridge>([]() {
		return std::make_unique<DeviceInformationBridgeWindows>();
	});

	setFactory<ProcessCreator>([]() {
		return std::make_unique<ProcessCreatorWindows>();
	});

	setFactory<SystemInformationBridge>([]() {
		return std::make_unique<SystemInformationBridgeWindows>();
	});

	setFactory<TimeZoneDataManager>([]() {
		return std::make_unique<TimeZoneDataManagerWindows>();
	});
}
