#include "Factory/FactoryRegistry.h"

#include "Logging/Windows/LogSystemWindows.h"
#include "Platform/Windows/DeviceInformationBridgeWindows.h"
#include "Platform/Windows/ProcessCreatorWindows.h"

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
}
