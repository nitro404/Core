#include "Factory/FactoryRegistry.h"

#include "Platform/Windows/DeviceInformationBridgeWindows.h"

void FactoryRegistry::assignPlatformFactories() {
	setFactory<DeviceInformationBridge>([]() {
		return std::make_unique<DeviceInformationBridgeWindows>();
	});
}
