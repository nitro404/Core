#include "PlatformBridgeLocator.h"

#include "DeviceInformationBridge.h"
#include "PlatformBridgeFactory.h"

std::unique_ptr<DeviceInformationBridge> PlatformBridgeLocator::s_deviceInformationBridge;

void PlatformBridgeLocator::setDeviceInformationBridge(std::unique_ptr<DeviceInformationBridge> bridge) {
	if(bridge == nullptr || s_deviceInformationBridge != nullptr) {
		return;
	}

	s_deviceInformationBridge = std::move(bridge);
}

DeviceInformationBridge * PlatformBridgeLocator::getDeviceInformationBridge() {
	if(s_deviceInformationBridge == nullptr) {
		s_deviceInformationBridge = std::unique_ptr<DeviceInformationBridge>(PlatformBridgeFactory::createDeviceInformationBridge());

// TODO: cleanup
	}

	return s_deviceInformationBridge.get();
}
