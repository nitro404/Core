#ifndef _PLATFORM_BRIDGE_LOCATOR_H_
#define _PLATFORM_BRIDGE_LOCATOR_H_

#include "DeviceInformationBridge.h"

#include <memory>

class PlatformBridgeLocator final {
public:
	static void setDeviceInformationBridge(std::unique_ptr<DeviceInformationBridge> bridge);
	static DeviceInformationBridge * getDeviceInformationBridge();

private:
	PlatformBridgeLocator() = delete;

	static std::unique_ptr<DeviceInformationBridge> s_deviceInformationBridge;
};

#endif // _PLATFORM_BRIDGE_LOCATOR_H_
