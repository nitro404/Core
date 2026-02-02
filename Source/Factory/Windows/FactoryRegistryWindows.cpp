#include "Factory/FactoryRegistry.h"

#include "Logging/Windows/LogSystemWindows.h"
#include "Platform/Windows/DeviceInformationBridgeWindows.h"
#include "Platform/Windows/ProcessCreatorWindows.h"
#include "Platform/Windows/SystemInformationBridgeWindows.h"
#include "Platform/Windows/TimeZoneDataManagerWindows.h"

void FactoryRegistry::assignPlatformFactories() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	setFactory<LogSystem>([]() {
		return std::unique_ptr<LogSystemWindows>(new LogSystemWindows());
	});

	setFactory<DeviceInformationBridge>([]() {
		return std::unique_ptr<DeviceInformationBridgeWindows>(new DeviceInformationBridgeWindows());
	});

	setFactory<ProcessCreator>([]() {
		return std::unique_ptr<ProcessCreatorWindows>(new ProcessCreatorWindows());
	});

	setFactory<SystemInformationBridge>([]() {
		return std::unique_ptr<SystemInformationBridgeWindows>(new SystemInformationBridgeWindows());
	});

	setFactory<TimeZoneDataManager>([]() {
		return std::unique_ptr<TimeZoneDataManagerWindows>(new TimeZoneDataManagerWindows());
	});
}
