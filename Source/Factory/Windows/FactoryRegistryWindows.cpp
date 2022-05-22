#include "Factory/FactoryRegistry.h"

#include "Logging/Windows/LogSystemWindows.h"
#include "Platform/Windows/DeviceInformationBridgeWindows.h"

void FactoryRegistry::assignPlatformFactories() {
	setFactory<LogSystem>([]() {
		return std::make_unique<LogSystemWindows>();
	});

	setFactory<DeviceInformationBridge>([]() {
		return std::make_unique<DeviceInformationBridgeWindows>();
	});
}
