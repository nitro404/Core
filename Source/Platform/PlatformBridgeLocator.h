#ifndef _PLATFORM_BRIDGE_LOCATOR_H_
#define _PLATFORM_BRIDGE_LOCATOR_H_

#include "DeviceInformationBridge.h"

#include <memory>

class PlatformBridgeLocator final {
public:
	static void setDeviceInformationBridge(std::unique_ptr<DeviceInformationBridge> bridge);
	static DeviceInformationBridge * getDeviceInformationBridge();

private:
	static std::unique_ptr<DeviceInformationBridge> s_deviceInformationBridge;

	PlatformBridgeLocator() = delete;
	PlatformBridgeLocator(const PlatformBridgeLocator &) = delete;
	PlatformBridgeLocator(PlatformBridgeLocator &&) noexcept = delete;
	const PlatformBridgeLocator & operator = (const PlatformBridgeLocator &) = delete;
	const PlatformBridgeLocator & operator = (PlatformBridgeLocator &&) noexcept = delete;
};

#endif // _PLATFORM_BRIDGE_LOCATOR_H_
