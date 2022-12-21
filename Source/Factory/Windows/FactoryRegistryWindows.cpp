#include "Factory/FactoryRegistry.h"

#include "Logging/Windows/LogSystemWindows.h"
#include "Platform/Windows/DeviceInformationBridgeWindows.h"
#include "Platform/Windows/ProcessCreatorWindows.h"
#include "Platform/Windows/TimeZoneDataManagerWindows.h"

void FactoryRegistry::assignPlatformFactories() {
	setFactory<LogSystem>([]() {
		return std::make_unique<LogSystemWindows>();
	});

	setFactory<DeviceInformationBridge>([]() {
		return std::make_unique<DeviceInformationBridgeWindows>();
	});

	setFactory<ProcessCreator>([]() {
		return std::make_unique<ProcessCreatorWindows>();
	});

	setFactory<TimeZoneDataManager>([]() {
		return std::make_unique<TimeZoneDataManagerWindows>();
	});
}
