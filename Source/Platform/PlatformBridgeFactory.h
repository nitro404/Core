#ifndef _PLATFORM_BRIDGE_FACTORY_H_
#define _PLATFORM_BRIDGE_FACTORY_H_

#include "DeviceInformationBridge.h"

class PlatformBridgeFactory final {
public:
	static DeviceInformationBridge * createDeviceInformationBridge();

private:
	PlatformBridgeFactory() = delete;
	PlatformBridgeFactory(const PlatformBridgeFactory &) = delete;
	PlatformBridgeFactory(PlatformBridgeFactory &&) noexcept = delete;
	const PlatformBridgeFactory & operator = (const PlatformBridgeFactory &) = delete;
	const PlatformBridgeFactory & operator = (PlatformBridgeFactory &&) noexcept = delete;
};

#endif // _PLATFORM_BRIDGE_FACTORY_H_
